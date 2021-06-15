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

class Table {
public:
    using Value = int32_t;

    Table(int32_t width_) : width(width_), values() {}

    std::vector<Value> GetTuple(int32_t index) const {
        std::vector<Value> result;
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
        if ((values.size() % width) != 0) { throw; }
        return values.size() / width;
    }

    int32_t Width() const {
        return width;
    }

private:
    int32_t width;
    std::vector<int32_t> values;
};

class Interpreter {
public:
    Interpreter(const absl::btree_map<std::string, Table>& variables_)
        : variables(variables_) {}

    absl::Status Interpret(const Viewed<Relation*>& input);
    absl::Status Interpret(Relation* input);

    absl::optional<Table> Lookup(Relation* input) {
        if (context.contains(input)) {
            return context.at(input);
        }
        return absl::nullopt;
    }

private:
    absl::btree_map<std::string, Table> variables;
    absl::btree_map<Relation*, Table> context;
};

absl::Status Interpreter::Interpret(Relation* input) {
    if (auto r = DynamicCast<Relation, RelationReference>(input)) {
        context.insert_or_assign(input, variables.at(r.value()->name));
    } else if (auto r = DynamicCast<Relation, RelationNot>(input)) {
        RETURN_IF_ERROR(Interpret(r.value()->rel));
        return absl::InternalError("Interpreter cannot support Not");
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
                    std::vector<int32_t> result_tuple;
                    for (int32_t value : lhs_tuple) {
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
        absl::flat_hash_set<std::vector<int32_t>> restricted_rhs;
        Table result(lhs->Width());
        for (int32_t i = 0; i < rhs->NumberOfTuples(); i++) {
            auto tuple = rhs->GetTuple(i);
            std::vector<int32_t> restricted_tuple;
            for (const auto& [x, y] : r.value()->attributes) {
                restricted_tuple.push_back(tuple[y]);
            }
            restricted_rhs.insert(restricted_tuple);
        }
        for (int32_t i = 0; i < lhs->NumberOfTuples(); i++) {
            auto tuple = lhs->GetTuple(i);
            std::vector<int32_t> restricted_tuple;
            for (const auto& [x, y] : r.value()->attributes) {
                restricted_tuple.push_back(tuple[x]);
            }
            if (restricted_rhs.contains(restricted_tuple)) {
                RETURN_IF_ERROR(result.InsertTuple(tuple));
            }
        }
        context.insert_or_assign(input, result);
    } else if (auto r = DynamicCast<Relation, RelationUnion>(input)) {
        return absl::InternalError("Have not yet implemented Union");
    } else if (auto r = DynamicCast<Relation, RelationDifference>(input)) {
        return absl::InternalError("Have not yet implemented Difference");
    } else if (auto r = DynamicCast<Relation, RelationSelect>(input)) {
        return absl::InternalError("Have not yet implemented Select");
    } else if (auto r = DynamicCast<Relation, RelationMap>(input)) {
        return absl::InternalError("Interpreter cannot support Map");
    } else if (auto r = DynamicCast<Relation, RelationView>(input)) {
        return absl::InternalError("Have not yet implemented View");
    } else {
        return absl::InternalError(
            "If this is reached, a new relation op has been added but no case "
            "was added to the interpreter. Please add one.");
    }

    return absl::OkStatus();
}

absl::Status Interpreter::Interpret(const Viewed<Relation*>& input) {
    return absl::OkStatus();
}

}  // namespace rdss

#endif  // RDSS_INTERPRETER_H_
