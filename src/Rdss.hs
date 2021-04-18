{-# language
    DataKinds
  , DeriveDataTypeable
  , DeriveGeneric
  , DeriveTraversable
  , DerivingStrategies
  , DuplicateRecordFields
  , GeneralizedNewtypeDeriving
  , ImportQualifiedPost
  , LambdaCase
  , OverloadedStrings
  , PackageImports
  , RankNTypes
  , RecordWildCards
  , ScopedTypeVariables
  , StrictData
  , TypeApplications
#-}

module Rdss where

import "base" Control.Monad
import "base" Data.Char qualified as Char
import "base" Data.Data (Data)
import "base" Data.Maybe
import "base" Data.List qualified as List
import "base" GHC.Generics (Generic, Generic1)
import "base" Numeric.Natural
import "base" Data.String (IsString)
import "containers" Data.Map.Strict (Map)
import "containers" Data.Map.Strict qualified as Map
import "containers" Data.Set (Set)
import "generic-lens" Data.Generics.Product (field)
import "ilist" Data.List.Index (iforM_)
import "lens" Control.Lens (Lens', (^.))
import "lens" Control.Lens qualified as Lens
import "mtl" Control.Monad.State.Class (MonadState, get, modify, put)
import "mtl" Control.Monad.State.Strict (State, execState)
import "transformers" Control.Monad.Trans.Writer.CPS (WriterT, Writer, execWriter, tell)

--------------------------------------------------------------------------------

type Attr = Int

type AttrPermutation = [Attr]

type AttrPartialPermutation = [Maybe Attr]

-- invariant: must have number of arguments equal to number of columns in the table
data Function rel
  = Function
      String -- ^ name of function in scope
  deriving stock (Eq, Ord, Show, Generic, Generic1)
  deriving stock (Functor, Foldable, Traversable)

data Viewed rel
  = Viewed
  { attrPartialPermutation :: AttrPartialPermutation
  , rel :: rel
  }
  deriving stock (Eq, Ord, Show, Generic, Generic1)
  deriving stock (Functor, Foldable, Traversable)

data Predicate rel
  = PredicateAnd  (Predicate rel) (Predicate rel)
  | PredicateOr   (Predicate rel) (Predicate rel)
  | PredicateNot  (Predicate rel)
  | PredicateLike Attr            String
  | PredicateLT   Attr            Int
  | PredicateEQ   Attr            Int
  deriving stock (Eq, Ord, Show, Generic, Generic1)
  deriving stock (Functor, Foldable, Traversable)

--------------------------------------------------------------------------------

data RelAlgebra rel
  = Not                        (Viewed rel)
  | Join       (Set Attr)      (Viewed rel) (Viewed rel)
  | Union                      (Viewed rel) (Viewed rel)
  | Difference                 (Viewed rel) (Viewed rel)
  | Select     (Predicate rel) (Viewed rel)
  | Map        (Function rel)  (Viewed rel)
  | View                       (Viewed rel)
  -- TODO: aggregation?
  deriving stock (Eq, Ord, Show, Generic, Generic1)
  deriving stock (Functor, Foldable, Traversable)

newtype Rel = Rel { getRel :: Int }

testRel :: DataStructure
testRel = synthesize
  [ ( Rel 2
    , Union (Viewed [Just 0] (Rel 0)) (Viewed [Just 0] (Rel 1))
    )
  ]

synthesize :: [(Rel, RelAlgebra Rel)] -> DataStructure
synthesize = snd . flip execState (0, initialD) . mapM_ go
  where
    initialD = DataStructure "Empty"
      []
      []
      [ Method "insert0" [("r", TyRow [TyInt])] []
      , Method "insert1" [("r", TyRow [TyInt])] []
      ]

    freshName :: State (Int, a) VarName
    freshName = do
      (i, _) <- get
      Lens.modifying Lens._1 (+1)
      pure $ VarName $ "x" ++ show i

    go :: (Rel, RelAlgebra Rel) -> State (Int, DataStructure) ()
    go (lhs, rhs) = do
      case rhs of
        Not v -> do
          pure ()
        Join attrs vx vy -> do
          let rx = vx ^. field @"rel"
          let ry = vy ^. field @"rel"
          pure ()
        Union vx vy -> do
          let rx = vx ^. field @"rel"
          let ry = vy ^. field @"rel"
          (resultx, gvx) <- genView vx "r"
          Lens.modifying
            (methodLens (insertMethod rx))
            ( (++ [Invoke (insertMethod lhs) ["ds", resultx]])
            . (++ gvx)
            )
          (resulty, gvy) <- genView vy "r"
          Lens.modifying
            (methodLens (insertMethod ry))
            ( (++ [Invoke (insertMethod lhs) ["ds", resulty]])
            . (++ gvy)
            )
          pure ()
        Difference vx vy -> do
          pure ()
        Select p v -> do
          pure ()
        Map f v -> do
          pure ()
        View v -> do
          pure ()

    insertMethod :: Rel -> VarName
    insertMethod r = VarName $ "insert" ++ (show $ getRel r)

    methodLens :: VarName -> Lens.ASetter' (Int, DataStructure) [Action]
    methodLens varName = Lens._2
      . field @"methods"
      . methodAt varName
      . field @"body"

    genView :: Viewed Rel -> VarName -> State (Int, a) (VarName, [Action])
    genView (Viewed perm _) input = do
      result <- freshName
      indices <- forM inverted $ \attr -> do
        n <- freshName
        pure (n, (IndexRow n input (fromIntegral attr) (length perm == 1)))
      let creation = CreateRow result (map fst indices)
      pure (result, (map snd indices ++ [creation]))
      where
        inverted :: [Attr]
        inverted =
          let m = maximum (catMaybes perm)
          in map (\i -> fromJust (List.elemIndex (Just i) perm)) [0..m]

methodAt :: VarName -> Lens' [Method] Method
methodAt varName = Lens.lens
  (fromJust . List.find p)
  (\methods m -> case List.findIndex p methods of
    Nothing -> error "methodAt: invariant violated"
    Just i -> Lens.set (Lens.ix i) m methods
  )
  where
    p = (== varName) . (^. field @"name")

--------------------------------------------------------------------------------

test :: IO ()
test = do
  let ds =
        [ DataStructure
          { name = "Tuple"
          , tyParams =
              [ TyParam "a"
              , TyParam "b"
              ]
          , members =
              [ Member "x" (TyBasic (TypeName "a"))
              , Member "y" (TyBasic (TypeName "b"))
              ]
          , methods =
              [ Method "setFst"
                  [ ("x", TyBasic (TypeName "a"))
                  ]
                  [ CreateHashMap "h"
                  , InsertHashMap "h" "foo" "1"
                  , InsertHashMap "h" "bar" "2"
                  , IterateOverHashMap "h"
                    $ \k v ->
                    [ DeleteHashMap "h" k
                    , InsertHashMap "h" k v
                    , IterateOverHashMap "h"
                      $ \k' v' ->
                      [ DeleteHashMap "h" k'
                      , InsertHashMap "h" k' v'
                      ]
                    ]
                  , LookupHashMap "result" "h" "foo"
                  , CreateRow "testRow" ["x", "y", "z"]
                  , IndexRow "fromRow" "testRow" 1 False
                  ]
              ]
          }
        , testRel
        ]
  mapM_ (putStrLn . toHaskell) ds

toHaskell :: DataStructure -> String
toHaskell d@DataStructure{..} = execWriter go
  where
    go :: Writer String ()
    go = do
      tell $ "data " ++ capFirst name
      forM_ (TyParam "s" : tyParams) $ \(TyParam tyParam) -> do
        tell " "
        tell tyParam
      newline
      line $ indent $ "= Mk" ++ capFirst name
      tell $ indent "{"
      iforM_ members $ \i member -> do
        let prefix = if i == 0 then " " else indent ", "
        line $ prefix ++ member ^. field @"name" ++ " :: " ++ haskellType (member ^. field @"typ")
      line $ indent "}"
      forM_ methods $ \method -> do
        newline
        line $ haskellMethod method

    haskellType :: Type -> String
    haskellType = \case
      TyBasic (TypeName s) -> s
      TyInt -> "Int"
      TyRow [] -> "()"
      TyRow [x] -> haskellType x
      TyRow xs -> "(" ++ List.intercalate ", " (map haskellType xs) ++ ")"
      _ -> error "TODO"

    haskellMethod :: Method -> String
    haskellMethod Method{..} = execWriter go'
      where
        go' :: Writer String ()
        go' = do
          tell $ getVarName name ++ " :: "
          let typsig = d ^. field @"name"
                ++ " s "
                ++ concatMap ((++ " ") . getTyParam) tyParams
                ++ "-> "
                ++ List.intercalate " -> "
                     (map (haskellType . snd) args)
                ++ " -> ST s ()"
          line typsig
          tell $ getVarName name
          tell " ds"
          forM_ args $ \(VarName bndr, _) -> do
            tell $ " " ++ bndr
          line " = do"
          forM_ body $ \action -> do
            haskellAction 0 action
          line $ indent $ "pure ()"

    haskellAction :: Int -> Action -> Writer String ()
    haskellAction i = \case
      AssignConstant (VarName assignTo) c -> do
        line $ indent $ assignTo ++ " <- newMutVar " ++ c
      Invoke (VarName f) args -> do
        line $ indent $ f ++ " " ++ List.intercalate " " (map getVarName args)
      CreateRow (VarName row) elements -> do
        line $ indent $ "let " ++ row ++ " = (" ++ List.intercalate ", " (map getVarName elements) ++ ")"
      IndexRow (VarName assignTo) (VarName row) index hasSize1 -> do
        if hasSize1
        then do
          line $ indent $ "let " ++ assignTo ++ " = " ++ row
        else do
          line $ indent $ "let " ++ assignTo ++ " = " ++ row ++ " ^. _" ++ show (index + 1)
      CreateHashMap (VarName hmap) -> do
        line $ indent $ hmap ++ " <- H.new"
      LookupHashMap (VarName result) (VarName hmap) (VarName key) -> do
        line $ indent $ result ++ " <- fromJust <$> " ++ "H.lookup " ++ hmap ++ " " ++ key
      InsertHashMap (VarName hmap) (VarName key) (VarName value) -> do
        line $ indent $ "H.insert " ++ List.intercalate " " [hmap, key, value]
      DeleteHashMap (VarName hmap) (VarName key) -> do
        line $ indent $ "H.delete " ++ List.intercalate " " [hmap, key]
      IterateOverHashMap (VarName hmap) loop -> do
        let k = "k" ++ show i
        let v = "v" ++ show i
        let actions = loop (VarName k) (VarName v)

        line $ indent $ "flip H.mapM_ " ++ hmap ++ " $ \\(" ++ k ++ ", " ++ v ++ ") -> do"
        line $ indent $ execWriter $ do
          forM_ actions $ \action -> do
            haskellAction (i + 1) action

    line :: Monad m => String -> WriterT String m ()
    line s = tell (s ++ "\n")

    newline :: Monad m => WriterT String m ()
    newline = line ""


indentSpaces :: Int
indentSpaces = 2

indent :: String -> String
indent = List.intercalate "\n" . map (replicate indentSpaces ' ' ++) . lines

data DataStructure
  = DataStructure
    { name :: String
    , tyParams :: [TyParam]
    , members :: [Member]
    , methods :: [Method]
    }
  deriving stock (Generic)

newtype TyParam = TyParam { getTyParam :: String }

data Member
  = Member
    { name :: String
    , typ :: Type
    }
  deriving stock (Generic)

data Type
  = TyInt
  | TyBasic   TypeName
  | TyRow     [Type]
  | TyHashSet Type
  | TyHashMap Type Type
  | TyTrie    Type Type
  | TyVector  Type

data Method
  = Method
    { name :: VarName
    , args :: [(VarName, Type)]
    , body :: [Action]
    }
  deriving stock (Generic)

data Action
  = AssignConstant
    VarName  -- ^ variable to assign to
    String -- ^ constant to assign
  | CreateRow
    VarName -- ^ row name
    [VarName] -- ^ elements in the row
  | IndexRow
    VarName -- ^ variable to assign to
    VarName -- ^ row to index into
    Natural -- ^ index to use
    Bool -- ^ whether the row has size 1
  | Invoke
    VarName -- ^ name of method
    [VarName] -- ^ arguments
  | CreateHashSet
    VarName -- ^ hashset to create
  | InsertHashSet
    VarName -- ^ hashset to modify
    VarName -- ^ value to insert
  | DeleteHashSet
    VarName -- ^ hashset to modify
    VarName -- ^ value to delete
  | IterateOverHashSet
    VarName             -- ^ hashset to loop over
    (VarName -> [Action]) -- ^ body of loop

  | CreateHashMap
    VarName -- ^ hashmap to create
  | LookupHashMap
    VarName -- ^ where to store the result
    VarName -- ^ hashmap to query
    VarName -- ^ key
  | InsertHashMap
    VarName -- ^ hashmap to modify
    VarName -- ^ key
    VarName -- ^ value
  | DeleteHashMap
    VarName -- ^ hashmap to modify
    VarName -- ^ key to delete
  | IterateOverHashMap
    VarName                        -- ^ hashmap to loop over
    (VarName -> VarName -> [Action]) -- ^ body of loop

  | CreateTrie
    VarName -- ^ trie to create
  | InsertTrie
    VarName -- ^ trie to modify
    VarName -- ^ key (string)
    VarName -- ^ value
  | DeleteTrie
    VarName -- ^ trie to modify
    VarName -- ^ key (string)

newtype TypeName = TypeName String
newtype VarName = VarName { getVarName :: String }
  deriving newtype (Eq, IsString)

--------------------------------------------------------------------------------

capFirst :: String -> String
capFirst = \case
  [] -> []
  (c : cs) -> Char.toUpper c : cs

--------------------------------------------------------------------------------

-- Suppose we have
--   A = Join [] B C
-- and we insert a row R into B
-- then we want to find any row S in C
-- and insert S ⋈ R into A.

-- Suppose we have
--   A = Join [i] B C
-- and we insert a row R into B
-- then we want to find any row S in C such that S[i] = R[i]
-- and insert S ⋈ᵢ R into A.

-- Suppose we have
--   A = Join [i, j] B C
-- and we insert a row R into B
-- then we want to find any row S in C such that S[i] = R[i] and S[j] = R[j]
-- and insert S ⋈ᵢⱼ R into A.

-- Suppose we have
--   A = Union B C
-- and we want to insert a row R into B
-- then we want to insert R into A

-- Suppose we have
--   A = Difference B C
-- and we want to insert a row R into B
-- then we want to insert R into A

-- Suppose we have
--   A = Difference B C
-- and we want to insert a row R into C
-- then we want to delete R from A

-- Suppose we have
--   A = Select P B
-- and we want to insert a row R into B
-- then we check if R satisfies P, and if so, insert it into A

-- Suppose we have
--   A = Map F B
-- and we want to insert a row R into B
-- then we apply F to R and insert the result into A

--------------------------------------------------------------------------------

-- Suppose we have
--   A = Join attrs B C
-- and we delete a row R from B
-- then we want to find any row S in A
-- where supposing that the attributes of R are x₁, ..., xₙ
-- and S.x₁ = R.x₁, ..., S.xₙ = R.xₙ
-- then we should should delete S from A

-- Suppose we have
--   A = Union B C
-- and we want to delete a row R from B
-- then we check if R is in C, and if not, delete R from A

-- Suppose we have
--   A = Difference B C
-- and we want to delete a row R from B
-- then we delete R from A

-- Suppose we have
--   A = Difference B C
-- and we want to delete a row R from C
-- then if B contains R, we add R to A

-- Suppose we have
--   A = Select P B
-- and we want to delete a row R from B
-- then we check if R satisfies P, and if so, delete it from A

-- Suppose we have
--   A = Map F B
-- and we want to delete a row R from B
-- then we delete F(R) from A

--------------------------------------------------------------------------------
