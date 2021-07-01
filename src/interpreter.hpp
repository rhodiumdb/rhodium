// Copyright 2021 The RDSS Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RDSS_INTERPRETER_H_
#define RDSS_INTERPRETER_H_

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/memory/memory.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/types/optional.h>

#include "ast.hpp"
#include "macros.hpp"

namespace rdss {

using Value = int32_t;

using Tuple = std::vector<Value>;

class Table {
public:
    Table(int32_t width_) : width(width_), values() {}

    Tuple GetTuple(int32_t index) const {
        Tuple result;
        for (int32_t i = 0; i < width; i++) {
            result.push_back(values.at(index * width + i));
        }
        return result;
    }

    absl::Status InsertTuple(absl::Span<const Value> tuple) {
        if (tuple.size() != width) {
            return absl::InternalError(
                "given tuple does not match table width");
        }
        for (const Value& val : tuple) {
            values.push_back(val);
        }
        return absl::OkStatus();
    }

    int32_t NumberOfTuples() const {
        RDSS_CHECK_EQ(values.size() % width, 0);
        return values.size() / width;
    }

    int32_t Width() const {
        return width;
    }

private:
    int32_t width;
    std::vector<Value> values;
};

bool InterpretPredicate(Predicate* predicate,
                        const Tuple& tuple) {
    if (auto p = DynamicCast<Predicate, PredicateAnd>(predicate)) {
        bool result = true;
        for (Predicate* child : p.value()->children) {
            result &= InterpretPredicate(child, tuple);
        }
        return result;
    } else if (auto p = DynamicCast<Predicate, PredicateOr>(predicate)) {
        bool result = false;
        for (Predicate* child : p.value()->children) {
            result |= InterpretPredicate(child, tuple);
        }
        return result;
    } else if (auto p = DynamicCast<Predicate, PredicateNot>(predicate)) {
        return !InterpretPredicate(p.value()->pred, tuple);
    } else if (auto p = DynamicCast<Predicate, PredicateLike>(predicate)) {
        RDSS_CHECK(false) << "InterpretPredicate does not yet support LIKE";
    } else if (auto p = DynamicCast<Predicate, PredicateLessThan>(predicate)) {
        return tuple[p.value()->attr] < p.value()->integer;
    } else if (auto p = DynamicCast<Predicate, PredicateEquals>(predicate)) {
        return tuple[p.value()->attr] == p.value()->integer;
    } else {
        RDSS_CHECK(false)
            << "If this is reached, a new predicate has been added but no "
            << "case was added to the interpreter. Please add one.";
    }
}

class Interpreter {
public:
    Interpreter(const absl::btree_map<RelName, Table>& variables_)
        : variables(variables_) {}

    absl::Status Interpret(Relation* input);

    absl::optional<Table> Lookup(Relation* input) {
        if (context.contains(input)) {
            return context.at(input);
        }
        return absl::nullopt;
    }

private:
    absl::btree_map<RelName, Table> variables;
    absl::btree_map<Relation*, Table> context;
};

absl::Status Interpreter::Interpret(Relation* input) {
    if (auto r = DynamicCast<Relation, RelationReference>(input)) {
        context.insert_or_assign(input, variables.at(r.value()->name));
    } else if (auto r = DynamicCast<Relation, RelationJoin>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->lhs));
        RETURN_IF_ERROR(Interpret(r.value()->rhs));

        auto lhs = &context.at(r.value()->lhs);
        auto rhs = &context.at(r.value()->rhs);
        Table result(r.value()->Arity());
        for (int32_t i = 0; i < lhs->NumberOfTuples(); i++) {
            for (int32_t j = 0; j < rhs->NumberOfTuples(); j++) {
                auto lhs_tuple = lhs->GetTuple(i);
                auto rhs_tuple = rhs->GetTuple(j);
                bool add = true;
                absl::flat_hash_set<Attr> rhs_nonincluded;
                for (const auto& [x, y] : r.value()->attributes) {
                    if (lhs_tuple[x] != rhs_tuple[y]) {
                        add = false;
                        break;
                    }
                    rhs_nonincluded.insert(y);
                }
                if (add) {
                    Tuple result_tuple;
                    for (Value value : lhs_tuple) {
                        result_tuple.push_back(value);
                    }
                    for (int32_t k = 0; k < rhs->Width(); k++) {
                        if (!rhs_nonincluded.contains(k)) {
                            result_tuple.push_back(rhs_tuple[k]);
                        }
                    }
                    RETURN_IF_ERROR(result.InsertTuple(result_tuple));
                }
            }
        }
        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationSemijoin>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->lhs));
        RETURN_IF_ERROR(Interpret(r.value()->rhs));

        auto lhs = &context.at(r.value()->lhs);
        auto rhs = &context.at(r.value()->rhs);
        absl::flat_hash_set<Tuple> restricted_rhs;
        Table result(r.value()->Arity());
        for (int32_t i = 0; i < rhs->NumberOfTuples(); i++) {
            auto tuple = rhs->GetTuple(i);
            Tuple restricted_tuple;
            for (const auto& [x, y] : r.value()->attributes) {
                restricted_tuple.push_back(tuple[y]);
            }
            restricted_rhs.insert(restricted_tuple);
        }
        for (int32_t i = 0; i < lhs->NumberOfTuples(); i++) {
            auto tuple = lhs->GetTuple(i);
            Tuple restricted_tuple;
            for (const auto& [x, y] : r.value()->attributes) {
                restricted_tuple.push_back(tuple[x]);
            }
            if (restricted_rhs.contains(restricted_tuple)) {
                RETURN_IF_ERROR(result.InsertTuple(tuple));
            }
        }
        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationUnion>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->lhs));
        RETURN_IF_ERROR(Interpret(r.value()->rhs));

        auto lhs = &context.at(r.value()->lhs);
        auto rhs = &context.at(r.value()->rhs);

        Table result(r.value()->Arity());
        for (int32_t i = 0; i < lhs->NumberOfTuples(); i++) {
            RETURN_IF_ERROR(result.InsertTuple(lhs->GetTuple(i)));
        }
        for (int32_t i = 0; i < rhs->NumberOfTuples(); i++) {
            RETURN_IF_ERROR(result.InsertTuple(rhs->GetTuple(i)));
        }

        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationDifference>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->lhs));
        RETURN_IF_ERROR(Interpret(r.value()->rhs));

        auto lhs = &context.at(r.value()->lhs);
        auto rhs = &context.at(r.value()->rhs);


        absl::flat_hash_set<std::vector<Value>> tuples_in_rhs;
        for (int32_t i = 0; i < rhs->NumberOfTuples(); i++) {
            tuples_in_rhs.insert(rhs->GetTuple(i));
        }

        Table result(r.value()->Arity());
        for (int32_t i = 0; i < lhs->NumberOfTuples(); i++) {
            auto tuple = lhs->GetTuple(i);
            if (!tuples_in_rhs.contains(tuple)) {
                RETURN_IF_ERROR(result.InsertTuple(tuple));
            }
        }

        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationSelect>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->rel));

        auto predicate = r.value()->predicate;
        auto rel = &context.at(r.value()->rel);

        Table result(r.value()->Arity());
        for (int32_t i = 0; i < rel->NumberOfTuples(); i++) {
            auto tuple = rel->GetTuple(i);
            if (InterpretPredicate(predicate, tuple)) {
                RETURN_IF_ERROR(result.InsertTuple(tuple));
            }
        }

        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationMap>(input)) {
        return absl::InternalError("Interpreter cannot support Map");
    } else if (auto r = DynamicCast<Relation, RelationView>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->rel.rel));

        auto perm = r.value()->rel.perm;
        auto rel = &context.at(r.value()->rel.rel);

        Table result(r.value()->Arity());
        for (int32_t i = 0; i < rel->NumberOfTuples(); i++) {
            Tuple input_tuple = rel->GetTuple(i);
            Tuple output_tuple;
            output_tuple.resize(result.Width(), Value());
            int32_t j = 0;
            for (absl::optional<Attr> attr_maybe : perm) {
                if (attr_maybe) {
                    output_tuple[*attr_maybe] = input_tuple[j];
                }
                j++;
            }
            RETURN_IF_ERROR(result.InsertTuple(output_tuple));
        }

        context.insert_or_assign(input, result);
    } else {
        return absl::InternalError(
            "If this is reached, a new relation op has been added but no case "
            "was added to the interpreter. Please add one.");
    }

    return absl::OkStatus();
}

}  // namespace rdss

#endif  // RDSS_INTERPRETER_H_
