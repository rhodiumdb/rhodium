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

#ifndef RDSS_PARSER_H_
#define RDSS_PARSER_H_

#include <string>

#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>
#include <absl/status/statusor.h>
#include <absl/strings/string_view.h>
#include <absl/types/variant.h>
#include <tree_sitter/api.h>

namespace rdss {

namespace syntax {

using Span = std::pair<TSPoint, TSPoint>;

struct Node {
    absl::optional<Span> span;

    void SetSpan(const Span& span_) {
        this->span = span_;
    }

    virtual ~Node() = default;
};

////////////////////////////////////////////////////////////////////////////////

// Forward declarations

struct Type;
struct MetaItem;
struct Block;
struct If;
struct IfLet;

////////////////////////////////////////////////////////////////////////////////

struct Identifier : public Node {
    std::string name;

    explicit Identifier(absl::string_view name_) : name(name_) {}
};

struct TypeIdentifier : public Node {
    std::string name;

    explicit TypeIdentifier(absl::string_view name_) : name(name_) {}
};

struct FieldIdentifier : public Node {
    std::string name;

    explicit FieldIdentifier(absl::string_view name_) : name(name_) {}
};

struct LoopLabel : public Node {
    std::string name;

    explicit LoopLabel(absl::string_view name_) : name(name_) {}
};

////////////////////////////////////////////////////////////////////////////////

using TypeArguments = std::vector<Type*>;

struct Path : public Node {
    std::vector<absl::variant<Identifier, TypeArguments>> path;

    explicit Path(
        absl::Span<absl::variant<Identifier, TypeArguments> const> path_)
        : path(path_.begin(), path_.end()) {}
};

////////////////////////////////////////////////////////////////////////////////

struct Literal : public Node {
    absl::variant<std::string, char, bool, std::vector<bool>> literal;

    explicit Literal(absl::string_view string)
        : literal(std::string(string.begin(), string.end())) {}
    explicit Literal(char c) : literal(c) {}
    explicit Literal(bool b) : literal(b) {}
    explicit Literal(const std::vector<bool>& integer) : literal(integer) {}
};

////////////////////////////////////////////////////////////////////////////////

struct MetaArguments : public Node {
    std::vector<std::pair<MetaItem*, Literal*>> arguments;

    explicit MetaArguments(
        absl::Span<std::pair<MetaItem*, Literal*> const> arguments_)
        : arguments(arguments_.begin(), arguments_.end()) {}
};

struct MetaItem : public Node {
    Path path;
    absl::optional<absl::variant<Literal*, MetaArguments*>> value_or_arguments;

    explicit MetaItem(const Path& path_) : path(path_), value_or_arguments() {}
    explicit MetaItem(const Path& path_, Literal* literal_)
        : path(path_), value_or_arguments(literal_) {}
    explicit MetaItem(const Path& path_, MetaArguments* arguments_)
        : path(path_), value_or_arguments(arguments_) {}
};

////////////////////////////////////////////////////////////////////////////////

enum class Mutability {
    Immutable,
    Mutable
};

enum class Visibility {
    Private,
    Public
};

////////////////////////////////////////////////////////////////////////////////

struct Type : public Node {
};

struct GenericOrScopedType : public Type {
    Path path;

    explicit GenericOrScopedType(const Path& path_) : path(path_) {}
};

struct TupleType : public Type {
    std::vector<Type*> types;

    explicit TupleType(absl::Span<Type*> types_)
        : types(types_.begin(), types_.end()) {}
};

struct EmptyType : public Type {
};

struct ArrayType : public Type {
    Type* element;
    absl::optional<int32_t> length;

    explicit ArrayType(Type* element_, absl::optional<int32_t> length_)
        : element(element_), length(length_) {}
};

struct PointerType : public Type {
    Type* type;
    Mutability mutability;

    explicit PointerType(Type* type_, Mutability mutability_)
        : type(type_), mutability(mutability_) {}
};

struct IntegerType : public Type {
    bool is_unsigned;
    int32_t bits;

    explicit IntegerType(bool is_unsigned_, int32_t bits_)
        : is_unsigned(is_unsigned_), bits(bits_) {}
};

struct StringType : public Type {
};

struct BoolType : public Type {
};

////////////////////////////////////////////////////////////////////////////////

struct Pattern : public Node {
};

struct LiteralPattern : public Pattern {
    Literal* literal;

    explicit LiteralPattern(Literal* literal_) : literal(literal_) {}
};

struct VariablePattern : public Pattern {
    Identifier variable;

    explicit VariablePattern(Identifier variable_) : variable(variable_) {}
};

struct TuplePattern : public Pattern {
    std::vector<Pattern*> children;

    explicit TuplePattern(absl::Span<Pattern* const> children_)
        : children(children_.begin(), children_.end()) {}
};

struct StructPattern : public Pattern {
    absl::btree_map<FieldIdentifier*, Pattern*> fields;
    bool remaining_field;

    explicit StructPattern(
        absl::Span<std::pair<FieldIdentifier*, Pattern*> const> fields_,
        bool remaining_field_ = false)
        : fields(fields_.begin(), fields_.end())
        , remaining_field(remaining_field_) {}
};

struct Wildcard : public Pattern {
};

////////////////////////////////////////////////////////////////////////////////

struct Statement : public Node {
};

struct Declaration : public Statement {
};

struct Expression : public Statement {
};

////////////////////////////////////////////////////////////////////////////////

using Fields = absl::btree_map<FieldIdentifier*, Type*>;

struct IdentifierWithParameters : public Node {
    Identifier name;
    std::vector<TypeIdentifier> parameters;

    explicit IdentifierWithParameters(
        const Identifier& name_,
        absl::Span<TypeIdentifier const> parameters_)
        : name(name_), parameters(parameters_.begin(), parameters_.end()) {}
};

struct TypeIdentifierWithParameters : public Node {
    TypeIdentifier name;
    std::vector<TypeIdentifier> parameters;

    explicit TypeIdentifierWithParameters(
        const TypeIdentifier& name_,
        absl::Span<TypeIdentifier const> parameters_)
        : name(name_), parameters(parameters_.begin(), parameters_.end()) {}
};

struct Parameter : public Node {
    absl::optional<MetaItem*> attribute;
    Mutability mutability;
    Pattern* pattern;
    Type* type;

    explicit Parameter(absl::optional<MetaItem*> attribute_,
                       Mutability mutability_,
                       Pattern* pattern_,
                       Type* type_)
        : attribute(attribute_)
        , mutability(mutability_)
        , pattern(pattern_)
        , type(type_) {}
};

struct Argument : public Node {
    absl::optional<MetaItem*> attribute;
    Expression* value;

    explicit Argument(absl::optional<MetaItem*> attribute_,
                      Expression* value_)
        : attribute(attribute_), value(value_) {}
};

////////////////////////////////////////////////////////////////////////////////

struct Attribute : public Declaration {
    MetaItem* meta_item;

    explicit Attribute(MetaItem* meta_item_) : meta_item(meta_item_) {}
};

struct Struct : public Declaration {
    Visibility visibility;
    TypeIdentifierWithParameters name;
    Fields fields;

    explicit Struct(Visibility visibility_,
                    const TypeIdentifierWithParameters& name_,
                    const Fields& fields_)
        : visibility(visibility_)
        , name(name_)
        , fields(fields_) {}
};

struct Table : public Declaration {
    Visibility visibility;
    TypeIdentifierWithParameters name;
    Fields fields;
    using FieldTuple = std::vector<FieldIdentifier*>;
    absl::btree_set<std::pair<FieldTuple, FieldTuple>> fundeps;

    explicit Table(
        Visibility visibility_,
        const TypeIdentifierWithParameters& name_,
        const Fields& fields_,
        absl::Span<std::pair<FieldTuple, FieldTuple> const> fundeps_)
        : visibility(visibility_)
        , name(name_)
        , fields(fields_)
        , fundeps(fundeps_.begin(), fundeps_.end()) {}
};

struct Enum : public Declaration {
    Visibility visibility;
    TypeIdentifierWithParameters name;
    absl::btree_map<Identifier*, Fields> variants;

    explicit Enum(Visibility visibility_,
                  const TypeIdentifierWithParameters& name_,
                  absl::Span<std::pair<Identifier*, Fields> const> variants_)
        : visibility(visibility_)
        , name(name_)
        , variants(variants_.begin(), variants_.end()) {}
};

struct TypeAlias : public Declaration {
    Visibility visibility;
    TypeIdentifierWithParameters name;
    Type* type;

    explicit TypeAlias(Visibility visibility_,
                       const TypeIdentifierWithParameters& name_,
                       Type* type_)
        : visibility(visibility_), name(name_), type(type_) {}
};

struct Function : public Declaration {
    Visibility visibility;
    IdentifierWithParameters name;
    std::vector<Parameter> parameters;
    Type* return_type;
    absl::optional<Block*> block;

    explicit Function(Visibility visibility_,
                      const IdentifierWithParameters& name_,
                      absl::Span<Parameter const> parameters_,
                      Type* return_type_,
                      absl::optional<Block*> block_)
        : visibility(visibility_)
        , name(name_)
        , parameters(parameters_.begin(), parameters_.end())
        , return_type(return_type_)
        , block(block_) {}
};

struct Query : public Declaration {
    Visibility visibility;
    TypeIdentifier scope;
    IdentifierWithParameters name;
    bool self_parameter;
    std::vector<Parameter> parameters;
    Type* return_type;
    Block* block;

    explicit Query(Visibility visibility_,
                   TypeIdentifier scope_,
                   const IdentifierWithParameters& name_,
                   bool self_parameter_,
                   absl::Span<Parameter const> parameters_,
                   Type* return_type_,
                   Block* block_)
        : visibility(visibility_)
        , scope(scope_)
        , name(name_)
        , self_parameter(self_parameter_)
        , parameters(parameters_.begin(), parameters_.end())
        , return_type(return_type_)
        , block(block_) {}
};

struct Let : public Declaration {
    Mutability mutability;
    Pattern* pattern;
    absl::optional<Type*> type;
    Expression* value;

    explicit Let(Mutability mutability_,
                 Pattern* pattern_,
                 absl::optional<Type*> type_,
                 Expression* value_)
        : mutability(mutability_)
        , pattern(pattern_)
        , type(type_)
        , value(value_) {}
};

////////////////////////////////////////////////////////////////////////////////

enum class Unary {
    NegateInteger,
    NegateBoolean,
    Dereference,
    Reference
};

enum class Binary {
    AndBoolean,
    OrBoolean,
    AndBitwise,
    OrBitwise,
    XorBitwise,
    Equals,
    NotEquals,
    LessThan,
    LessEquals,
    GreaterThan,
    GreaterEquals,
    ShiftLeft,
    ShiftRight,
    Add,
    Subtract,
    Times,
    Divide,
    Modulo
};

////////////////////////////////////////////////////////////////////////////////

struct UnaryOperator : public Expression {
    Unary op;
    Expression* value;

    explicit UnaryOperator(Unary op_, Expression* value_)
        : op(op_), value(value_) {}
};

struct BinaryOperator : public Expression {
    Binary op;
    Expression* lhs;
    Expression* rhs;

    explicit BinaryOperator(Binary op_, Expression* lhs_, Expression* rhs_)
        : op(op_), lhs(lhs_), rhs(rhs_) {}
};

struct Assignment : public Expression {
    Expression* lhs;
    Expression* rhs;

    explicit Assignment(Expression* lhs_, Expression* rhs_)
        : lhs(lhs_), rhs(rhs_) {}
};

struct TypeCast : public Expression {
    Expression* value;
    Type* type;

    explicit TypeCast(Expression* value_, Type* type_)
        : value(value_), type(type_) {}
};

struct Range : public Expression {
    Expression* from;
    Expression* to;
    bool inclusive;

    explicit Range(Expression* from_, Expression* to_, bool inclusive_)
        : from(from_), to(to_), inclusive(inclusive_) {}
};

struct Instantiation : public Expression {
    IdentifierWithParameters name;

    explicit Instantiation(const IdentifierWithParameters& name_)
        : name(name_) {}
};

struct FunctionCall : public Expression {
    Expression* function;
    absl::btree_map<FieldIdentifier*, Argument> arguments;

    explicit FunctionCall(
        Expression* function_,
        absl::Span<std::pair<FieldIdentifier*, Argument> const> arguments_)
        : function(function_)
        , arguments(arguments_.begin(), arguments_.end()) {}
};

struct Return : public Expression {
    absl::optional<Expression*> value;

    explicit Return(absl::optional<Expression*> value_) : value(value_) {}
};

struct LabelBreak : public Expression {
    absl::optional<LoopLabel> label;

    explicit LabelBreak(absl::optional<LoopLabel> label_) : label(label_) {}
};

struct LoopBreak : public Expression {
    Expression* value;

    explicit LoopBreak(Expression* value_) : value(value_) {}
};

struct Continue : public Expression {
    absl::optional<LoopLabel> label;

    explicit Continue(absl::optional<LoopLabel> label_) : label(label_) {}
};

struct VarReference : public Expression {
    Identifier ident;

    explicit VarReference(Identifier ident_) : ident(ident_) {}
};

struct FieldAccess : public Expression {
    Expression* value;
    FieldIdentifier field;

    explicit FieldAccess(Expression* value_, FieldIdentifier field_)
        : value(value_), field(field_) {}
};

struct ArrayCreate : public Expression {
    std::vector<Expression*> array;
    Expression* repetitions;

    explicit ArrayCreate(absl::Span<Expression* const> array_,
                         Expression* repetitions_)
        : array(array_.begin(), array_.end()), repetitions(repetitions_) {}
};

struct TupleCreate : public Expression {
    std::vector<MetaItem*> attributes;
    std::vector<Expression*> values;

    explicit TupleCreate(absl::Span<MetaItem* const> attributes_,
                         absl::Span<Expression* const> values_)
        : attributes(attributes_.begin(), attributes_.end())
        , values(values_.begin(), values_.end()) {}
};

struct StructCreate : public Expression {
    TypeIdentifierWithParameters name;
    using ExprWithAttributes = std::pair<Expression*, std::vector<MetaItem*>>;
    absl::btree_map<FieldIdentifier*, ExprWithAttributes> fields;
    absl::optional<Expression*> base_field_initializer;

    explicit StructCreate(
        TypeIdentifierWithParameters name_,
        absl::Span<std::pair<FieldIdentifier*, ExprWithAttributes> const> fields_,
        absl::optional<Expression*> base_field_initializer_)
        : name(name_)
        , fields(fields_.begin(), fields_.end())
        , base_field_initializer(base_field_initializer_) {}
};

struct ArrayIndex : public Expression {
    Expression* array;
    Expression* index;

    explicit ArrayIndex(Expression* array_, Expression* index_)
        : array(array_), index(index_) {}
};

struct Block : public Expression {
    std::vector<Statement*> statements;
    absl::optional<Expression*> final_expression;

    explicit Block(absl::Span<Statement* const> statements_,
                   absl::optional<Expression*> final_expression_)
        : statements(statements_.begin(), statements_.end())
        , final_expression(final_expression_) {}
};

using ElseClause = absl::variant<Block*, If*, IfLet*>;

struct If : public Expression {
    Expression* condition;
    Block* consequence;
    absl::optional<ElseClause> alternative;

    explicit If(Expression* condition_,
                Block* consequence_,
                absl::optional<ElseClause> alternative_)
        : condition(condition_)
        , consequence(consequence_)
        , alternative(alternative_) {}
};

struct IfLet : public Expression {
    Pattern* pattern;
    Expression* value;
    Block* consequence;
    absl::optional<ElseClause> alternative;

    explicit IfLet(Pattern* pattern_,
                   Expression* value_,
                   Block* consequence_,
                   absl::optional<ElseClause> alternative_)
        : pattern(pattern_)
        , value(value_)
        , consequence(consequence_)
        , alternative(alternative_) {}
};

struct MatchPattern : public Node {
    Pattern* pattern;
    absl::optional<Expression*> condition;

    explicit MatchPattern(Pattern* pattern_,
                          absl::optional<Expression*> condition_)
        : pattern(pattern_), condition(condition_) {}
};

struct MatchArm {
    MatchPattern* pattern;
    Expression* value;

    explicit MatchArm(MatchPattern* pattern_, Expression* value_)
        : pattern(pattern_), value(value_) {}
};

struct Match : public Expression {
    Expression* scrutinee;
    std::vector<MatchArm> arms;

    explicit Match(Expression* scrutinee_, absl::Span<MatchArm const> arms_)
        : scrutinee(scrutinee_), arms(arms_.begin(), arms_.end()) {}
};

struct While : public Expression {
    absl::optional<LoopLabel> label;
    Expression* condition;
    Block* body;

    explicit While(absl::optional<LoopLabel> label_,
                   Expression* condition_,
                   Block* body_)
        : label(label_), condition(condition_), body(body_) {}
};

struct WhileLet : public Expression {
    absl::optional<LoopLabel> label;
    Pattern* pattern;
    Expression* value;
    Block* body;

    explicit WhileLet(absl::optional<LoopLabel> label_,
                      Pattern* pattern_,
                      Expression* value_,
                      Block* body_)
        : label(label_), pattern(pattern_), value(value_), body(body_) {}
};

struct Loop : public Expression {
    absl::optional<LoopLabel> label;
    Block* body;

    explicit Loop(absl::optional<LoopLabel> label_, Block* body_)
        : label(label_), body(body_) {}
};

struct For : public Expression {
    absl::optional<LoopLabel> label;
    Pattern* pattern;
    Expression* value;
    Block* body;

    explicit For(absl::optional<LoopLabel> label_,
                 Pattern* pattern_,
                 Expression* value_,
                 Block* body_)
        : label(label_), pattern(pattern_), value(value_), body(body_) {}
};

////////////////////////////////////////////////////////////////////////////////

struct SourceFile : public Node {
    std::vector<Declaration*> declarations;

    explicit SourceFile(absl::Span<Declaration* const> declarations_)
        : declarations(declarations_.begin(), declarations_.end()) {}
};

}  // namespace syntax

absl::StatusOr<TSTree*> RunParser(const std::string& code);

std::string PrintNode(const char* file, TSNode node);
std::string PrintTree(const char* file, const TSTree* tree);

}  // namespace rdss

#endif  // RDSS_PARSER_H_
