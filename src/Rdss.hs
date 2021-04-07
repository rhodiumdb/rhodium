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
  = PredicateAnd (Predicate rel) (Predicate rel)
  | PredicateOr (Predicate rel) (Predicate rel)
  | PredicateNot (Predicate rel)
  | PredicateLike Attr String
  | PredicateLT Attr Int
  | PredicateEQ Attr Int

--------------------------------------------------------------------------------

data Viewed rel
  = Viewed ![Maybe Attr] !rel
  deriving stock (Eq, Ord, Show, Generic)
  deriving stock (Functor, Foldable, Traversable)

data RelAlgebra rel
  = Not                        (Viewed rel)
  | Join       Natural         (Viewed rel) (Viewed rel)
  | Union                      (Viewed rel) (Viewed rel)
  | Difference                 (Viewed rel) (Viewed rel)
  | Select     (Predicate rel) (Viewed rel)
  | Map        (Function rel)  (Viewed rel)
  | View                       (Viewed rel)
  deriving stock (Eq, Ord, Show, Generic)
  deriving stock (Functor, Foldable, Traversable)

--------------------------------------------------------------------------------
