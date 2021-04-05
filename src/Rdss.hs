{-# LANGUAGE DeriveDataTypeable  #-}
{-# LANGUAGE DeriveFoldable      #-}
{-# LANGUAGE DeriveFunctor       #-}
{-# LANGUAGE DeriveGeneric       #-}
{-# LANGUAGE DeriveTraversable   #-}
{-# LANGUAGE DerivingStrategies  #-}
{-# LANGUAGE ImportQualifiedPost #-}
{-# LANGUAGE LambdaCase          #-}
{-# LANGUAGE PackageImports      #-}
{-# LANGUAGE ScopedTypeVariables #-}

module Rdss where

import "base" Control.Monad
import "base" Data.Data (Data)
import "base" Data.List qualified as List
import "base" GHC.Generics (Generic)
import "base" Numeric.Natural
import "containers" Data.Map.Strict (Map)
import "containers" Data.Map.Strict qualified as Map
import "containers" Data.Set (Set)
import "mtl" Control.Monad.State.Class (get, modify, put)
import "mtl" Control.Monad.State.Strict (State)

--------------------------------------------------------------------------------

type Attr = Int

type AttrPermutation = [Attr]

data Constant
  = ConstantInt Int
  | ConstantBool Bool
  | ConstantBitString [Bool]
  deriving stock (Eq, Ord, Show, Generic, Data)

data Function rel
  = Function -- invariant: must have number of arguments equal to number of columns in the table
      String -- ^ name of function in scope

data Predicate rel
  = PredicateAnd (RelAlgebra rel) (RelAlgebra rel)
  | PredicateOr (RelAlgebra rel) (RelAlgebra rel)
  | PredicateNot (RelAlgebra rel)
  | PredicateLike (RelAlgebra rel) (RelAlgebra rel)
  | PredicateLT (RelAlgebra rel) (RelAlgebra rel)
  | PredicateEQ (RelAlgebra rel) (RelAlgebra rel)

--------------------------------------------------------------------------------

data RelAlgebra rel
  = Not rel
  | Join Natural rel rel
  | Union rel rel
  | Project [Attr] rel -- TODO: should be (Set Attr)
  | Rename AttrPermutation rel
  | Difference rel rel
  | Select Attr (Predicate rel) rel
  | Map (Function rel) rel
  deriving stock (Eq, Ord, Show, Generic)
  deriving stock (Functor, Foldable, Traversable)

--------------------------------------------------------------------------------
