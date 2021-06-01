#ifndef RDSS_AST_H_
#define RDSS_AST_H_

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/types/optional.h>

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

using Attr = int32_t;
using AttrPermutation = std::vector<Attr>;
using AttrPartialPermutation = std::vector<absl::optional<Attr>>;

template<typename T>
struct Viewed {
    AttrPartialPermutation perm;
    T rel;
};

////////////////////////////////////////////////////////////////////////////////

struct Function {
    std::string name;
};

////////////////////////////////////////////////////////////////////////////////

struct Predicate {
};

struct PredicateAnd : public Predicate {
    Predicate lhs;
    Predicate rhs;
};

struct PredicateOr : public Predicate {
    Predicate lhs;
    Predicate rhs;
};

struct PredicateNot : public Predicate {
    Predicate rel;
};

struct PredicateLike : public Predicate {
    Attr attr;
    std::string string;
};

struct PredicateLessThan : public Predicate {
    Attr attr;
    int32_t integer;
};

struct PredicateEquals : public Predicate {
    Attr attr;
    int32_t integer;
};

////////////////////////////////////////////////////////////////////////////////

struct Relation {
};

template<typename Child>
struct RelationNot : public Relation {
    Viewed<Child> rel;
};

template<typename Child>
struct RelationJoin : public Relation {
    Viewed<Child> lhs;
    Viewed<Child> rhs;
    int32_t overlapping;
};

template<typename Child>
struct RelationUnion : public Relation {
    Viewed<Child> lhs;
    Viewed<Child> rhs;
};

template<typename Child>
struct RelationDifference : public Relation {
    Viewed<Child> lhs;
    Viewed<Child> rhs;
};

template<typename Child>
struct RelationSelect : public Relation {
    Predicate predicate;
    Viewed<Child> rel;
};

template<typename Child>
struct RelationMap : public Relation {
    Function function;
    Viewed<Child> rel;
};

template<typename Child>
struct RelationView : public Relation {
    Viewed<Child> rel;
};

////////////////////////////////////////////////////////////////////////////////

struct TypeParameter {
    std::string name;

    TypeParameter(std::string name) : name(name) {}
    TypeParameter(const char* name) : name(name) {}

    std::string ToCpp() const { return this->name; }
};

struct VarName {
    std::string name;

    VarName(std::string name) : name(name) {}
    VarName(const char* name) : name(name) {}

    std::string ToCpp() const { return this->name; }
};

struct TypeName {
    std::string name;

    TypeName(std::string name) : name(name) {}
    TypeName(const char* name) : name(name) {}

    std::string ToCpp() const { return this->name; }
};

////////////////////////////////////////////////////////////////////////////////

struct Type {
    virtual std::string ToCpp() const = 0;
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
    std::vector<Type> elements;

    std::string ToCpp() const override {
        std::stringstream ss;
        ss << "std::tuple<";
        std::vector<std::string> types;
        for (const auto& type : this->elements) {
            types.push_back(type.ToCpp());
        }
        ss << absl::StrJoin(types, ", ") << ">";
        return ss.str();
    }
};

struct TypeHashSet : public Type {
    std::unique_ptr<Type> element;

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_set<",
                            this->element->ToCpp(), ">");
    }
};

struct TypeHashMap : public Type {
    std::unique_ptr<Type> key;
    std::unique_ptr<Type> value;

    std::string ToCpp() const override {
        return absl::StrCat("absl::flat_hash_map<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeTrie : public Type {
    std::unique_ptr<Type> key;
    std::unique_ptr<Type> value;

    std::string ToCpp() const override {
        return absl::StrCat("trie<",
                            this->key->ToCpp(), ", ",
                            this->value->ToCpp(), ">");
    }
};

struct TypeVector : public Type {
    std::unique_ptr<Type> element;

    std::string ToCpp() const override {
        return absl::StrCat("std::vector<", this->element->ToCpp(), ">");
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Action {
    virtual std::string ToCpp() const = 0;
};

struct ActionGetMember : public Action {
    VarName pointer;
    VarName struct_field;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionAssignConstant : public Action {
    VarName variable;
    std::string constant;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionCreateRow : public Action {
    VarName variable;
    std::vector<VarName> elements;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionIndexRow : public Action {
    VarName variable;
    VarName row_to_index;
    uint32_t index;
    bool has_size_1;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionInvoke : public Action {
    VarName method;
    std::vector<VarName> arguments;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionCreateHashSet : public Action {
    VarName variable;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionInsertHashSet : public Action {
    VarName hash_set;
    VarName value_to_insert;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionDeleteHashSet : public Action {
    VarName hash_set;
    VarName value_to_delete;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionIterateOverHashSet : public Action {
    VarName hash_set;
    std::function<std::vector<Action>(VarName)> body;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionCreateHashMap : public Action {
    VarName variable;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionInsertHashMap : public Action {
    VarName hash_map;
    VarName key_to_insert;
    VarName value_to_insert;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionDeleteHashMap : public Action {
    VarName hash_map;
    VarName key_to_delete;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionIterateOverHashMap : public Action {
    VarName hash_map;
    std::function<std::vector<Action>(VarName, VarName)> body;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionCreateTrie : public Action {
    VarName variable;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionInsertTrie : public Action {
    VarName trie;
    VarName key_to_insert;
    VarName value_to_insert;

    std::string ToCpp() const {
        return "";
    }
};

struct ActionDeleteTrie : public Action {
    VarName trie;
    VarName key_to_delete;

    std::string ToCpp() const {
        return "";
    }
};

////////////////////////////////////////////////////////////////////////////////

struct Member {
    VarName name;
    std::unique_ptr<Type> type;

    std::string ToCpp() const;
};

////////////////////////////////////////////////////////////////////////////////

struct Method {
    VarName name;
    std::vector<std::pair<VarName, std::unique_ptr<Type>>> arguments;
    std::vector<Action> body;

    explicit Method(VarName name) : name(name) {}

    std::string ToCpp() const;
};

////////////////////////////////////////////////////////////////////////////////

struct DataStructure {
    std::string name;
    std::vector<TypeParameter> type_parameters;
    std::vector<Member> members;
    std::vector<Method> methods;

    explicit DataStructure(std::string name) : name(name) {}

    std::string ToCpp() const;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_AST_H_
