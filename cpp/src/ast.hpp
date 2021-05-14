#ifndef RDSS_AST_H_
#define RDSS_AST_H_

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

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
};

struct VarName {
    std::string name;
};

struct TypeName {
    std::string name;
};

////////////////////////////////////////////////////////////////////////////////

struct Type {};

struct TypeInt : public Type {};

struct TypeBasic : public Type {
    TypeName name;
};

struct TypeRow : public Type {
    std::vector<Type> elements;
};

struct TypeHashSet : public Type {
    Type element;
};

struct TypeHashMap : public Type {
    Type key;
    Type value;
};

struct TypeTrie : public Type {
    Type key;
    Type value;
};

struct TypeVector : public Type {
    Type element;
};

////////////////////////////////////////////////////////////////////////////////

struct Action {};

struct ActionGetMember : public Action {
    VarName pointer;
    VarName struct_field;
};

struct ActionAssignConstant : public Action {
    VarName variable;
    std::string constant;
};

struct ActionCreateRow : public Action {
    VarName variable;
    std::vector<VarName> elements;
};

struct ActionIndexRow : public Action {
    VarName variable;
    VarName row_to_index;
    uint32_t index;
    bool has_size_1;
};

struct ActionInvoke : public Action {
    VarName method;
    std::vector<VarName> arguments;
};

struct ActionCreateHashSet : public Action {
    VarName variable;
};

struct ActionInsertHashSet : public Action {
    VarName hash_set;
    VarName value_to_insert;
};

struct ActionDeleteHashSet : public Action {
    VarName hash_set;
    VarName value_to_delete;
};

struct ActionIterateOverHashSet : public Action {
    VarName hash_set;
    std::function<std::vector<Action>(VarName)> body;
};

struct ActionCreateHashMap : public Action {
    VarName variable;
};

struct ActionInsertHashMap : public Action {
    VarName hash_map;
    VarName key_to_insert;
    VarName value_to_insert;
};

struct ActionDeleteHashMap : public Action {
    VarName hash_map;
    VarName key_to_delete;
};

struct ActionIterateOverHashMap : public Action {
    VarName hash_map;
    std::function<std::vector<Action>(VarName, VarName)> body;
};

struct ActionCreateTrie : public Action {
    VarName variable;
};

struct ActionInsertTrie : public Action {
    VarName trie;
    VarName key_to_insert;
    VarName value_to_insert;
};

struct ActionDeleteTrie : public Action {
    VarName trie;
    VarName key_to_delete;
};

////////////////////////////////////////////////////////////////////////////////

struct Member {
    VarName name;
    Type type;
};

////////////////////////////////////////////////////////////////////////////////

struct Method {
    VarName name;
    std::vector<std::pair<VarName, Type>> arguments;
    std::vector<Action> body;
};

////////////////////////////////////////////////////////////////////////////////

struct DataStructure {
    std::string name;
    std::vector<TypeParameter> type_parameters;
    std::vector<Member> members;
    std::vector<Method> methods;
};

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_AST_H_
