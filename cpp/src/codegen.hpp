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

#ifndef RDSS_CODEGEN_H_
#define RDSS_CODEGEN_H_

#include "macros.hpp"

#include <absl/container/btree_map.h>
#include <absl/status/statusor.h>
#include <absl/strings/str_cat.h>

namespace rdss {

struct RelationCode {
    int32_t member;
    int32_t insertion_method;
    int32_t deletion_method;
};

struct Codegen {
    Codegen() : table_relations(), view_relations() {}

    absl::btree_map<std::string, RelationCode> table_relations;
    absl::btree_map<Relation*, RelationCode> view_relations;

    absl::Status Run(DataStructure* ds,
                     Relation* rel,
                     FreshVariableSource* source,
                     const absl::btree_map<Relation*, Type*>& typing_context) {
        if (view_relations.contains(rel)) {
            return absl::OkStatus();
        }

        if (auto r = DynamicCast<Relation, RelationReference>(rel)) {
            std::string name = r.value()->name;
            Type* type = typing_context.at(rel);

            if (table_relations.contains(name)) {
                view_relations[rel] = table_relations.at(name);
                return absl::OkStatus();
            }

            RelationCode rel_code;

            {
                int32_t member = ds->members.size();
                ds->members.push_back(
                    Member { VarName(name), new TypeHashSet(type) });
                rel_code.member = member;
            }

            {
                int32_t insertion_method = ds->methods.size();
                ds->methods.push_back(
                    Method(VarName(absl::StrCat(name, "_insert"))));
                ds->methods.back().arguments.push_back(
                    { VarName("tuple"), type });
                ds->methods.back().body.push_back(
                    new ActionInsertHashSet(VarName(name), VarName("tuple")));
                rel_code.insertion_method = insertion_method;
            }

            {
                int32_t deletion_method = ds->methods.size();
                ds->methods.push_back(
                    Method(VarName(absl::StrCat(name, "_delete"))));
                ds->methods.back().arguments.push_back(
                    { VarName("tuple"), type });
                ds->methods.back().body.push_back(
                    new ActionDeleteHashSet(VarName(name), VarName("tuple")));
                rel_code.deletion_method = deletion_method;
            }

            table_relations[name] = rel_code;
            view_relations[rel] = rel_code;
        } else if (auto r = DynamicCast<Relation, RelationJoin>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationSemijoin>(rel)) {
            auto lhs = r.value()->lhs;
            auto rhs = r.value()->rhs;

            RETURN_IF_ERROR(this->Run(ds, lhs, source, typing_context));
            RETURN_IF_ERROR(this->Run(ds, rhs, source, typing_context));

            // FIXME: finish implementing this case
        } else if (auto r = DynamicCast<Relation, RelationUnion>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationDifference>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationSelect>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationMap>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationView>(rel)) {
            // FIXME: implement this case
        } else {
            return absl::InternalError(
                "If this is reached, a new relation op has been added but no "
                "case was added to the codegen. Please add one.");
        }
        return absl::OkStatus();
    }
};

}  // namespace rdss

#endif  // RDSS_CODEGEN_H_
