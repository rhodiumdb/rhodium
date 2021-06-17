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
#include <sstream>
#include <string>
#include <vector>

#include <absl/container/btree_set.h>
#include <absl/container/flat_hash_set.h>
#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/types/optional.h>

#include "logging/logging.hpp"

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

using Attr = int32_t;
using AttrPermutation = std::vector<Attr>;
using AttrPartialPermutation = std::vector<absl::optional<Attr>>;

template<typename T>
struct Viewed {
    AttrPartialPermutation perm;
    T rel;

    Viewed(const T& rel_) : rel(rel_), perm() {
        for (int32_t i = 0; i < rel_->Arity(); i++) {
            perm.push_back(i);
        }
    }

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
};

////////////////////////////////////////////////////////////////////////////////

struct Function {
    std::string name;
    int32_t arguments;
    int32_t results;
};

////////////////////////////////////////////////////////////////////////////////

struct Predicate {
    virtual std::string ToString() const = 0;
    virtual ~Predicate() = default;
};

struct PredicateFactory {
    std::vector<std::unique_ptr<Predicate>> predicates;

    template<typename T, typename... Args>
    T* Make(Args&&... args) {
        std::unique_ptr<T> value =
            absl::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = value.get();
        predicates.push_back(std::move(value));
        return ptr;
    }
};

struct PredicateAnd : public Predicate {
    std::vector<Predicate*> children;

    explicit PredicateAnd(
        absl::Span<Predicate* const> children_)
        : children(children_.begin(), children_.end()) {}

    std::string ToString() const override {
        std::vector<std::string> child_strings;
        for (Predicate* child : children) {
            child_strings.push_back(child->ToString());
        }
        return absl::StrCat("(", absl::StrJoin(child_strings, " && "), ")");
    }
};

struct PredicateOr : public Predicate {
    std::vector<Predicate*> children;

    explicit PredicateOr(
        absl::Span<Predicate* const> children_)
        : children(children_.begin(), children_.end()) {}

    std::string ToString() const override {
        std::vector<std::string> child_strings;
        for (Predicate* child : children) {
            child_strings.push_back(child->ToString());
        }
        return absl::StrCat("(", absl::StrJoin(child_strings, " || "), ")");
    }
};

struct PredicateNot : public Predicate {
    Predicate* pred;

    explicit PredicateNot(Predicate* pred_)
        : pred(pred_) {}

    std::string ToString() const override {
        return absl::StrCat("!", pred->ToString());
    }
};

struct PredicateLike : public Predicate {
    Attr attr;
    std::string string;

    PredicateLike(Attr attr_, const std::string& string_)
        : attr(attr_), string(string_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d LIKE \"%s\")", attr, string);
    }
};

struct PredicateLessThan : public Predicate {
    Attr attr;
    int32_t integer;

    PredicateLessThan(Attr attr_, int32_t integer_)
        : attr(attr_), integer(integer_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d < %d)", attr, integer);
    }
};

struct PredicateEquals : public Predicate {
    Attr attr;
    int32_t integer;

    PredicateEquals(Attr attr_, int32_t integer_)
        : attr(attr_), integer(integer_) {}

    std::string ToString() const override {
        return absl::StrFormat("(attr%d ≡ %d)", attr, integer);
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Relation {
    virtual std::string ToString() const = 0;
    virtual int32_t Arity() const = 0;
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

template<typename S, typename T>
absl::optional<T*> DynamicCast(S* pointer) {
    T* casted = dynamic_cast<T*>(pointer);
    if (casted == nullptr) { return absl::nullopt; }
    return casted;
}

struct RelationReference : public Relation {
    std::string name;
    int32_t arity;

    explicit RelationReference(absl::string_view name_, int32_t arity_)
        : name(name_), arity(arity_) {}

    std::string ToString() const override {
        return name;
    }

    int32_t Arity() const override {
        return arity;
    }
};

struct RelationNot : public Relation {
    Relation* rel;

    explicit RelationNot(Relation* rel_) : rel(rel_) {}

    std::string ToString() const override {
        return absl::StrFormat("Not(%s)", rel->ToString());
    }

    int32_t Arity() const override { return rel->Arity(); }
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
};

////////////////////////////////////////////////////////////////////////////////

struct TypeParameter {
    std::string name;

    TypeParameter(absl::string_view name_) : name(name_.begin(), name_.end()) {}

    std::string ToCpp() const { return this->name; }
};

struct VarName {
    std::string name;

    VarName(absl::string_view name_) : name(name_.begin(), name_.end()) {}

    std::string ToCpp() const { return this->name; }
};

struct TypeName {
    std::string name;

    TypeName(absl::string_view name_) : name(name_.begin(), name_.end()) {}

    std::string ToCpp() const { return this->name; }
};

struct FreshVariableSource {
    FreshVariableSource() : number(0) {}

    VarName Fresh() {
        std::string result = absl::StrFormat("fresh%d", number);
        number++;
        return VarName(result);
    }

private:
    int32_t number;
};

////////////////////////////////////////////////////////////////////////////////

struct Type {
    virtual std::string ToCpp() const = 0;
    virtual ~Type() = default;
};

struct TypeInt : public Type {
    std::string ToCpp() const override {
        return "int32_t";
    }
};

struct TypeBasic : public Type {
    TypeName name;

    std::string ToCpp() const override {
        return this->name.ToCpp();
    }
};

struct TypeRow : public Type {
    std::vector<Type*> elements;

    std::string ToCpp() const override {
        std::stringstream ss;
        ss << "std::tuple<";
        std::vector<std::string> types;
        for (const auto& type : this->elements) {
            types.push_back(type->ToCpp());
        }
        ss << absl::StrJoin(types, ", ") << ">";
        return ss.str();
    }
};

struct TypeHashSet : public Type {
    Type* element;

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_set<",
                            this->element->ToCpp(), ">");
    }
};

struct TypeHashMap : public Type {
    Type* key;
    Type* value;

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_map<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeTrie : public Type {
    Type* key;
    Type* value;

    std::string ToCpp() const override {
        return absl::StrCat("trie<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeVector : public Type {
    Type* element;

    std::string ToCpp() const override {
        return absl::StrCat("std::vector<", this->element->ToCpp(), ">");
    }
};

////////////////////////////////////////////////////////////////////////////////

// TODO: created Typed<T> to streamline access to type information

struct Action {
    virtual std::string ToCpp(FreshVariableSource* source) const = 0;
};

struct ActionGetMember : public Action {
    VarName variable;
    VarName pointer;
    VarName struct_field;

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s = %s.%s;",
                               variable.ToCpp(),
                               pointer.ToCpp(),
                               struct_field.ToCpp());
    }
};

struct ActionAssignConstant : public Action {
    VarName variable;
    std::string constant;

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s = %s;", variable.ToCpp(), constant);
    }
};

struct ActionCreateRow : public Action {
    VarName variable;
    std::vector<std::pair<VarName, Type*>> elements;

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

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("absl::flat_hash_set<%s> %s;",
                               type->ToCpp(),
                               variable.ToCpp());
    }
};

struct ActionInsertHashSet : public Action {
    VarName hash_set;
    VarName value_to_insert;

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.insert(%s);",
                               hash_set.ToCpp(),
                               value_to_insert.ToCpp());
    }
};

struct ActionDeleteHashSet : public Action {
    VarName hash_set;
    VarName value_to_delete;

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.erase(%s);",
                               hash_set.ToCpp(),
                               value_to_delete.ToCpp());
    }
};

struct ActionIterateOverHashSet : public Action {
    VarName hash_set;
    std::function<std::vector<Action*>(VarName)> body;

    std::string ToCpp(FreshVariableSource* source) const override {
        auto value = source->Fresh();
        std::string body_string;
        for (const auto& action : this->body(value)) {
            absl::StrAppend(&body_string, action->ToCpp(source), "\n");
        }
        return absl::StrFormat("for (const auto& %s : %s) {%s}",
                               value.ToCpp(),
                               hash_set.ToCpp(),
                               body_string);
    }
};

struct ActionCreateHashMap : public Action {
    VarName variable;
    Type* key_type;
    Type* value_type;

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

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.erase(%s);",
                               hash_map.ToCpp(),
                               key_to_delete.ToCpp());
    }
};

struct ActionIterateOverHashMap : public Action {
    VarName hash_map;
    std::function<std::vector<Action*>(VarName, VarName)> body;

    std::string ToCpp(FreshVariableSource* source) const override {
        auto key = source->Fresh();
        auto value = source->Fresh();
        std::string body_string;
        for (const auto& action : this->body(key, value)) {
            absl::StrAppend(&body_string, action->ToCpp(source), "\n");
        }
        return absl::StrFormat("for (const auto& [%s, %s] : %s) { %s }",
                               key.ToCpp(),
                               value.ToCpp(),
                               hash_map.ToCpp(),
                               body_string);
    }
};

struct ActionCreateTrie : public Action {
    VarName variable;

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("Trie<auto> %s;",
                               this->variable.ToCpp());
    }
};

struct ActionInsertTrie : public Action {
    VarName trie;
    VarName key_to_insert;
    VarName value_to_insert;

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

    std::string ToCpp(FreshVariableSource* source) const override {
        return absl::StrFormat("%s.Delete(%s)",
                               this->trie.ToCpp(),
                               this->key_to_delete.ToCpp());
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
