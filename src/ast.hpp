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

#ifndef RDSS_AST_H_
#define RDSS_AST_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include <absl/container/btree_set.h>
#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/types/optional.h>

#include "attr.hpp"
#include "predicate.hpp"
#include "logging/logging.hpp"

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct Viewed {
    AttrPartialPermutation perm;
    T rel;

    Viewed(const T& rel_) : rel(rel_), perm() {
        for (int32_t i = 0; i < rel_->Arity(); i++) {
            perm.push_back(i);
        }
    }

    Viewed(const AttrPartialPermutation& perm_, const T& rel_)
        : rel(rel_), perm(perm_) {}

    std::string ToString() const {
        std::vector<std::string> strings;
        for (const auto& attr_maybe : perm) {
            if (attr_maybe.has_value()) {
                strings.push_back(absl::StrFormat("%d", attr_maybe.value()));
            } else {
                strings.push_back("ø");
            }
        }
        std::string perm_string =
            absl::StrFormat("[%s]", absl::StrJoin(strings, ", "));
        if (Viewed(rel).perm == perm) {
            return rel->ToString();
        }
        return absl::StrFormat("Viewed(%s, %s)", perm_string, rel->ToString());
    }

    int32_t Arity() const {
        int32_t result = 0;
        for (const auto& attr_maybe : perm) {
            if (attr_maybe.has_value()) {
                result++;
            }
        }
        return result;
    }

    bool IsLocal() const {
        return rel->IsLocal();
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Function {
    std::string name;
    int32_t arguments;
    int32_t results;
};

////////////////////////////////////////////////////////////////////////////////

struct Relation {
    virtual std::string ToString() const = 0;
    virtual int32_t Arity() const = 0;
    virtual bool IsLocal() const = 0;
    virtual ~Relation() = default;
};

struct RelationFactory {
    std::vector<std::unique_ptr<Relation>> relations;

    template<typename T, typename... Args>
    T* Make(Args&&... args) {
        std::unique_ptr<T> value =
            absl::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = value.get();
        relations.push_back(std::move(value));
        return ptr;
    }
};

struct RelName {
    std::string name;

    RelName(absl::string_view name_) : name(name_) {}

    std::string ToString() const {
        return name;
    }

    auto operator<=>(const RelName&) const = default;
};


template<typename S, typename T>
absl::optional<T*> DynamicCast(S* pointer) {
    T* casted = dynamic_cast<T*>(pointer);
    if (casted == nullptr) { return absl::nullopt; }
    return casted;
}

struct RelationReference : public Relation {
    RelName name;
    int32_t arity;
    bool local;

    RelationReference(RelName name_, int32_t arity_, bool local_ = false)
        : name(name_), arity(arity_), local(local_) {}

    RelationReference(
        absl::string_view name_, int32_t arity_, bool local_ = false)
        : name(name_), arity(arity_), local(local_) {}

    std::string ToString() const override {
        return name.ToString();
    }

    int32_t Arity() const override {
        return arity;
    }

    bool IsLocal() const override {
        return local;
    }
};

using JoinOn = absl::btree_set<std::pair<Attr, Attr>>;

struct RelationJoin : public Relation {
    Relation* lhs;
    Relation* rhs;
    JoinOn attributes;

    RelationJoin(
        Relation* lhs_,
        Relation* rhs_,
        const JoinOn& attributes_)
        : lhs(lhs_), rhs(rhs_), attributes(attributes_) {}

    std::string ToString() const override {
        std::vector<std::string> attribute_strings;
        for (const auto& [x, y] : this->attributes) {
            attribute_strings.push_back(absl::StrFormat("(%d, %d)", x, y));
        }
        return absl::StrFormat("Join([%s], %s, %s)",
                               absl::StrJoin(attribute_strings, ", "),
                               lhs->ToString(),
                               rhs->ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs->Arity();
        int32_t rhs_arity = rhs->Arity();
        int32_t result_arity = lhs_arity + rhs_arity - attributes.size();
        RDSS_CHECK_GE(result_arity, 0) << "type error got past the typechecker";
        return result_arity;
    }

    bool IsLocal() const override {
        return lhs->IsLocal() || rhs->IsLocal();
    }
};

struct RelationSemijoin : public Relation {
    Relation* lhs;
    Relation* rhs;
    JoinOn attributes;

    RelationSemijoin(
        Relation* lhs_,
        Relation* rhs_,
        const JoinOn& attributes_)
        : lhs(lhs_), rhs(rhs_), attributes(attributes_) {}

    std::string ToString() const override {
        std::vector<std::string> attribute_strings;
        for (const auto& [x, y] : this->attributes) {
            attribute_strings.push_back(absl::StrFormat("(%d, %d)", x, y));
        }
        return absl::StrFormat("Semijoin([%s], %s, %s)",
                               absl::StrJoin(attribute_strings, ", "),
                               lhs->ToString(),
                               rhs->ToString());
    }

    int32_t Arity() const override {
        return lhs->Arity();
    }

    bool IsLocal() const override {
        return lhs->IsLocal() || rhs->IsLocal();
    }
};

struct RelationUnion : public Relation {
    Relation* lhs;
    Relation* rhs;

    RelationUnion(Relation* lhs_,
                  Relation* rhs_)
        : lhs(lhs_), rhs(rhs_) {}

    std::string ToString() const override {
        return absl::StrFormat("Union(%s, %s)",
                               lhs->ToString(),
                               rhs->ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs->Arity();
        int32_t rhs_arity = rhs->Arity();
        RDSS_CHECK_EQ(lhs_arity, rhs_arity)
            << "type error got past the typechecker";
        return lhs_arity;
    }

    bool IsLocal() const override {
        return lhs->IsLocal() || rhs->IsLocal();
    }
};

struct RelationDifference : public Relation {
    Relation* lhs;
    Relation* rhs;

    RelationDifference(Relation* lhs_,
                       Relation* rhs_)
        : lhs(lhs_), rhs(rhs_) {}

    std::string ToString() const override {
        return absl::StrFormat("Difference(%s, %s)",
                               lhs->ToString(),
                               rhs->ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs->Arity();
        int32_t rhs_arity = rhs->Arity();
        RDSS_CHECK_EQ(lhs_arity, rhs_arity)
            << "type error got past the typechecker";
        return lhs_arity;
    }

    bool IsLocal() const override {
        return lhs->IsLocal() || rhs->IsLocal();
    }
};

struct RelationSelect : public Relation {
    Predicate* predicate;
    Relation* rel;

    RelationSelect(Predicate* predicate_,
                   Relation* rel_)
        : predicate(predicate_), rel(rel_) {}

    std::string ToString() const override {
        // FIXME: add pretty-printing for predicates
        return absl::StrFormat("Select(<predicate>, %s)",
                               rel->ToString());
    }

    int32_t Arity() const override {
        return rel->Arity();
    }

    bool IsLocal() const override {
        return rel->IsLocal();
    }
};

struct RelationMap : public Relation {
    Function function;
    Relation* rel;

    RelationMap(const Function& function_,
                Relation* rel_)
        : function(function_), rel(rel_) {}

    std::string ToString() const override {
        return absl::StrFormat("Map(%s, %s)",
                               function.name,
                               rel->ToString());
    }

    int32_t Arity() const override {
        RDSS_CHECK_EQ(function.arguments, rel->Arity())
            << "type error got past the typechecker";
        return function.results;
    }

    bool IsLocal() const override {
        return rel->IsLocal();
    }
};

struct RelationView : public Relation {
    Viewed<Relation*> rel;

    RelationView(const Viewed<Relation*>& rel_) : rel(rel_) {}

    std::string ToString() const override {
        return absl::StrFormat("View(%s)",
                               rel.ToString());
    }

    int32_t Arity() const override {
        return rel.Arity();
    }

    bool IsLocal() const override {
        return rel.IsLocal();
    }
};

////////////////////////////////////////////////////////////////////////////////

struct RAction {
    virtual ~RAction() = default;
};

struct RSeq : public RAction {
    std::vector<RAction*> body;

    explicit RSeq(const std::vector<RAction*>& body_) : body(body_) {}
};

struct RUnionWith : public RAction {
    RelName name;
    Relation* relation;

    RUnionWith(RelName name_, Relation* relation_)
        : name(name_), relation(relation_) {}
};

struct RFor : public RAction {
    Relation* relation;
    RelName variable;
    std::vector<RAction*> body;

    RFor(Relation* relation_,
         RelName variable_,
         std::vector<RAction*> body_)
        : relation(relation_), variable(variable_), body(body_) {}
};

struct RReturn : public RAction {
    Relation* relation;

    explicit RReturn(Relation* relation_) : relation(relation_) {}
};

////////////////////////////////////////////////////////////////////////////////

struct TypeParameter {
    std::string name;

    TypeParameter(absl::string_view name_) : name(name_) {}

    std::string ToCpp() const { return this->name; }
};

struct VarName {
    std::string name;

    VarName(absl::string_view name_) : name(name_) {}

    std::string ToCpp() const { return this->name; }
};

struct TypeName {
    std::string name;

    TypeName(absl::string_view name_) : name(name_) {}

    std::string ToCpp() const { return this->name; }
};

struct FreshVariableSource {
    FreshVariableSource() : var_number(0), rel_number(0) {}

    VarName Fresh() {
        std::string result = absl::StrFormat("fresh%d", var_number);
        var_number++;
        return VarName(result);
    }

    RelName FreshRel() {
        std::string result = absl::StrFormat("Rel%d", rel_number);
        rel_number++;
        return RelName(result);
    }

private:
    int32_t var_number;
    int32_t rel_number;
};

////////////////////////////////////////////////////////////////////////////////

struct Type {
    virtual std::string ToCpp() const = 0;
    virtual ~Type() = default;
};

struct TypeInt : public Type {
    TypeInt() {}

    std::string ToCpp() const override {
        return "int32_t";
    }
};

struct TypeBasic : public Type {
    TypeName name;

    TypeBasic(TypeName name_) : name(name_) {}

    std::string ToCpp() const override {
        return this->name.ToCpp();
    }
};

struct TypeRow : public Type {
    std::vector<Type*> elements;

    TypeRow(absl::Span<Type* const> elements_)
        : elements(elements_.begin(), elements_.end()) {}

    std::string ToCpp() const override {
        std::vector<std::string> types;
        for (const auto& type : this->elements) {
            types.push_back(type->ToCpp());
        }
        return absl::StrFormat("std::tuple<%s>", absl::StrJoin(types, ", "));
    }
};

struct TypeHashSet : public Type {
    Type* element;

    TypeHashSet(Type* element_) : element(element_) {}

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_set<",
                            this->element->ToCpp(), ">");
    }
};

struct TypeBag : public Type {
    Type* element;

    TypeBag(Type* element_) : element(element_) {}

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_map<",
                            this->element->ToCpp(), ", int32_t>");
    }
};

struct TypeHashMap : public Type {
    Type* key;
    Type* value;

    TypeHashMap(Type* key_, Type* value_) : key(key_), value(value_) {}

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_map<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeTrie : public Type {
    Type* key;
    Type* value;

    TypeTrie(Type* key_, Type* value_) : key(key_), value(value_) {}

    std::string ToCpp() const override {
        return absl::StrCat("trie<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeVector : public Type {
    Type* element;

    TypeVector(Type* element_) : element(element_) {}

    std::string ToCpp() const override {
        return absl::StrCat("std::vector<", this->element->ToCpp(), ">");
    }
};

////////////////////////////////////////////////////////////////////////////////

inline std::string Indent(absl::string_view str, int32_t n) {
    std::vector<absl::string_view> lines = absl::StrSplit(str, '\n');
    std::string indent;
    indent.resize(4 * n, ' ');
    std::string result;
    for (absl::string_view line : lines) {
        if (!line.empty()) {
            absl::StrAppend(&result, indent, line, "\n");
        }
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////

// TODO: created Typed<T> to streamline access to type information

struct Action {
    virtual std::string ToCpp(FreshVariableSource* source) const = 0;
};

struct ActionGetMember : public Action {
    VarName variable;
    VarName pointer;
    VarName struct_field;

    ActionGetMember(VarName variable_, VarName pointer_, VarName struct_field_)
        : variable(variable_), pointer(pointer_), struct_field(struct_field_)
        {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s = %s.%s;",
                               variable.ToCpp(),
                               pointer.ToCpp(),
                               struct_field.ToCpp());
    }
};

struct ActionIfEquals : public Action {
    std::vector<std::pair<VarName, VarName>> equalities;
    std::vector<Action*> body;

    ActionIfEquals(absl::Span<std::pair<VarName, VarName> const> equalities_,
                   absl::Span<Action* const> body_)
        : equalities(equalities_.begin(), equalities_.end())
        , body(body_.begin(), body_.end()) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        std::vector<std::string> equality_strings;
        for (const auto& [x, y] : this->equalities) {
            equality_strings.push_back(absl::StrFormat("(%s == %s)",
                                                       x.ToCpp(),
                                                       y.ToCpp()));
        }
        std::string body_string;
        for (const auto& action : this->body) {
            absl::StrAppend(
                &body_string, Indent(action->ToCpp(source), 1), "\n");
        }
        return absl::StrFormat("if (%s) {\n%s}",
                               absl::StrJoin(equality_strings, " || "),
                               body_string);
    }
};

struct ActionAssignConstant : public Action {
    VarName variable;
    std::string constant;

    ActionAssignConstant(VarName variable_, absl::string_view constant_)
        : variable(variable_), constant(constant_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("auto %s = %s;", variable.ToCpp(), constant);
    }
};

struct ActionCreateRow : public Action {
    VarName variable;
    std::vector<std::pair<VarName, Type*>> elements;

    explicit ActionCreateRow(VarName variable_)
        : variable(variable_), elements() {}

    ActionCreateRow(VarName variable_,
                    absl::Span<const std::pair<VarName, Type*>> elements_)
        : variable(variable_), elements(elements_.begin(), elements_.end()) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        std::vector<std::string> element_strings;
        std::vector<std::string> type_strings;
        for (const auto& [element, type] : elements) {
            element_strings.push_back(element.ToCpp());
            type_strings.push_back(type->ToCpp());
        }
        return absl::StrFormat("std::tuple<%s> %s { %s };",
                               absl::StrJoin(type_strings, ", "),
                               variable.ToCpp(),
                               absl::StrJoin(element_strings, ", "));
    }
};

struct ActionIndexRow : public Action {
    VarName variable;
    VarName row_to_index;
    uint32_t index;

    ActionIndexRow(VarName variable_, VarName row_to_index_, uint32_t index_)
        : variable(variable_), row_to_index(row_to_index_), index(index_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("auto %s = std::get<%d>(%s);",
                               variable.ToCpp(),
                               index,
                               row_to_index.ToCpp());
    }
};

struct ActionInvoke : public Action {
    VarName method;
    std::vector<VarName> arguments;

    ActionInvoke(VarName method_, absl::Span<const VarName> arguments_)
        : method(method_), arguments(arguments_.begin(), arguments_.end()) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        std::vector<std::string> argument_strings;
        for (const VarName& argument : arguments) {
            argument_strings.push_back(argument.ToCpp());
        }
        return absl::StrFormat("%s(%s);",
                               method.ToCpp(),
                               absl::StrJoin(argument_strings, ", "));
    }
};

struct ActionCreateHashSet : public Action {
    VarName variable;
    Type* type;

    ActionCreateHashSet(VarName variable_, Type* type_)
        : variable(variable_), type(type_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("absl::flat_hash_set<%s> %s;",
                               type->ToCpp(),
                               variable.ToCpp());
    }
};

struct ActionInsertHashSet : public Action {
    VarName hash_set;
    VarName value_to_insert;

    ActionInsertHashSet(VarName hash_set_, VarName value_to_insert_)
        : hash_set(hash_set_), value_to_insert(value_to_insert_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.insert(%s);",
                               hash_set.ToCpp(),
                               value_to_insert.ToCpp());
    }
};

struct ActionDeleteHashSet : public Action {
    VarName hash_set;
    VarName value_to_delete;

    ActionDeleteHashSet(VarName hash_set_, VarName value_to_delete_)
        : hash_set(hash_set_), value_to_delete(value_to_delete_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.erase(%s);",
                               hash_set.ToCpp(),
                               value_to_delete.ToCpp());
    }
};

struct ActionIterateOverHashSet : public Action {
    VarName hash_set;
    std::function<std::vector<Action*>(VarName)> body;

    ActionIterateOverHashSet(VarName hash_set_,
                             std::function<std::vector<Action*>(VarName)> body_)
        : hash_set(hash_set_), body(body_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        auto value = source->Fresh();
        std::string body_string;
        for (const auto& action : this->body(value)) {
            absl::StrAppend(
                &body_string, Indent(action->ToCpp(source), 1), "\n");
        }
        return absl::StrFormat("for (const auto& %s : %s) {\n%s}",
                               value.ToCpp(),
                               hash_set.ToCpp(),
                               body_string);
    }
};

struct ActionContainsHashSet : public Action {
    VarName variable;
    VarName hash_set;
    VarName value;

    ActionContainsHashSet(VarName variable_, VarName hash_set_, VarName value_)
        : variable(variable_), hash_set(hash_set_), value(value_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("bool %s = %s.contains(%s);",
                               variable.ToCpp(),
                               hash_set.ToCpp(),
                               value.ToCpp());
    }
};

struct ActionCreateHashMap : public Action {
    VarName variable;
    Type* key_type;
    Type* value_type;

    ActionCreateHashMap(VarName variable_, Type* key_type_, Type* value_type_)
        : variable(variable_), key_type(key_type_), value_type(value_type_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("absl::flat_hash_map<%s, %s> %s;",
                               key_type->ToCpp(),
                               value_type->ToCpp(),
                               variable.ToCpp());
    }
};

struct ActionInsertHashMap : public Action {
    VarName hash_map;
    VarName key_to_insert;
    VarName value_to_insert;

    ActionInsertHashMap(VarName hash_map_,
                        VarName key_to_insert_,
                        VarName value_to_insert_)
        : hash_map(hash_map_)
        , key_to_insert(key_to_insert_)
        , value_to_insert(value_to_insert_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.insert_or_assign(%s, %s);",
                               hash_map.ToCpp(),
                               key_to_insert.ToCpp(),
                               value_to_insert.ToCpp());
    }
};

struct ActionDeleteHashMap : public Action {
    VarName hash_map;
    VarName key_to_delete;

    ActionDeleteHashMap(VarName hash_map_,
                        VarName key_to_delete_)
        : hash_map(hash_map_), key_to_delete(key_to_delete_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.erase(%s);",
                               hash_map.ToCpp(),
                               key_to_delete.ToCpp());
    }
};

struct ActionIterateOverHashMap : public Action {
    VarName hash_map;
    std::function<std::vector<Action*>(VarName, VarName)> body;

    ActionIterateOverHashMap(
        VarName hash_map_,
        std::function<std::vector<Action*>(VarName, VarName)> body_)
        : hash_map(hash_map_), body(body_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        auto key = source->Fresh();
        auto value = source->Fresh();
        std::string body_string;
        for (const auto& action : this->body(key, value)) {
            absl::StrAppend(&body_string, action->ToCpp(source), "\n");
        }
        return absl::StrFormat("for (const auto& [%s, %s] : %s) {\n%s}",
                               key.ToCpp(),
                               value.ToCpp(),
                               hash_map.ToCpp(),
                               body_string);
    }
};

struct ActionCreateTrie : public Action {
    VarName variable;
    Type* key_type;
    Type* value_type;

    ActionCreateTrie(VarName variable_, Type* key_type_, Type* value_type_)
        : variable(variable_), key_type(key_type_), value_type(value_type_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("Trie<%s, %s> %s;",
                               key_type->ToCpp(),
                               value_type->ToCpp(),
                               variable.ToCpp());
    }
};

struct ActionInsertTrie : public Action {
    VarName trie;
    VarName key_to_insert;
    VarName value_to_insert;

    ActionInsertTrie(VarName trie_,
                     VarName key_to_insert_,
                     VarName value_to_insert_)
        : trie(trie_)
        , key_to_insert(key_to_insert_)
        , value_to_insert(value_to_insert_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.Insert(%s, %s);",
                               this->trie.ToCpp(),
                               this->key_to_insert.ToCpp(),
                               this->value_to_insert.ToCpp());
    }
};

struct ActionDeleteTrie : public Action {
    VarName trie;
    VarName key_to_delete;

    ActionDeleteTrie(VarName trie_,
                     VarName key_to_delete_)
        : trie(trie_), key_to_delete(key_to_delete_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.Delete(%s)",
                               this->trie.ToCpp(),
                               this->key_to_delete.ToCpp());
    }
};

struct ActionCreateBag : public Action {
    VarName variable;
    Type* value_type;

    ActionCreateBag(VarName variable_, Type* value_type_)
        : variable(variable_), value_type(value_type_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("absl::flat_hash_map<%s, int32_t> %s;",
                               value_type->ToCpp(),
                               variable.ToCpp());
    }
};

struct ActionIncrementBag : public Action {
    VarName bag;
    VarName value_to_insert;

    ActionIncrementBag(VarName bag_, VarName value_to_insert_)
        : bag(bag_)
        , value_to_insert(value_to_insert_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        auto bag_cpp = this->bag.ToCpp();
        auto value_cpp = this->value_to_insert.ToCpp();
        return absl::StrFormat(
            "if (%s.contains(%s)) { %s[%s]++; } else { %s[%s] = 1; }",
            bag_cpp, value_cpp, bag_cpp, value_cpp, bag_cpp, value_cpp);
    }
};

struct ActionDecrementBag : public Action {
    VarName bag;
    VarName value_to_delete;

    ActionDecrementBag(VarName bag_, VarName value_to_delete_)
        : bag(bag_), value_to_delete(value_to_delete_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        auto bag_cpp = this->bag.ToCpp();
        auto value_cpp = this->value_to_delete.ToCpp();

        return absl::StrFormat(
            "if (%s.contains(%s)) { %s[%s]--; if (%s[%s] < 0) %s.erase(%s); }",
            bag_cpp, value_cpp,
            bag_cpp, value_cpp,
            bag_cpp, value_cpp,
            bag_cpp, value_cpp);
    }
};

struct ActionIterateOverBag : public Action {
    VarName bag;
    std::function<std::vector<Action*>(VarName)> body;

    ActionIterateOverBag(VarName bag_,
                         std::function<std::vector<Action*>(VarName)> body_)
        : bag(bag_), body(body_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        auto value = source->Fresh();
        std::string body_string;
        for (const auto& action : this->body(value)) {
            absl::StrAppend(
                &body_string, Indent(action->ToCpp(source), 1), "\n");
        }
        return absl::StrFormat("for (const auto& [%s, %s] : %s) {\n%s}",
                               value.ToCpp(),
                               // multiplicities should be invisible
                               source->Fresh().name,
                               bag.ToCpp(),
                               body_string);
    }
};

struct ActionContainsBag : public Action {
    VarName variable;
    VarName bag;
    VarName value;

    ActionContainsBag(VarName variable_, VarName bag_, VarName value_)
        : variable(variable_), bag(bag_), value(value_) {}

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("bool %s = %s.contains(%s);",
                               variable.ToCpp(),
                               bag.ToCpp(),
                               value.ToCpp());
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Member {
    VarName name;
    Type* type;

    std::string ToCpp(FreshVariableSource* source) const;
};

////////////////////////////////////////////////////////////////////////////////

struct Method {
    VarName name;
    std::vector<std::pair<VarName, Type*>> arguments;
    std::vector<Action*> body;

    explicit Method(VarName name_) : name(name_), arguments(), body() {}

    std::string ToCpp(FreshVariableSource* source) const;
};

////////////////////////////////////////////////////////////////////////////////

struct DataStructure {
    std::string name;
    std::vector<TypeParameter> type_parameters;
    std::vector<Member> members;
    std::vector<Method> methods;

    explicit DataStructure(std::string name_)
        : name(name_), type_parameters(), members(), methods() {}

    std::string ToCpp(FreshVariableSource* source) const;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_AST_H_
