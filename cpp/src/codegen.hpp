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

namespace {

template<typename T>
std::vector<T>& operator+=(std::vector<T>& a, const std::vector<T>& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

}  // namespace

std::vector<Attr> LHSIndices(const JoinOn& join_on) {
    std::vector<Attr> result;
    for (auto [i, j] : join_on) {
        result.push_back(i);
    }
    return result;
}

std::vector<Attr> RHSIndices(const JoinOn& join_on) {
    std::vector<Attr> result;
    for (auto [i, j] : join_on) {
        result.push_back(j);
    }
    return result;
}

using TypingContext = absl::btree_map<Relation*, Type*>;

struct Codegen {
    Codegen(absl::string_view name)
        : table_relations(), view_relations(), ds(std::string(name)) {}

    absl::btree_map<std::string, RelationCode> table_relations;
    absl::btree_map<Relation*, RelationCode> view_relations;
    DataStructure ds;

    Member* MemberOfTable(absl::string_view name) {
        return &ds.members.at(table_relations.at(name).member);
    }

    Member* MemberOfView(Relation* rel) {
        return &ds.members.at(view_relations.at(rel).member);
    }

    Method* InsertionOfTable(absl::string_view name) {
        return &ds.methods.at(table_relations.at(name).insertion_method);
    }

    Method* InsertionOfView(Relation* rel) {
        return &ds.methods.at(view_relations.at(rel).insertion_method);
    }

    Method* DeletionOfTable(absl::string_view name) {
        return &ds.methods.at(table_relations.at(name).deletion_method);
    }

    Method* DeletionOfView(Relation* rel) {
        return &ds.methods.at(view_relations.at(rel).deletion_method);
    }

    RelationCode SimpleRelationCode(absl::string_view name,
                                    Type* type) {
        RelationCode rel_code;

        {
            int32_t member = ds.members.size();
            ds.members.push_back(
                Member { VarName(name), new TypeHashSet(type) });
            rel_code.member = member;
        }

        {
            int32_t insertion_method = ds.methods.size();
            ds.methods.push_back(
                Method(VarName(absl::StrCat(name, "_insert"))));
            ds.methods.back().arguments.push_back(
                { VarName("tuple"), type });
            ds.methods.back().body.push_back(
                new ActionInsertHashSet(VarName(name), VarName("tuple")));
            rel_code.insertion_method = insertion_method;
        }

        {
            int32_t deletion_method = ds.methods.size();
            ds.methods.push_back(
                Method(VarName(absl::StrCat(name, "_delete"))));
            ds.methods.back().arguments.push_back(
                { VarName("tuple"), type });
            ds.methods.back().body.push_back(
                new ActionDeleteHashSet(VarName(name), VarName("tuple")));
            rel_code.deletion_method = deletion_method;
        }

        return rel_code;
    }

    std::pair<std::vector<Action*>, Type*>
    FilterTuple(VarName output,
                std::pair<VarName, Type*> tuple,
                absl::Span<int32_t const> element_indices,
                FreshVariableSource* source) {
        std::vector<Action*> result;
        std::vector<Type*> type_vector;
        std::vector<std::pair<VarName, Type*>> restricted_elements;
        for (int32_t i : element_indices) {
            VarName elem = source->Fresh();
            result.push_back(new ActionIndexRow(elem, tuple.first, i));
            Type* type =
                DynamicCast<Type, TypeRow>(tuple.second)
                .value()->elements.at(i);
            type_vector.push_back(type);
            restricted_elements.push_back({elem, type});
        }
        result.push_back(new ActionCreateRow(output, restricted_elements));
        return {result, new TypeRow(type_vector)};
    }

    absl::Status Run(Relation* rel,
                     FreshVariableSource* source,
                     const TypingContext& typing_context) {
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

            RelationCode rel_code = SimpleRelationCode(name, type);

            table_relations[name] = rel_code;
            view_relations[rel] = rel_code;
        } else if (auto r = DynamicCast<Relation, RelationJoin>(rel)) {
            // FIXME: implement this case
        } else if (auto r = DynamicCast<Relation, RelationSemijoin>(rel)) {
            VarName rel_name = source->Fresh();
            Type* rel_type = typing_context.at(rel);

            view_relations[rel] = SimpleRelationCode(rel_name.name, rel_type);

            auto lhs = r.value()->lhs;
            auto rhs = r.value()->rhs;
            auto join_on = r.value()->attributes;

            RETURN_IF_ERROR(this->Run(lhs, source, typing_context));
            RETURN_IF_ERROR(this->Run(rhs, source, typing_context));

            auto lhs_member = ds.members.at(view_relations.at(lhs).member);
            auto rhs_member = ds.members.at(view_relations.at(rhs).member);

            {
                auto& lhs_insertion_method =
                    ds.methods.at(view_relations.at(lhs).insertion_method);

                VarName restricted_lhs = source->Fresh();

                lhs_insertion_method.body +=
                    FilterTuple(restricted_lhs,
                                {VarName("tuple"), typing_context.at(lhs)},
                                LHSIndices(join_on),
                                source).first;

                lhs_insertion_method.body.push_back(
                    new ActionIterateOverHashSet {
                        rhs_member.name,
                        [=, this](VarName tuple) -> std::vector<Action*> {
                            std::vector<Action*> result;

                            VarName restricted_rhs = source->Fresh();

                            result +=
                                FilterTuple(restricted_rhs,
                                            {tuple, typing_context.at(rhs)},
                                            RHSIndices(join_on),
                                            source).first;

                            result.push_back(
                                new ActionIfEquals(
                                    {{restricted_lhs, restricted_rhs}},
                                    {
                                        new ActionInvoke(
                                            ds.methods.at(view_relations.at(rel).insertion_method).name,
                                            {VarName("tuple")})
                                    }));
                            return result;
                        }
                    });
            }

            {
                auto& rhs_insertion_method =
                    ds.methods.at(view_relations.at(rhs).insertion_method);

                VarName restricted_rhs = source->Fresh();

                rhs_insertion_method.body +=
                    FilterTuple(restricted_rhs,
                                {VarName("tuple"), typing_context.at(rhs)},
                                RHSIndices(join_on),
                                source).first;

                rhs_insertion_method.body.push_back(
                    new ActionIterateOverHashSet {
                        lhs_member.name,
                        [=, this](VarName tuple) -> std::vector<Action*> {
                            std::vector<Action*> result;

                            VarName restricted_lhs = source->Fresh();

                            result +=
                                FilterTuple(restricted_lhs,
                                            {tuple, typing_context.at(lhs)},
                                            LHSIndices(join_on),
                                            source).first;

                            result.push_back(
                                new ActionIfEquals(
                                    {{restricted_lhs, restricted_rhs}},
                                    {
                                        new ActionInvoke(
                                            ds.methods.at(view_relations.at(rel).insertion_method).name,
                                            {tuple})
                                    }));
                            return result;
                        }
                    });
            }
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
