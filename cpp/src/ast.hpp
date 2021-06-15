#ifndef RDSS_AST_H_
#define RDSS_AST_H_

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <absl/container/flat_hash_set.h>
#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
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
    Viewed<Relation*> rel;

    explicit RelationNot(const Viewed<Relation*>& rel_) : rel(rel_) {}

    std::string ToString() const override {
        return absl::StrFormat("Not(%s)", rel.ToString());
    }

    int32_t Arity() const override { return rel.Arity(); }
};

struct RelationJoin : public Relation {
    Viewed<Relation*> lhs;
    Viewed<Relation*> rhs;
    absl::flat_hash_set<std::pair<Attr, Attr>> attributes;

    RelationJoin(
        const Viewed<Relation*>& lhs_,
        const Viewed<Relation*>& rhs_,
        const absl::flat_hash_set<std::pair<Attr, Attr>>& attributes_)
        : lhs(lhs_), rhs(rhs_), attributes(attributes_) {}

    std::string ToString() const override {
        std::vector<std::string> attribute_strings;
        for (const auto& [x, y] : this->attributes) {
            attribute_strings.push_back(absl::StrFormat("(%d, %d)", x, y));
        }
        return absl::StrFormat("Join([%s], %s, %s)",
                               absl::StrJoin(attribute_strings, ", "),
                               lhs.ToString(),
                               rhs.ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs.Arity();
        int32_t rhs_arity = rhs.Arity();
        int32_t result_arity = lhs_arity + rhs_arity - attributes.size();
        if (result_arity < 0) {
            // throw "type error got past the typechecker";
        }
        return result_arity;
    }
};

struct RelationSemijoin : public Relation {
    Viewed<Relation*> lhs;
    Viewed<Relation*> rhs;
    absl::flat_hash_set<std::pair<Attr, Attr>> attributes;

    RelationSemijoin(
        const Viewed<Relation*>& lhs_,
        const Viewed<Relation*>& rhs_,
        const absl::flat_hash_set<std::pair<Attr, Attr>>& attributes_)
        : lhs(lhs_), rhs(rhs_), attributes(attributes_) {}

    std::string ToString() const override {
        std::vector<std::string> attribute_strings;
        for (const auto& [x, y] : this->attributes) {
            attribute_strings.push_back(absl::StrFormat("(%d, %d)", x, y));
        }
        return absl::StrFormat("Semijoin([%s], %s, %s)",
                               absl::StrJoin(attribute_strings, ", "),
                               lhs.ToString(),
                               rhs.ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs.Arity();
        int32_t rhs_arity = rhs.Arity();
        int32_t result_arity = lhs_arity + rhs_arity - attributes.size();
        if (result_arity < 0) {
            // throw "type error got past the typechecker";
        }
        return result_arity;
    }
};

struct RelationUnion : public Relation {
    Viewed<Relation*> lhs;
    Viewed<Relation*> rhs;

    RelationUnion(const Viewed<Relation*>& lhs_,
                  const Viewed<Relation*>& rhs_)
        : lhs(lhs_), rhs(rhs_) {}

    std::string ToString() const override {
        return absl::StrFormat("Union(%s, %s)",
                               lhs.ToString(),
                               rhs.ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs.Arity();
        int32_t rhs_arity = rhs.Arity();
        if (lhs_arity != rhs_arity) {
            // throw "type error got past the typechecker";
        }
        return lhs_arity;
    }
};

struct RelationDifference : public Relation {
    Viewed<Relation*> lhs;
    Viewed<Relation*> rhs;

    RelationDifference(const Viewed<Relation*>& lhs_,
                       const Viewed<Relation*>& rhs_)
        : lhs(lhs_), rhs(rhs_) {}

    std::string ToString() const override {
        return absl::StrFormat("Difference(%s, %s)",
                               lhs.ToString(),
                               rhs.ToString());
    }

    int32_t Arity() const override {
        int32_t lhs_arity = lhs.Arity();
        int32_t rhs_arity = rhs.Arity();
        if (lhs_arity != rhs_arity) {
            // throw "type error got past the typechecker";
        }
        return lhs_arity;
    }
};

struct RelationSelect : public Relation {
    Predicate predicate;
    Viewed<Relation*> rel;

    RelationSelect(const Predicate& predicate_,
                   const Viewed<Relation*>& rel_)
        : predicate(predicate_), rel(rel_) {}

    std::string ToString() const override {
        // FIXME: add pretty-printing for predicates
        return absl::StrFormat("Select(<predicate>, %s)",
                               rel.ToString());
    }

    int32_t Arity() const override {
        return rel.Arity();
    }
};

struct RelationMap : public Relation {
    Function function;
    Viewed<Relation*> rel;

    RelationMap(const Function& function_,
                const Viewed<Relation*>& rel_)
        : function(function_), rel(rel_) {}

    std::string ToString() const override {
        return absl::StrFormat("Map(%s, %s)",
                               function.name,
                               rel.ToString());
    }

    int32_t Arity() const override {
        if (function.arguments != rel.Arity()) {
            // throw "type error got past the typechecker";
        }
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
