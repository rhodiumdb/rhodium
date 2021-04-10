{-# language
    DataKinds
  , DeriveDataTypeable
  , DeriveGeneric
  , DeriveTraversable
  , DerivingStrategies
  , DuplicateRecordFields
  , ImportQualifiedPost
  , LambdaCase
  , PackageImports
  , RecordWildCards
  , ScopedTypeVariables
  , StrictData
  , TypeApplications
#-}

module Rdss where

import "base" Control.Monad
import "base" Data.Char qualified as Char
import "base" Data.Data (Data)
import "base" Data.List qualified as List
import "base" GHC.Generics (Generic, Generic1)
import "base" Numeric.Natural
import "containers" Data.Map.Strict (Map)
import "containers" Data.Map.Strict qualified as Map
import "containers" Data.Set (Set)
import "generic-lens" Data.Generics.Product (field)
import "ilist" Data.List.Index (iforM_)
import "lens" Control.Lens ((^.))
import "mtl" Control.Monad.State.Class (get, modify, put)
import "mtl" Control.Monad.State.Strict (State)
import "transformers" Control.Monad.Trans.Writer.CPS (WriterT, Writer, execWriter, tell)

--------------------------------------------------------------------------------

type Attr = Int

type AttrPermutation = [Attr]

type AttrPartialPermutation = [Maybe Attr]

data Constant
  = ConstantInt Int
  | ConstantBool Bool
  | ConstantBitString [Bool]
  deriving stock (Eq, Ord, Show, Generic, Data)

-- invariant: must have number of arguments equal to number of columns in the table
data Function rel
  = Function
      String -- ^ name of function in scope
  deriving stock (Eq, Ord, Show, Generic, Generic1)
  deriving stock (Functor, Foldable, Traversable)

data Viewed rel
  = Viewed AttrPartialPermutation rel
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
          , methods = []
          }
        ]
  mapM_ (putStrLn . toHaskell) ds

toHaskell :: DataStructure -> String
toHaskell DataStructure{..} = execWriter go
  where
    go :: Writer String ()
    go = do
      let indentSpaces = 2
      let indent s = replicate indentSpaces ' ' ++ s

      tell $ "data " ++ capFirst name
      forM_ tyParams $ \(TyParam tyParam) -> do
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
      _ -> error "TODO"

    haskellMethod :: Method -> String
    haskellMethod = error "TODO"

    line :: Monad m => String -> WriterT String m ()
    line s = tell (s ++ "\n")

    newline :: Monad m => WriterT String m ()
    newline = line ""

data DataStructure
  = DataStructure
    { name :: String
    , tyParams :: [TyParam]
    , members :: [Member]
    , methods :: [Method]
    }

newtype TyParam = TyParam String

data Member
  = Member
    { name :: String
    , typ :: Type
    }
  deriving stock (Generic)

data Type
  = TyBasic   TypeName
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

data Action
  = Temp
  | AssignConstant
    VarName  -- ^ variable to assign to
    Constant -- ^ constant to assign
  | IndexRow
    VarName -- ^ variable to assign to
    VarName -- ^ row to index into
    Natural -- ^ index to use

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
    (VarName -> Action) -- ^ body of loop

  | CreateHashMap
    VarName -- ^ hashmap to create
  | InsertHashMap
    VarName -- ^ hashmap to modify
    VarName -- ^ key
    VarName -- ^ value
  | DeleteHashMap
    VarName -- ^ hashmap to modify
    VarName -- ^ key to delete
  | IterateOverHashMap
    VarName                        -- ^ hashmap to loop over
    (VarName -> VarName -> Action) -- ^ body of loop

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
newtype VarName = VarName String

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
