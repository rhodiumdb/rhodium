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
    Codegen(absl::string_view name,
            FreshVariableSource* source_,
            const TypingContext& typing_context_)
        : table_relations()
        , view_relations()
        , ds(std::string(name))
        , source(source_)
        , typing_context(typing_context_) {}

    absl::btree_map<RelName, RelationCode> table_relations;
    absl::btree_map<Relation*, RelationCode> view_relations;
    DataStructure ds;
    FreshVariableSource* source;
    TypingContext typing_context;

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
                absl::Span<int32_t const> element_indices) {
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

    absl::Status ProcessRelationReference(RelationReference* rel) {
        RelName name = rel->name;
        Type* type = typing_context.at(rel);

        if (table_relations.contains(name)) {
            view_relations[rel] = table_relations.at(name);
            return absl::OkStatus();
        }

        RelationCode rel_code = SimpleRelationCode(name.name, type);

        table_relations[name] = rel_code;
        view_relations[rel] = rel_code;

        return absl::OkStatus();
    }

    absl::Status ProcessRelationJoin(RelationJoin* rel) {
        // FIXME: implement this
        return absl::OkStatus();
    }

    absl::Status ProcessRelationSemijoin(RelationSemijoin* rel) {
        VarName rel_name = source->Fresh();
        Type* rel_type = typing_context.at(rel);

        view_relations[rel] = SimpleRelationCode(rel_name.name, rel_type);

        auto lhs = rel->lhs;
        auto rhs = rel->rhs;
        auto join_on = rel->attributes;

        RETURN_IF_ERROR(this->ProcessRelation(lhs));
        RETURN_IF_ERROR(this->ProcessRelation(rhs));

        {
            VarName restricted_lhs = source->Fresh();

            InsertionOfView(lhs)->body +=
                FilterTuple(restricted_lhs,
                            {VarName("tuple"), typing_context.at(lhs)},
                            LHSIndices(join_on)).first;

            InsertionOfView(lhs)->body.push_back(
                new ActionIterateOverHashSet {
                    MemberOfView(rhs)->name,
                    [=, this](VarName tuple) -> std::vector<Action*> {
                        std::vector<Action*> result;

                        VarName restricted_rhs = source->Fresh();

                        result += FilterTuple(restricted_rhs,
                                              {tuple, typing_context.at(rhs)},
                                              RHSIndices(join_on)).first;

                        result.push_back(
                            new ActionIfEquals(
                                {{restricted_lhs, restricted_rhs}},
                                {new ActionInvoke(InsertionOfView(rel)->name,
                                                  {VarName("tuple")})}));
                        return result;
                    }
                });
        }

        {
            VarName restricted_rhs = source->Fresh();

            InsertionOfView(rhs)->body +=
                FilterTuple(restricted_rhs,
                            {VarName("tuple"), typing_context.at(rhs)},
                            RHSIndices(join_on)).first;

            InsertionOfView(rhs)->body.push_back(
                new ActionIterateOverHashSet {
                    MemberOfView(lhs)->name,
                    [=, this](VarName tuple) -> std::vector<Action*> {
                        std::vector<Action*> result;

                        VarName restricted_lhs = source->Fresh();

                        result += FilterTuple(restricted_lhs,
                                              {tuple, typing_context.at(lhs)},
                                              LHSIndices(join_on)).first;

                        result.push_back(
                            new ActionIfEquals(
                                {{restricted_lhs, restricted_rhs}},
                                {new ActionInvoke(InsertionOfView(rel)->name,
                                                  {tuple})}));

                        return result;
                    }
                });
        }

        // FIXME: implement deletion for semijoins

        return absl::OkStatus();
    }

    absl::Status ProcessRelationUnion(RelationUnion* rel) {
        view_relations[rel] = SimpleRelationCode(source->Fresh().name,
                                                 typing_context.at(rel));

        auto lhs = rel->lhs;
        auto rhs = rel->rhs;

        RETURN_IF_ERROR(this->ProcessRelation(lhs));
        RETURN_IF_ERROR(this->ProcessRelation(rhs));

        InsertionOfView(lhs)->body.push_back(
            new ActionInvoke(InsertionOfView(rel)->name, {VarName("tuple")}));

        InsertionOfView(rhs)->body.push_back(
            new ActionInvoke(InsertionOfView(rel)->name, {VarName("tuple")}));

        // FIXME: implement deletions

        return absl::OkStatus();
    }

    absl::Status ProcessRelationDifference(RelationDifference* rel) {
        view_relations[rel] = SimpleRelationCode(source->Fresh().name,
                                                 typing_context.at(rel));

        auto lhs = rel->lhs;
        auto rhs = rel->rhs;

        RETURN_IF_ERROR(this->ProcessRelation(lhs));
        RETURN_IF_ERROR(this->ProcessRelation(rhs));

        InsertionOfView(lhs)->body.push_back(
            new ActionInvoke(InsertionOfView(rel)->name, {VarName("tuple")}));

        InsertionOfView(rhs)->body.push_back(
            new ActionInvoke(DeletionOfView(rel)->name, {VarName("tuple")}));

        DeletionOfView(lhs)->body.push_back(
            new ActionInvoke(DeletionOfView(rel)->name, {VarName("tuple")}));

        auto contains_var = source->Fresh();
        auto true_var = source->Fresh();
        DeletionOfView(rhs)->body += {
            new ActionContainsHashSet(
                contains_var, MemberOfView(lhs)->name, {VarName("tuple")}),
            new ActionAssignConstant(true_var, "true"),
            new ActionIfEquals({{contains_var, true_var}},
                               {new ActionInvoke(InsertionOfView(rel)->name,
                                                 {VarName("tuple")})})
        };

        return absl::OkStatus();
    }

    absl::Status ProcessRelationSelect(RelationSelect* rel) {
        return absl::OkStatus(); // FIXME: implement this case
    }

    absl::Status ProcessRelationMap(RelationMap* rel) {
        return absl::OkStatus(); // FIXME: implement this case
    }

    absl::Status ProcessRelationView(RelationView* rel) {
        view_relations[rel] = SimpleRelationCode(source->Fresh().name,
                                                 typing_context.at(rel));

        auto perm = rel->rel.perm;
        auto underlying = rel->rel.rel;

        RETURN_IF_ERROR(this->ProcessRelation(underlying));

        std::vector<std::pair<VarName, Type*>> viewed_elements;
        viewed_elements.resize(rel->rel.Arity(), {VarName(""), nullptr});
        int32_t i = 0;
        for (std::optional<Attr> attr_maybe : perm) {
            if (attr_maybe.has_value()) {
                VarName elem = source->Fresh();
                InsertionOfView(underlying)->body.push_back(
                    new ActionIndexRow(elem, VarName("tuple"), i));
                Type* type =
                    DynamicCast<Type, TypeRow>(typing_context.at(underlying))
                    .value()->elements.at(i);
                viewed_elements.at(attr_maybe.value()) = {elem, type};
            }
            i++;
        }
        VarName output = source->Fresh();
        InsertionOfView(underlying)->body += {
            new ActionCreateRow(output, viewed_elements),
            new ActionInvoke(InsertionOfView(rel)->name, {output})
        };

        // FIXME: implement deletions for views

        return absl::OkStatus();
    }

    absl::Status ProcessRelation(Relation* rel) {
        if (view_relations.contains(rel)) {
            return absl::OkStatus();
        }

        if (auto r = DynamicCast<Relation, RelationReference>(rel)) {
            RETURN_IF_ERROR(ProcessRelationReference(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationJoin>(rel)) {
            RETURN_IF_ERROR(ProcessRelationJoin(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationSemijoin>(rel)) {
            RETURN_IF_ERROR(ProcessRelationSemijoin(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationUnion>(rel)) {
            RETURN_IF_ERROR(ProcessRelationUnion(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationDifference>(rel)) {
            RETURN_IF_ERROR(ProcessRelationDifference(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationSelect>(rel)) {
            RETURN_IF_ERROR(ProcessRelationSelect(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationMap>(rel)) {
            RETURN_IF_ERROR(ProcessRelationMap(r.value()));
        } else if (auto r = DynamicCast<Relation, RelationView>(rel)) {
            RETURN_IF_ERROR(ProcessRelationView(r.value()));
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
