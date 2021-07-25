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

#ifndef RDSS_LSP_H_
#define RDSS_LSP_H_

#include "macros.hpp"
#include "logging/logging.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include <absl/container/btree_map.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/types/optional.h>
#include <absl/types/variant.h>

#include <json/json.h>

namespace rdss {

namespace lsp {

using JSON = Json::Value;

JSON ToJSON(const JSON& input) {
    return input;
}

JSON ToJSON(const absl::monostate& input) {
    return JSON();
}

template<typename A, typename B>
JSON ToJSON(const std::pair<A, B>& input) {
    JSON result(Json::arrayValue);
    result.append(ToJSON(input.first));
    result.append(ToJSON(input.second));
    return result;
}

template<typename A, typename B>
JSON ToJSON(const absl::variant<A, B>& input) {
    if (absl::holds_alternative<A>(input)) {
        return ToJSON(absl::get<A>(input));
    } else if (absl::holds_alternative<B>(input)) {
        return ToJSON(absl::get<B>(input));
    }
    RDSS_CHECK(false);
}

template<typename A, typename B, typename C>
JSON ToJSON(const absl::variant<A, B, C>& input) {
    if (absl::holds_alternative<A>(input)) {
        return ToJSON(absl::get<A>(input));
    } else if (absl::holds_alternative<B>(input)) {
        return ToJSON(absl::get<B>(input));
    } else if (absl::holds_alternative<C>(input)) {
        return ToJSON(absl::get<C>(input));
    }
    RDSS_CHECK(false);
}

template<typename A, typename B, typename C, typename D>
JSON ToJSON(const absl::variant<A, B, C, D>& input) {
    if (absl::holds_alternative<A>(input)) {
        return ToJSON(absl::get<A>(input));
    } else if (absl::holds_alternative<B>(input)) {
        return ToJSON(absl::get<B>(input));
    } else if (absl::holds_alternative<C>(input)) {
        return ToJSON(absl::get<C>(input));
    } else if (absl::holds_alternative<D>(input)) {
        return ToJSON(absl::get<D>(input));
    }
    RDSS_CHECK(false);
}

template<typename T>
JSON ToJSON(const absl::optional<T>& input) {
    if (input.has_value()) {
        return ToJSON(input.value());
    } else {
        return JSON();
    }
}

template<typename T>
JSON ToJSON(const std::vector<T>& input) {
    JSON result(Json::arrayValue);
    for (const T& elem : input) {
        result.append(ToJSON(elem));
    }
    return result;
}

template<typename T>
std::string ToString(T input) {
    JSON json = ToJSON(input);
    RDSS_CHECK(json.isString());
    return json.asString();
}

template<typename K, typename V>
JSON ToJSON(const absl::btree_map<K, V>& input) {
    JSON result(Json::objectValue);
    for (const auto& [key, value] : input) {
        result[ToString(key)] = ToJSON(value);
    }
    return result;
}

template<typename T>
void Set(JSON* obj, const char* key, const T& val) {
    (*obj)[key] = ToJSON(val);
}

template<typename T>
void OptionallySet(JSON* obj, const char* key, const absl::optional<T>& val) {
    if (val.has_value()) {
        (*obj)[key] = ToJSON(val.value());
    }
}

void Merge(JSON* obj, const JSON& value) {
    for (const auto& key : value.getMemberNames()) {
        (*obj)[key] = value[key];
    }
}

enum class ErrorCodes : int32_t {
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602,
    InternalError = -32603,
    JsonRpcServerNotInitialized = -32002,
    JsonRpcUnknownErrorCode = -32001,
    LspContentModified = -32801,
    LspRequestCancelled = -32800
};

JSON ToJSON(const ErrorCodes& input) {
    return static_cast<int32_t>(input);
}

using RequestId = absl::variant<int32_t, std::string>;

struct ResponseError {
    int32_t code;
    std::string message;
    absl::optional<JSON> data;
};

JSON ToJSON(const ResponseError& input) {
    JSON result(Json::objectValue);
    Set(&result, "code", input.code);
    Set(&result, "message", input.message);
    OptionallySet(&result, "data", input.data);
    return result;
}

struct ResponseMessage {
    absl::optional<RequestId> id;
    absl::optional<JSON> result;
    absl::optional<ResponseError> error;
};

JSON ToJSON(const ResponseMessage& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "id", input.id);
    OptionallySet(&result, "result", input.result);
    OptionallySet(&result, "error", input.error);
    return result;
}

template<typename T>
struct Response {
    // the request ID should get injected by other stuff
    absl::optional<T> result;
    absl::optional<ResponseError> error;
};

using DocumentUri = std::string;
using URI = std::string;

struct RegularExpressionsClientCapabilities {
    std::string engine;
    absl::optional<std::string> version;
};

JSON ToJSON(const RegularExpressionsClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "engine", input.engine);
    OptionallySet(&result, "version", input.version);
    return result;
}

struct Position {
    uint32_t line;
    uint32_t character;
};

JSON ToJSON(const Position& input) {
    JSON result(Json::objectValue);
    Set(&result, "line", input.line);
    Set(&result, "character", input.character);
    return result;
}

struct Range {
    Position start;
    Position end;
};

JSON ToJSON(const Range& input) {
    JSON result(Json::objectValue);
    Set(&result, "start", input.start);
    Set(&result, "end", input.end);
    return result;
}

struct Location {
    DocumentUri uri;
    Range range;
};

JSON ToJSON(const Location& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "range", input.range);
    return result;
}

struct LocationLink {
    absl::optional<Range> origin_selection_range;
    DocumentUri target_uri;
    Range target_range;
    Range target_selection_range;
};

JSON ToJSON(const LocationLink& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "originSelectionRange", input.origin_selection_range);
    Set(&result, "targetUri", input.target_uri);
    Set(&result, "targetRange", input.target_range);
    Set(&result, "targetSelectionRange", input.target_selection_range);
    return result;
}

enum class DiagnosticSeverity : int32_t {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4
};

JSON ToJSON(const DiagnosticSeverity& input) {
    return static_cast<int32_t>(input);
}

struct DiagnosticCode {
    absl::variant<int32_t, std::string> code;
};

JSON ToJSON(const DiagnosticCode& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.code));
    return result;
}

struct CodeDescription {
    URI href;
};

JSON ToJSON(const CodeDescription& input) {
    JSON result(Json::objectValue);
    Set(&result, "href", input.href);
    return result;
}

enum class DiagnosticTag : int32_t {
    Unnecessary = 1,
    Deprecated = 2
};

JSON ToJSON(const DiagnosticTag& input) {
    return static_cast<int32_t>(input);
}

struct DiagnosticRelatedInformation {
    Location location;
    std::string message;
};

JSON ToJSON(const DiagnosticRelatedInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "location", input.location);
    Set(&result, "message", input.message);
    return result;
}

struct Diagnostic {
    Range range;
    absl::optional<DiagnosticSeverity> severity;
    absl::optional<DiagnosticCode> code;
    absl::optional<CodeDescription> code_description;
    absl::optional<std::string> source;
    std::string message;
    std::vector<DiagnosticTag> tags;
    std::vector<DiagnosticRelatedInformation> related_information;
    JSON data;
};

JSON ToJSON(const Diagnostic& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "severity", input.severity);
    OptionallySet(&result, "code", input.code);
    OptionallySet(&result, "codeDescription", input.code_description);
    OptionallySet(&result, "source", input.source);
    Set(&result, "message", input.message);
    Set(&result, "tags", input.tags);
    Set(&result, "relatedInformation", input.related_information);
    Set(&result, "data", input.data);
    return result;
}

struct Command {
    std::string title;
    std::string command;
    absl::optional<std::vector<JSON>> arguments;
};

JSON ToJSON(const Command& input) {
    JSON result(Json::objectValue);
    Set(&result, "title", input.title);
    Set(&result, "command", input.command);
    OptionallySet(&result, "arguments", input.arguments);
    return result;
}

struct TextEdit {
    Range range;
    std::string new_text;
};

JSON ToJSON(const TextEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    Set(&result, "newText", input.new_text);
    return result;
}

struct ChangeAnnotation {
    std::string label;
    absl::optional<bool> needs_confirmation;
    absl::optional<std::string> description;
};

JSON ToJSON(const ChangeAnnotation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "needsConfirmation", input.needs_confirmation);
    OptionallySet(&result, "description", input.description);
    return result;
}

using ChangeAnnotationIdentifier = std::string;

struct TextDocumentIdentifier {
    DocumentUri uri;
};

JSON ToJSON(const TextDocumentIdentifier& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

struct TextDocumentItem {
    DocumentUri uri;
    std::string language_id;
    int32_t version;
    std::string text;
};

JSON ToJSON(const TextDocumentItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "languageId", input.language_id);
    Set(&result, "version", input.version);
    Set(&result, "text", input.text);
    return result;
}

struct VersionedTextDocumentIdentifier {
    TextDocumentIdentifier underlying;
    int32_t version;
};

JSON ToJSON(const VersionedTextDocumentIdentifier& input) {
    JSON result = ToJSON(input.underlying);
    Set(&result, "version", input.version);
    return result;
}

struct OptionalVersionedTextDocumentIdentifier {
    TextDocumentIdentifier underlying;
    absl::optional<int32_t> version;
};

JSON ToJSON(const OptionalVersionedTextDocumentIdentifier& input) {
    JSON result = ToJSON(input.underlying);
    OptionallySet(&result, "version", input.version);
    return result;
}

struct AnnotatedTextEdit {
    TextEdit underlying;
    ChangeAnnotationIdentifier annotation_id;
};

JSON ToJSON(const AnnotatedTextEdit& input) {
    JSON result = ToJSON(input.underlying);
    Set(&result, "annotationId", input.annotation_id);
    return result;
}

struct TextDocumentEdit {
    OptionalVersionedTextDocumentIdentifier text_document;
    std::vector<absl::variant<TextEdit, AnnotatedTextEdit>> edits;
};

JSON ToJSON(const TextDocumentEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "edits", input.edits);
    return result;
}

struct CreateFileOptions {
    absl::optional<bool> overwrite;
    absl::optional<bool> ignore_if_exists;
};

JSON ToJSON(const CreateFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "overwrite", input.overwrite);
    OptionallySet(&result, "ignoreIfExists", input.ignore_if_exists);
    return result;
}

struct CreateFile {
    // kind: 'create'
    DocumentUri uri;
    absl::optional<CreateFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

JSON ToJSON(const CreateFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "create";
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

struct RenameFileOptions {
    absl::optional<bool> overwrite;
    absl::optional<bool> ignore_if_exists;
};

JSON ToJSON(const RenameFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "overwrite", input.overwrite);
    OptionallySet(&result, "ignoreIfExists", input.ignore_if_exists);
    return result;
}

struct RenameFile {
    // kind: 'rename'
    DocumentUri old_uri;
    DocumentUri new_uri;
    absl::optional<RenameFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

JSON ToJSON(const RenameFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "rename";
    Set(&result, "oldUri", input.old_uri);
    Set(&result, "newUri", input.new_uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

struct DeleteFileOptions {
    absl::optional<bool> recursive;
    absl::optional<bool> ignore_if_not_exists;
};

JSON ToJSON(const DeleteFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "recursive", input.recursive);
    OptionallySet(&result, "ignoreIfNotExists", input.ignore_if_not_exists);
    return result;
}

struct DeleteFile {
    // kind: 'delete'
    DocumentUri uri;
    absl::optional<DeleteFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

JSON ToJSON(const DeleteFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "delete";
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

struct DocumentChangeOperation {
    absl::variant<TextDocumentEdit,
                  CreateFile,
                  RenameFile,
                  DeleteFile> operation;
};

JSON ToJSON(const DocumentChangeOperation& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.operation));
    return result;
}

struct DocumentChanges {
    absl::variant<std::vector<TextDocumentEdit>,
                  std::vector<DocumentChangeOperation>> changes;
};

JSON ToJSON(const DocumentChanges& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.changes));
    return result;
}

struct WorkspaceEdit {
    absl::optional<absl::btree_map<DocumentUri, std::vector<TextEdit>>> changes;
    absl::optional<DocumentChanges> document_changes;
    absl::optional<absl::btree_map<ChangeAnnotationIdentifier,
                                   ChangeAnnotation>> change_annotations;
};

JSON ToJSON(const WorkspaceEdit& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "changes", input.changes);
    OptionallySet(&result, "documentChanges", input.document_changes);
    OptionallySet(&result, "changeAnnotations", input.change_annotations);
    return result;
}

enum class ResourceOperationKind {
    Create, Rename, Delete
};

JSON ToJSON(const ResourceOperationKind& input) {
    switch (input) {
    case ResourceOperationKind::Create: return "create";
    case ResourceOperationKind::Rename: return "rename";
    case ResourceOperationKind::Delete: return "delete";
    }
    RDSS_CHECK(false);
    return JSON();
}

enum class FailureHandlingKind {
    Abort, Transactional, Undo, TextOnlyTransactional
};

JSON ToJSON(const FailureHandlingKind& input) {
    switch (input) {
    case FailureHandlingKind::Abort:                 return "abort";
    case FailureHandlingKind::Transactional:         return "transactional";
    case FailureHandlingKind::Undo:                  return "undo";
    case FailureHandlingKind::TextOnlyTransactional: return "textOnlyTransactional";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct ChangeAnnotationSupport {
    absl::optional<bool> groups_on_label;
};

JSON ToJSON(const ChangeAnnotationSupport& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "groupsOnLabel", input.groups_on_label);
    return result;
}

struct WorkspaceEditClientCapabilities {
    absl::optional<bool> document_changes;
    absl::optional<std::vector<ResourceOperationKind>> resource_operations;
    absl::optional<FailureHandlingKind> failure_handling;
    absl::optional<bool> normalizes_line_endings;
    absl::optional<ChangeAnnotationSupport> change_annotation_support;
};

JSON ToJSON(const WorkspaceEditClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "documentChanges", input.document_changes);
    OptionallySet(&result, "resourceOperations", input.resource_operations);
    OptionallySet(&result, "failureHandling", input.failure_handling);
    OptionallySet(&result, "normalizesLineEndings", input.normalizes_line_endings);
    OptionallySet(&result, "changeAnnotationSupport", input.change_annotation_support);
    return result;
}

struct TextDocumentPositionParams {
    TextDocumentIdentifier text_document;
    Position position;
};

JSON ToJSON(const TextDocumentPositionParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "position", input.position);
    return result;
}

struct DocumentFilter {
    absl::optional<std::string> language;
    absl::optional<std::string> scheme;
    absl::optional<std::string> pattern;
};

JSON ToJSON(const DocumentFilter& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "language", input.language);
    OptionallySet(&result, "scheme", input.scheme);
    OptionallySet(&result, "pattern", input.pattern);
    return result;
}

using DocumentSelector = std::vector<DocumentFilter>;

struct StaticRegistrationOptions {
    absl::optional<std::string> id;
};

JSON ToJSON(const StaticRegistrationOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "id", input.id);
    return result;
}

struct TextDocumentRegistrationOptions {
    // nullopt == null here
    absl::optional<DocumentSelector> document_selector;
};

JSON ToJSON(const TextDocumentRegistrationOptions& input) {
    JSON result(Json::objectValue);
    if (input.document_selector.has_value()) {
        result["documentSelector"] = ToJSON(input.document_selector.value());
    } else {
        result["documentSelector"] = JSON();
    }
    return result;
}

enum class MarkupKind {
    PlainText, Markdown
};

JSON ToJSON(const MarkupKind& input) {
    switch (input) {
    case MarkupKind::PlainText: return "plaintext";
    case MarkupKind::Markdown:  return "markdown";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct MarkupContent {
    MarkupKind kind;
    std::string value;
};

JSON ToJSON(const MarkupContent& input) {
    JSON result(Json::objectValue);
    Set(&result, "kind", input.kind);
    Set(&result, "value", input.value);
    return result;
}

struct MarkdownClientCapabilities {
    std::string parser;
    absl::optional<std::string> version;
};

JSON ToJSON(const MarkdownClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "parser", input.parser);
    OptionallySet(&result, "version", input.version);
    return result;
}

using ProgressToken = absl::variant<int32_t, std::string>;

struct WorkDoneProgressBegin {
    // kind: 'begin'
    std::string title;
    absl::optional<bool> cancellable;
    absl::optional<std::string> message;
    absl::optional<uint8_t> percentage;
};

JSON ToJSON(const WorkDoneProgressBegin& input) {
    JSON result(Json::objectValue);
    result["kind"] = "begin";
    Set(&result, "title", input.title);
    OptionallySet(&result, "cancellable", input.cancellable);
    OptionallySet(&result, "message", input.message);
    OptionallySet(&result, "percentage", input.percentage);
    return result;
}

struct WorkDoneProgressReport {
    // kind: 'report'
    absl::optional<bool> cancellable;
    absl::optional<std::string> message;
    absl::optional<uint8_t> percentage;
};

JSON ToJSON(const WorkDoneProgressReport& input) {
    JSON result(Json::objectValue);
    result["kind"] = "report";
    OptionallySet(&result, "cancellable", input.cancellable);
    OptionallySet(&result, "message", input.message);
    OptionallySet(&result, "percentage", input.percentage);
    return result;
}

struct WorkDoneProgressEnd {
    // kind: 'end'
    absl::optional<std::string> message;
};

JSON ToJSON(const WorkDoneProgressEnd& input) {
    JSON result(Json::objectValue);
    result["kind"] = "end";
    OptionallySet(&result, "message", input.message);
    return result;
}

struct WorkDoneProgressParams {
    absl::optional<ProgressToken> work_done_token;
};

JSON ToJSON(const WorkDoneProgressParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneToken", input.work_done_token);
    return result;
}

struct PartialResultParams {
    absl::optional<ProgressToken> partial_result_token;
};

JSON ToJSON(const PartialResultParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "partialResultToken", input.partial_result_token);
    return result;
}

enum class TraceValue {
    Off, Messages, Verbose
};

JSON ToJSON(const TraceValue& input) {
    switch (input) {
    case TraceValue::Off:      return "off";
    case TraceValue::Messages: return "messages";
    case TraceValue::Verbose:  return "verbose";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct ClientInfo {
    std::string name;
    absl::optional<std::string> version;
};

JSON ToJSON(const ClientInfo& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    OptionallySet(&result, "version", input.version);
    return result;
}

enum class SymbolKind : int32_t {
    File = 1,
    Module = 2,
    Namespace = 3,
    Package = 4,
    Class = 5,
    Method = 6,
    Property = 7,
    Field = 8,
    Constructor = 9,
    Enum = 10,
    Interface = 11,
    Function = 12,
    Variable = 13,
    Constant = 14,
    String = 15,
    Number = 16,
    Boolean = 17,
    Array = 18,
    Object = 19,
    Key = 20,
    Null = 21,
    EnumMember = 22,
    Struct = 23,
    Event = 24,
    Operator = 25,
    TypeParameter = 26
};

JSON ToJSON(const SymbolKind& input) {
    return static_cast<int32_t>(input);
}

struct SymbolKindClientCapabilities {
    absl::optional<std::vector<SymbolKind>> value_set;
};

JSON ToJSON(const SymbolKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

enum class SymbolTag : int32_t {
    Deprecated = 1
};

JSON ToJSON(const SymbolTag& input) {
    return static_cast<int32_t>(input);
}

struct SymbolTagSupportClientCapabilities {
    absl::optional<std::vector<SymbolTag>> value_set;
};

JSON ToJSON(const SymbolTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

struct TextDocumentSyncClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> will_save;
    absl::optional<bool> will_save_wait_until;
    absl::optional<bool> did_save;
};

JSON ToJSON(const TextDocumentSyncClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "willSave", input.will_save);
    OptionallySet(&result, "willSaveWaitUntil", input.will_save_wait_until);
    OptionallySet(&result, "didSave", input.did_save);
    return result;
}

struct DiagnosticTagSupportClientCapabilities {
    std::vector<DiagnosticTag> value_set;
};

JSON ToJSON(const DiagnosticTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

struct PublishDiagnosticsClientCapabilities {
    absl::optional<bool> related_information;
    absl::optional<DiagnosticTagSupportClientCapabilities> tag_support;
    absl::optional<bool> version_support;
    absl::optional<bool> code_description_support;
    absl::optional<bool> data_support;
};

JSON ToJSON(const PublishDiagnosticsClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "relatedInformation", input.related_information);
    OptionallySet(&result, "tagSupport", input.tag_support);
    OptionallySet(&result, "versionSupport", input.version_support);
    OptionallySet(&result, "codeDescriptionSupport", input.code_description_support);
    OptionallySet(&result, "dataSupport", input.data_support);
    return result;
}

enum class CompletionItemTag : int32_t {
    Deprecated = 1
};

JSON ToJSON(const CompletionItemTag& input) {
    return static_cast<int32_t>(input);
}

struct CompletionItemTagSupportClientCapabilities {
    std::vector<CompletionItemTag> value_set;
};

JSON ToJSON(const CompletionItemTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

struct CompletionItemResolveSupportClientCapabilities {
    std::vector<std::string> properties;
};

JSON ToJSON(const CompletionItemResolveSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "properties", input.properties);
    return result;
}

enum class InsertTextMode : int32_t {
    AsIs = 1, AdjustIndentation = 2
};

JSON ToJSON(const InsertTextMode& input) {
    return static_cast<int32_t>(input);
}

struct CompletionItemInsertTextModeSupportClientCapabilities {
    std::vector<InsertTextMode> value_set;
};

JSON ToJSON(const CompletionItemInsertTextModeSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

struct CompletionItemClientCapabilities {
    absl::optional<bool> snippet_support;
    absl::optional<bool> commit_characters_support;
    absl::optional<std::vector<MarkupKind>> documentation_format;
    absl::optional<bool> deprecated_support;
    absl::optional<bool> preselect_support;
    absl::optional<CompletionItemTagSupportClientCapabilities> tag_support;
    absl::optional<bool> insert_replace_support;
    absl::optional<CompletionItemResolveSupportClientCapabilities> resolve_support;
    absl::optional<CompletionItemInsertTextModeSupportClientCapabilities> insert_text_mode_support;
    absl::optional<bool> label_details_support;
};

JSON ToJSON(const CompletionItemClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "snippetSupport", input.snippet_support);
    OptionallySet(&result, "commitCharactersSupport", input.commit_characters_support);
    OptionallySet(&result, "documentationFormat", input.documentation_format);
    OptionallySet(&result, "deprecatedSupport", input.deprecated_support);
    OptionallySet(&result, "preselectSupport", input.preselect_support);
    OptionallySet(&result, "tagSupport", input.tag_support);
    OptionallySet(&result, "insertReplaceSupport", input.insert_replace_support);
    OptionallySet(&result, "resolveSupport", input.resolve_support);
    OptionallySet(&result, "insertTextModeSupport", input.insert_text_mode_support);
    OptionallySet(&result, "labelDetailsSupport", input.label_details_support);
    return result;
}

enum class CompletionItemKind : int32_t {
    Text = 1,
    Method = 2,
    Function = 3,
    Constructor = 4,
    Field = 5,
    Variable = 6,
    Class = 7,
    Interface = 8,
    Module = 9,
    Property = 10,
    Unit = 11,
    Value = 12,
    Enum = 13,
    Keyword = 14,
    Snippet = 15,
    Color = 16,
    File = 17,
    Reference = 18,
    Folder = 19,
    EnumMember = 20,
    Constant = 21,
    Struct = 22,
    Event = 23,
    Operator = 24,
    TypeParameter = 25
};

JSON ToJSON(const CompletionItemKind& input) {
    return static_cast<int32_t>(input);
}

struct CompletionItemKindClientCapabilities {
    absl::optional<std::vector<CompletionItemKind>> value_set;
};

JSON ToJSON(const CompletionItemKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

struct CompletionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<CompletionItemClientCapabilities> completion_item;
    absl::optional<CompletionItemKindClientCapabilities> completion_item_kind;
    absl::optional<bool> context_support;
    absl::optional<InsertTextMode> insert_text_mode;
};

JSON ToJSON(const CompletionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "completionItem", input.completion_item);
    OptionallySet(&result, "completionItemKind", input.completion_item_kind);
    OptionallySet(&result, "contextSupport", input.context_support);
    OptionallySet(&result, "insertTextMode", input.insert_text_mode);
    return result;
}

struct HoverClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<std::vector<MarkupKind>> content_format;
};

JSON ToJSON(const HoverClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "contentFormat", input.content_format);
    return result;
}

struct ParameterInformationClientCapabilities {
    absl::optional<bool> label_offset_support;
};

JSON ToJSON(const ParameterInformationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "labelOffsetSupport", input.label_offset_support);
    return result;
}

struct SignatureInformationClientCapabilities {
    absl::optional<std::vector<MarkupKind>> documentation_format;
    absl::optional<ParameterInformationClientCapabilities> parameter_information;
    absl::optional<bool> active_parameter_support;
};

JSON ToJSON(const SignatureInformationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "documentationFormat", input.documentation_format);
    OptionallySet(&result, "parameterInformation", input.parameter_information);
    OptionallySet(&result, "activeParameterSupport", input.active_parameter_support);
    return result;
}

struct SignatureHelpClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SignatureInformationClientCapabilities> signature_information;
    absl::optional<bool> context_support;
};

JSON ToJSON(const SignatureHelpClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "signatureInformation", input.signature_information);
    OptionallySet(&result, "contextSupport", input.context_support);
    return result;
}

struct DeclarationClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

JSON ToJSON(const DeclarationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

struct DefinitionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

JSON ToJSON(const DefinitionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

struct TypeDefinitionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

JSON ToJSON(const TypeDefinitionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

struct ImplementationClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

JSON ToJSON(const ImplementationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

struct ReferenceClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const ReferenceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentHighlightClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DocumentHighlightClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentSymbolClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SymbolKindClientCapabilities> symbol_kind;
    absl::optional<bool> hierarchical_document_symbol_support;
    absl::optional<SymbolTagSupportClientCapabilities> tag_support;
    absl::optional<bool> label_support;
};

JSON ToJSON(const DocumentSymbolClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "symbolKind", input.symbol_kind);
    OptionallySet(&result, "hierarchicalDocumentSymbolSupport", input.hierarchical_document_symbol_support);
    OptionallySet(&result, "tagSupport", input.tag_support);
    OptionallySet(&result, "labelSupport", input.label_support);
    return result;
}

enum class CodeActionKind {
    Empty,
    QuickFix,
    Refactor,
    RefactorExtract,
    RefactorInline,
    RefactorRewrite,
    Source,
    SourceOrganizeImports,
    SourceFixAll
};

JSON ToJSON(const CodeActionKind& input) {
    switch (input) {
    case CodeActionKind::Empty:                 return "";
    case CodeActionKind::QuickFix:              return "quickfix";
    case CodeActionKind::Refactor:              return "refactor";
    case CodeActionKind::RefactorExtract:       return "refactor.extract";
    case CodeActionKind::RefactorInline:        return "refactor.inline";
    case CodeActionKind::RefactorRewrite:       return "refactor.rewrite";
    case CodeActionKind::Source:                return "source";
    case CodeActionKind::SourceOrganizeImports: return "source.organizeImports";
    case CodeActionKind::SourceFixAll:          return "source.fixAll";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct CodeActionKindClientCapabilities {
    std::vector<CodeActionKind> value_set;
};

JSON ToJSON(const CodeActionKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

struct CodeActionLiteralClientCapabilities {
    CodeActionKindClientCapabilities code_action_kind;
};

JSON ToJSON(const CodeActionLiteralClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "codeActionKind", input.code_action_kind);
    return result;
}

struct CodeActionResolveClientCapabilities {
    std::vector<std::string> properties;
};

JSON ToJSON(const CodeActionResolveClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "properties", input.properties);
    return result;
}

struct CodeActionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<CodeActionLiteralClientCapabilities> code_action_literal_support;
    absl::optional<bool> is_preferred_support;
    absl::optional<bool> disabled_support;
    absl::optional<bool> data_support;
    absl::optional<CodeActionResolveClientCapabilities> resolve_support;
    absl::optional<bool> honors_change_annotations;
};

JSON ToJSON(const CodeActionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "codeActionLiteralSupport", input.code_action_literal_support);
    OptionallySet(&result, "isPreferredSupport", input.is_preferred_support);
    OptionallySet(&result, "disabledSupport", input.disabled_support);
    OptionallySet(&result, "dataSupport", input.data_support);
    OptionallySet(&result, "resolveSupport", input.resolve_support);
    OptionallySet(&result, "honorsChangeAnnotations", input.honors_change_annotations);
    return result;
}

struct CodeLensClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const CodeLensClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentLinkClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> tooltip_support;
};

JSON ToJSON(const DocumentLinkClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "tooltipSupport", input.tooltip_support);
    return result;
}

struct DocumentColorClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DocumentColorClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DocumentFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentRangeFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DocumentRangeFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DocumentOnTypeFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DocumentOnTypeFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

enum class PrepareSupportDefaultBehavior : int32_t {
    Identifier = 1
};

JSON ToJSON(const PrepareSupportDefaultBehavior& input) {
    return static_cast<int32_t>(input);
}

struct RenameClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> prepare_support;
    absl::optional<PrepareSupportDefaultBehavior> prepare_support_default_behavior;
    absl::optional<bool> honors_change_annotations;
};

JSON ToJSON(const RenameClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "prepareSupport", input.prepare_support);
    OptionallySet(&result, "prepareSupportDefaultBehavior", input.prepare_support_default_behavior);
    OptionallySet(&result, "honorsChangeAnnotations", input.honors_change_annotations);
    return result;
}

struct FoldingRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<uint32_t> range_limit;
    absl::optional<bool> line_folding_only;
};

JSON ToJSON(const FoldingRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "rangeLimit", input.range_limit);
    OptionallySet(&result, "lineFoldingOnly", input.line_folding_only);
    return result;
}

struct SelectionRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const SelectionRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct CallHierarchyClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const CallHierarchyClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct SemanticTokensRangeClientCapabilities {
};

JSON ToJSON(const SemanticTokensRangeClientCapabilities& input) {
    return JSON(Json::objectValue);
}

struct SemanticTokensFullClientCapabilities {
    absl::optional<bool> delta;
};

JSON ToJSON(const SemanticTokensFullClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "delta", input.delta);
    return result;
}

enum class TokenFormat {
    Relative
};

JSON ToJSON(const TokenFormat& input) {
    switch (input) {
    case TokenFormat::Relative: return "relative";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct SemanticTokensClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<absl::variant<bool, SemanticTokensRangeClientCapabilities>> requests_range; // encoded weirdly
    absl::optional<absl::variant<bool, SemanticTokensFullClientCapabilities>> requests_full; // encoded weirdly
    std::vector<std::string> token_types;
    std::vector<std::string> token_modifiers;
    std::vector<TokenFormat> formats;
    absl::optional<bool> overlapping_token_support;
    absl::optional<bool> multiline_token_support;
};

JSON ToJSON(const SemanticTokensClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    JSON requests(Json::objectValue);
    OptionallySet(&requests, "range", input.requests_range);
    OptionallySet(&requests, "full", input.requests_full);
    result["requests"] = requests;
    Set(&result, "tokenTypes", input.token_types);
    Set(&result, "tokenModifiers", input.token_modifiers);
    Set(&result, "formats", input.formats);
    OptionallySet(&result, "overlappingTokenSupport", input.overlapping_token_support);
    OptionallySet(&result, "multilineTokenSupport", input.multiline_token_support);
    return result;
}

struct LinkedEditingRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const LinkedEditingRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct MonikerClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const MonikerClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct TextDocumentClientCapabilities {
    absl::optional<TextDocumentSyncClientCapabilities> synchronization;
    absl::optional<CompletionClientCapabilities> completion;
    absl::optional<HoverClientCapabilities> hover;
    absl::optional<SignatureHelpClientCapabilities> signature_help;
    absl::optional<DeclarationClientCapabilities> declaration;
    absl::optional<DefinitionClientCapabilities> definition;
    absl::optional<TypeDefinitionClientCapabilities> type_definition;
    absl::optional<ImplementationClientCapabilities> implementation;
    absl::optional<ReferenceClientCapabilities> references;
    absl::optional<DocumentHighlightClientCapabilities> document_highlight;
    absl::optional<DocumentSymbolClientCapabilities> document_symbol;
    absl::optional<CodeActionClientCapabilities> code_action;
    absl::optional<CodeLensClientCapabilities> code_lens;
    absl::optional<DocumentLinkClientCapabilities> document_link;
    absl::optional<DocumentColorClientCapabilities> color_provider;
    absl::optional<DocumentFormattingClientCapabilities> formatting;
    absl::optional<DocumentRangeFormattingClientCapabilities> range_formatting;
    absl::optional<DocumentOnTypeFormattingClientCapabilities> on_type_formatting;
    absl::optional<RenameClientCapabilities> rename;
    absl::optional<PublishDiagnosticsClientCapabilities> publish_diagnostics;
    absl::optional<FoldingRangeClientCapabilities> folding_range;
    absl::optional<SelectionRangeClientCapabilities> selection_range;
    absl::optional<LinkedEditingRangeClientCapabilities> linked_editing_range;
    absl::optional<CallHierarchyClientCapabilities> call_hierarchy;
    absl::optional<SemanticTokensClientCapabilities> semantic_tokens;
    absl::optional<MonikerClientCapabilities> moniker;
};

JSON ToJSON(const TextDocumentClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "synchronization", input.synchronization);
    OptionallySet(&result, "completion", input.completion);
    OptionallySet(&result, "hover", input.hover);
    OptionallySet(&result, "signatureHelp", input.signature_help);
    OptionallySet(&result, "declaration", input.declaration);
    OptionallySet(&result, "definition", input.definition);
    OptionallySet(&result, "typeDefinition", input.type_definition);
    OptionallySet(&result, "implementation", input.implementation);
    OptionallySet(&result, "references", input.references);
    OptionallySet(&result, "documentHighlight", input.document_highlight);
    OptionallySet(&result, "documentSymbol", input.document_symbol);
    OptionallySet(&result, "codeAction", input.code_action);
    OptionallySet(&result, "codeLens", input.code_lens);
    OptionallySet(&result, "documentLink", input.document_link);
    OptionallySet(&result, "colorProvider", input.color_provider);
    OptionallySet(&result, "formatting", input.formatting);
    OptionallySet(&result, "rangeFormatting", input.range_formatting);
    OptionallySet(&result, "onTypeFormatting", input.on_type_formatting);
    OptionallySet(&result, "rename", input.rename);
    OptionallySet(&result, "publishDiagnostics", input.publish_diagnostics);
    OptionallySet(&result, "foldingRange", input.folding_range);
    OptionallySet(&result, "selectionRange", input.selection_range);
    OptionallySet(&result, "linkedEditingRange", input.linked_editing_range);
    OptionallySet(&result, "callHierarchy", input.call_hierarchy);
    OptionallySet(&result, "semanticTokens", input.semantic_tokens);
    OptionallySet(&result, "moniker", input.moniker);
    return result;
}

struct FileOperationsWorkspaceClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> did_create;
    absl::optional<bool> will_create;
    absl::optional<bool> did_rename;
    absl::optional<bool> will_rename;
    absl::optional<bool> did_delete;
    absl::optional<bool> will_delete;
};

JSON ToJSON(const FileOperationsWorkspaceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "didCreate", input.did_create);
    OptionallySet(&result, "willCreate", input.will_create);
    OptionallySet(&result, "didRename", input.did_rename);
    OptionallySet(&result, "willRename", input.will_rename);
    OptionallySet(&result, "didDelete", input.did_delete);
    OptionallySet(&result, "willDelete", input.will_delete);
    return result;
}

struct DidChangeConfigurationClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DidChangeConfigurationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct DidChangeWatchedFilesClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const DidChangeWatchedFilesClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct WorkspaceSymbolClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SymbolKindClientCapabilities> symbol_kind;
    absl::optional<SymbolTagSupportClientCapabilities> tag_support;
};

JSON ToJSON(const WorkspaceSymbolClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "symbolKind", input.symbol_kind);
    OptionallySet(&result, "tagSupport", input.tag_support);
    return result;
}

struct ExecuteCommandClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

JSON ToJSON(const ExecuteCommandClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

struct CodeLensWorkspaceClientCapabilities {
    absl::optional<bool> refresh_support;
};

JSON ToJSON(const CodeLensWorkspaceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "refreshSupport", input.refresh_support);
    return result;
}

struct SemanticTokensWorkspaceClientCapabilities {
    absl::optional<bool> refresh_support;
};

JSON ToJSON(const SemanticTokensWorkspaceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "refreshSupport", input.refresh_support);
    return result;
}

struct WorkspaceSpecificClientCapabilities {
    absl::optional<bool> apply_edit;
    absl::optional<WorkspaceEditClientCapabilities> workspace_edit;
    absl::optional<DidChangeConfigurationClientCapabilities> did_change_configuration;
    absl::optional<DidChangeWatchedFilesClientCapabilities> did_change_watched_files;
    absl::optional<WorkspaceSymbolClientCapabilities> symbol;
    absl::optional<ExecuteCommandClientCapabilities> execute_command;
    absl::optional<bool> workspace_folders;
    absl::optional<bool> configuration;
    absl::optional<SemanticTokensWorkspaceClientCapabilities> semantic_tokens;
    absl::optional<CodeLensWorkspaceClientCapabilities> code_lens;
    absl::optional<FileOperationsWorkspaceClientCapabilities> file_operations;
};

JSON ToJSON(const WorkspaceSpecificClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "applyEdit", input.apply_edit);
    OptionallySet(&result, "workspaceEdit", input.workspace_edit);
    OptionallySet(&result, "didChangeConfiguration", input.did_change_configuration);
    OptionallySet(&result, "didChangeWatchedFiles", input.did_change_watched_files);
    OptionallySet(&result, "symbol", input.symbol);
    OptionallySet(&result, "executeCommand", input.execute_command);
    OptionallySet(&result, "workspaceFolders", input.workspace_folders);
    OptionallySet(&result, "configuration", input.configuration);
    OptionallySet(&result, "semanticTokens", input.semantic_tokens);
    OptionallySet(&result, "codeLens", input.code_lens);
    OptionallySet(&result, "fileOperations", input.file_operations);
    return result;
}

struct MessageActionItemClientCapabilities {
    absl::optional<bool> additional_properties_support;
};

JSON ToJSON(const MessageActionItemClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "additionalPropertiesSupport", input.additional_properties_support);
    return result;
}

struct ShowMessageRequestClientCapabilities {
    absl::optional<MessageActionItemClientCapabilities> message_action_item;
};

JSON ToJSON(const ShowMessageRequestClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "messageActionItem", input.message_action_item);
    return result;
}

struct ShowDocumentClientCapabilities {
    bool support;
};

JSON ToJSON(const ShowDocumentClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "support", input.support);
    return result;
}

struct WindowSpecificClientCapabilities {
    absl::optional<bool> work_done_progress;
    absl::optional<ShowMessageRequestClientCapabilities> show_message;
    absl::optional<ShowDocumentClientCapabilities> show_document;
};

JSON ToJSON(const WindowSpecificClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneProgress", input.work_done_progress);
    OptionallySet(&result, "showMessage", input.show_message);
    OptionallySet(&result, "showDocument", input.show_document);
    return result;
}

struct GeneralClientCapabilities {
    absl::optional<RegularExpressionsClientCapabilities> regular_expressions;
    absl::optional<MarkdownClientCapabilities> markdown;
};

JSON ToJSON(const GeneralClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "regularExpressions", input.regular_expressions);
    OptionallySet(&result, "markdown", input.markdown);
    return result;
}

struct ClientCapabilities {
    absl::optional<WorkspaceSpecificClientCapabilities> workspace;
    absl::optional<TextDocumentClientCapabilities> text_document;
    absl::optional<WindowSpecificClientCapabilities> window;
    absl::optional<GeneralClientCapabilities> general;
    absl::optional<JSON> experimental;
};

JSON ToJSON(const ClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workspace", input.workspace);
    OptionallySet(&result, "textDocument", input.text_document);
    OptionallySet(&result, "window", input.window);
    OptionallySet(&result, "general", input.general);
    OptionallySet(&result, "experimental", input.experimental);
    return result;
}

enum class FileOperationPatternKind {
    File, Folder
};

JSON ToJSON(const FileOperationPatternKind& input) {
    switch (input) {
    case FileOperationPatternKind::File:   return "file";
    case FileOperationPatternKind::Folder: return "folder";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct FileOperationPatternOptions {
    absl::optional<bool> ignore_case;
};

JSON ToJSON(const FileOperationPatternOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "ignoreCase", input.ignore_case);
    return result;
}

struct FileOperationPattern {
    std::string glob;
    absl::optional<FileOperationPatternKind> matches;
    absl::optional<FileOperationPatternOptions> options;
};

JSON ToJSON(const FileOperationPattern& input) {
    JSON result(Json::objectValue);
    Set(&result, "glob", input.glob);
    OptionallySet(&result, "matches", input.matches);
    OptionallySet(&result, "options", input.options);
    return result;
}

struct FileOperationFilter {
    absl::optional<std::string> scheme;
    FileOperationPattern pattern;
};

JSON ToJSON(const FileOperationFilter& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "scheme", input.scheme);
    Set(&result, "pattern", input.pattern);
    return result;
}

struct FileOperationRegistrationOptions {
    std::vector<FileOperationFilter> filters;
};

JSON ToJSON(const FileOperationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "filters", input.filters);
    return result;
}

struct FileOperationsWorkspaceServerCapabilities {
    absl::optional<FileOperationRegistrationOptions> did_create;
    absl::optional<FileOperationRegistrationOptions> will_create;
    absl::optional<FileOperationRegistrationOptions> did_rename;
    absl::optional<FileOperationRegistrationOptions> will_rename;
    absl::optional<FileOperationRegistrationOptions> did_delete;
    absl::optional<FileOperationRegistrationOptions> will_delete;
};

JSON ToJSON(const FileOperationsWorkspaceServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "didCreate", input.did_create);
    OptionallySet(&result, "willCreate", input.will_create);
    OptionallySet(&result, "didRename", input.did_rename);
    OptionallySet(&result, "willRename", input.will_rename);
    OptionallySet(&result, "didDelete", input.did_delete);
    OptionallySet(&result, "willDelete", input.will_delete);
    return result;
}

struct WorkspaceFoldersServerCapabilities {
    absl::optional<bool> supported;
    absl::optional<absl::variant<bool, std::string>> change_notifications;
};

JSON ToJSON(const WorkspaceFoldersServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "supported", input.supported);
    OptionallySet(&result, "changeNotifications", input.change_notifications);
    return result;
}

struct WorkspaceSpecificServerCapabilities {
    absl::optional<WorkspaceFoldersServerCapabilities> workspace_folders;
    absl::optional<FileOperationsWorkspaceServerCapabilities> file_operations;
};

JSON ToJSON(const WorkspaceSpecificServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workspaceFolders", input.workspace_folders);
    OptionallySet(&result, "fileOperations", input.file_operations);
    return result;
}

enum class WatchKind : int32_t {
    Create = 1,
    Change = 2,
    Delete = 4
};

JSON ToJSON(const WatchKind& input) {
    return static_cast<int32_t>(input);
}

struct FileSystemWatcher {
    std::string glob_pattern;
    absl::optional<WatchKind> kind;
};

JSON ToJSON(const FileSystemWatcher& input) {
    JSON result(Json::objectValue);
    Set(&result, "globPattern", input.glob_pattern);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

struct DidChangeWatchedFilesRegistrationOptions {
    std::vector<FileSystemWatcher> watchers;
};

JSON ToJSON(const DidChangeWatchedFilesRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "watchers", input.watchers);
    return result;
}

struct WorkDoneProgressOptions {
    absl::optional<bool> work_done_progress;
};

JSON ToJSON(const WorkDoneProgressOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneProgress", input.work_done_progress);
    return result;
}

struct WorkspaceSymbolOptions {
    WorkDoneProgressOptions underlying_work_done_progress;
};

JSON ToJSON(const WorkspaceSymbolOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    return result;
}

struct WorkspaceSymbolRegistrationOptions {
    WorkspaceSymbolOptions underlying_workspace_symbol_options;
};

JSON ToJSON(const WorkspaceSymbolRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_workspace_symbol_options));
    return result;
}

struct ExecuteCommandOptions {
    WorkDoneProgressOptions underlying_work_done_progress;
    std::vector<std::string> commands;
};

JSON ToJSON(const ExecuteCommandOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Set(&result, "commands", input.commands);
    return result;
}

struct ExecuteCommandRegistrationOptions {
    ExecuteCommandOptions underlying_execute_command_options;
};

JSON ToJSON(const ExecuteCommandRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_execute_command_options));
    return result;
}

enum class TextDocumentSyncKind : int32_t {
    None = 0, Full = 1, Incremental = 2
};

JSON ToJSON(const TextDocumentSyncKind& input) {
    return static_cast<int32_t>(input);
}

struct SaveOptions {
    absl::optional<bool> include_text;
};

JSON ToJSON(const SaveOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "includeText", input.include_text);
    return result;
}

struct TextDocumentSyncOptions {
    absl::optional<bool> open_close;
    absl::optional<TextDocumentSyncKind> change;
    absl::optional<bool> will_save;
    absl::optional<bool> will_save_wait_until;
    absl::optional<absl::variant<bool, SaveOptions>> save;
};

JSON ToJSON(const TextDocumentSyncOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "openClose", input.open_close);
    OptionallySet(&result, "change", input.change);
    OptionallySet(&result, "willSave", input.will_save);
    OptionallySet(&result, "willSaveWaitUntil", input.will_save_wait_until);
    OptionallySet(&result, "save", input.save);
    return result;
}

struct CompletionItemOptions {
    absl::optional<bool> label_details_support;
};

JSON ToJSON(const CompletionItemOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "labelDetailsSupport", input.label_details_support);
    return result;
}

struct CompletionOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<std::string>> trigger_characters;
    absl::optional<std::vector<std::string>> all_commit_characters;
    absl::optional<bool> resolve_provider;
    absl::optional<CompletionItemOptions> completion_item;
};

JSON ToJSON(const CompletionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "triggerCharacters", input.trigger_characters);
    OptionallySet(&result, "allCommitCharacters", input.all_commit_characters);
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    OptionallySet(&result, "completionItem", input.completion_item);
    return result;
}

struct CompletionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CompletionOptions underlying_co;
};

JSON ToJSON(const CompletionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_co));
    return result;
}

struct HoverOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const HoverOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct HoverRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    HoverOptions underlying_ho;
};

JSON ToJSON(const HoverRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ho));
    return result;
}

struct SignatureHelpOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<std::string>> trigger_characters;
    absl::optional<std::vector<std::string>> retrigger_characters;
};

JSON ToJSON(const SignatureHelpOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "triggerCharacters", input.trigger_characters);
    OptionallySet(&result, "retriggerCharacters", input.retrigger_characters);
    return result;
}

struct SignatureHelpRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    SignatureHelpOptions underlying_sho;
};

JSON ToJSON(const SignatureHelpRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sho));
    return result;
}

struct DeclarationOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DeclarationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DeclarationRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    DeclarationOptions underlying_do;
};

JSON ToJSON(const DeclarationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_do));
    return result;
}

struct DefinitionOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DefinitionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DefinitionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DefinitionOptions underlying_do;
};

JSON ToJSON(const DefinitionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_do));
    return result;
}

struct TypeDefinitionOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const TypeDefinitionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct TypeDefinitionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    TypeDefinitionOptions underlying_tdo;
};

JSON ToJSON(const TypeDefinitionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_tdo));
    return result;
}

struct ImplementationOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const ImplementationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct ImplementationRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    ImplementationOptions underlying_io;
};

JSON ToJSON(const ImplementationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_io));
    return result;
}

struct ReferenceOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const ReferenceOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct ReferenceRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    ReferenceOptions underlying_ro;
};

JSON ToJSON(const ReferenceRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ro));
    return result;
}

struct DocumentHighlightOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DocumentHighlightOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DocumentHighlightRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentHighlightOptions underlying_dho;
};

JSON ToJSON(const DocumentHighlightRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dho));
    return result;
}

struct DocumentSymbolOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::string> label;
};

JSON ToJSON(const DocumentSymbolOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "label", input.label);
    return result;
}

struct DocumentSymbolRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentSymbolOptions underlying_dso;
};

JSON ToJSON(const DocumentSymbolRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dso));
    return result;
}

struct CodeActionOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<CodeActionKind>> code_action_kinds;
    absl::optional<bool> resolve_provider;
};

JSON ToJSON(const CodeActionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "codeActionKinds", input.code_action_kinds);
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

struct CodeActionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CodeActionOptions underlying_cao;
};

JSON ToJSON(const CodeActionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_cao));
    return result;
}

struct CodeLensOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> resolve_provider;
};

JSON ToJSON(const CodeLensOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

struct CodeLensRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CodeLensOptions underlying_clo;
};

JSON ToJSON(const CodeLensRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_clo));
    return result;
}

struct DocumentLinkOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> resolve_provider;
};

JSON ToJSON(const DocumentLinkOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

struct DocumentLinkRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentLinkOptions underlying_dlo;
};

JSON ToJSON(const DocumentLinkRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dlo));
    return result;
}

struct DocumentColorOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DocumentColorOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DocumentColorRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    DocumentColorOptions underlying_dco;
};

JSON ToJSON(const DocumentColorRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_dco));
    return result;
}

struct DocumentFormattingOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DocumentFormattingOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DocumentFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentFormattingOptions underlying_dfo;
};

JSON ToJSON(const DocumentFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dfo));
    return result;
}

struct DocumentRangeFormattingOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const DocumentRangeFormattingOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct DocumentRangeFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentRangeFormattingOptions underlying_drfo;
};

JSON ToJSON(const DocumentRangeFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_drfo));
    return result;
}

struct DocumentOnTypeFormattingOptions {
    std::string first_trigger_character;
    absl::optional<std::vector<std::string>> more_trigger_character;
};

JSON ToJSON(const DocumentOnTypeFormattingOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "firstTriggerCharacter", input.first_trigger_character);
    OptionallySet(&result, "moreTriggerCharacter", input.more_trigger_character);
    return result;
}

struct DocumentOnTypeFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentOnTypeFormattingOptions underlying_dotfo;
};

JSON ToJSON(const DocumentOnTypeFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dotfo));
    return result;
}

struct RenameOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> prepare_provider;
};

JSON ToJSON(const RenameOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "prepareProvider", input.prepare_provider);
    return result;
}

struct RenameRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    RenameOptions underlying_ro;
};

JSON ToJSON(const RenameRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ro));
    return result;
}

struct FoldingRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const FoldingRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct FoldingRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    FoldingRangeOptions underlying_fro;
};

JSON ToJSON(const FoldingRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_fro));
    return result;
}

struct SelectionRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const SelectionRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct SelectionRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    SelectionRangeOptions underlying_selro;
};

JSON ToJSON(const SelectionRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_selro));
    return result;
}

struct CallHierarchyOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const CallHierarchyOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct CallHierarchyRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    CallHierarchyOptions underlying_cho;
};

JSON ToJSON(const CallHierarchyRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_cho));
    return result;
}

struct SemanticTokensLegend {
    std::vector<std::string> token_types;
    std::vector<std::string> token_modifiers;
};

JSON ToJSON(const SemanticTokensLegend& input) {
    JSON result(Json::objectValue);
    Set(&result, "tokenTypes", input.token_types);
    Set(&result, "tokenModifiers", input.token_modifiers);
    return result;
}

struct SemanticTokensRangeOptions {
};

JSON ToJSON(const SemanticTokensRangeOptions& input) {
    JSON result(Json::objectValue);
    return result;
}

struct SemanticTokensFullOptions {
    absl::optional<bool> delta;
};

JSON ToJSON(const SemanticTokensFullOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "delta", input.delta);
    return result;
}

struct SemanticTokensOptions {
    WorkDoneProgressOptions underlying_wdpo;
    SemanticTokensLegend legend;
    absl::optional<absl::variant<bool, SemanticTokensRangeOptions>> range;
    absl::optional<absl::variant<bool, SemanticTokensFullOptions>> full;
};

JSON ToJSON(const SemanticTokensOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    Set(&result, "legend", input.legend);
    OptionallySet(&result, "range", input.range);
    OptionallySet(&result, "full", input.full);
    return result;
}

struct SemanticTokensRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    SemanticTokensOptions underlying_sto;
};

JSON ToJSON(const SemanticTokensRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_sto));
    return result;
}

struct LinkedEditingRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const LinkedEditingRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct LinkedEditingRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    LinkedEditingRangeOptions underlying_lero;
};

JSON ToJSON(const LinkedEditingRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_lero));
    return result;
}

struct MonikerOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

JSON ToJSON(const MonikerOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

struct MonikerRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    MonikerOptions underlying_mo;
};

JSON ToJSON(const MonikerRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_mo));
    return result;
}

struct ServerCapabilities {
    absl::optional<TextDocumentSyncOptions> text_document_sync;
    absl::optional<CompletionOptions> completion_provider;
    absl::optional<HoverOptions> hover_provider;
    absl::optional<SignatureHelpOptions> signature_help_provider;
    absl::optional<DeclarationRegistrationOptions> declaration_provider;
    absl::optional<DefinitionOptions> definition_provider;
    absl::optional<TypeDefinitionRegistrationOptions> type_definition_provider;
    absl::optional<ImplementationRegistrationOptions> implementation_provider;
    absl::optional<ReferenceOptions> references_provider;
    absl::optional<DocumentHighlightOptions> document_highlight_provider;
    absl::optional<DocumentSymbolOptions> document_symbol_provider;
    absl::optional<CodeActionOptions> code_action_provider;
    absl::optional<CodeLensOptions> code_lens_provider;
    absl::optional<DocumentLinkOptions> document_link_provider;
    absl::optional<DocumentColorRegistrationOptions> color_provider;
    absl::optional<DocumentFormattingOptions> document_formatting_provider;
    absl::optional<DocumentRangeFormattingOptions> document_range_formatting_provider;
    absl::optional<DocumentOnTypeFormattingOptions> document_on_type_formatting_provider;
    absl::optional<RenameOptions> rename_provider;
    absl::optional<FoldingRangeRegistrationOptions> folding_range_provider;
    absl::optional<ExecuteCommandOptions> execute_command_provider;
    absl::optional<SelectionRangeRegistrationOptions> selection_range_provider;
    absl::optional<LinkedEditingRangeRegistrationOptions> linked_editing_range_provider;
    absl::optional<CallHierarchyRegistrationOptions> call_hierarchy_provider;
    absl::optional<SemanticTokensRegistrationOptions> semantic_tokens_provider;
    absl::optional<MonikerRegistrationOptions> moniker_provider;
    absl::optional<WorkspaceSymbolOptions> workspace_symbol_provider;
    absl::optional<WorkspaceSpecificServerCapabilities> workspace;
    absl::optional<JSON> experimental;
};

JSON ToJSON(const ServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "textDocumentSync", input.text_document_sync);
    OptionallySet(&result, "completionProvider", input.completion_provider);
    OptionallySet(&result, "hoverProvider", input.hover_provider);
    OptionallySet(&result, "signatureHelpProvider", input.signature_help_provider);
    OptionallySet(&result, "declarationProvider", input.declaration_provider);
    OptionallySet(&result, "definitionProvider", input.definition_provider);
    OptionallySet(&result, "typeDefinitionProvider", input.type_definition_provider);
    OptionallySet(&result, "implementationProvider", input.implementation_provider);
    OptionallySet(&result, "referencesProvider", input.references_provider);
    OptionallySet(&result, "documentHighlightProvider", input.document_highlight_provider);
    OptionallySet(&result, "documentSymbolProvider", input.document_symbol_provider);
    OptionallySet(&result, "codeActionProvider", input.code_action_provider);
    OptionallySet(&result, "codeLensProvider", input.code_lens_provider);
    OptionallySet(&result, "documentLinkProvider", input.document_link_provider);
    OptionallySet(&result, "colorProvider", input.color_provider);
    OptionallySet(&result, "documentFormattingProvider", input.document_formatting_provider);
    OptionallySet(&result, "documentRangeFormattingProvider", input.document_range_formatting_provider);
    OptionallySet(&result, "documentOnTypeFormattingProvider", input.document_on_type_formatting_provider);
    OptionallySet(&result, "renameProvider", input.rename_provider);
    OptionallySet(&result, "foldingRangeProvider", input.folding_range_provider);
    OptionallySet(&result, "executeCommandProvider", input.execute_command_provider);
    OptionallySet(&result, "selectionRangeProvider", input.selection_range_provider);
    OptionallySet(&result, "linkedEditingRangeProvider", input.linked_editing_range_provider);
    OptionallySet(&result, "callHierarchyProvider", input.call_hierarchy_provider);
    OptionallySet(&result, "semanticTokensProvider", input.semantic_tokens_provider);
    OptionallySet(&result, "monikerProvider", input.moniker_provider);
    OptionallySet(&result, "workspaceSymbolProvider", input.workspace_symbol_provider);
    OptionallySet(&result, "workspace", input.workspace);
    return result;
}

struct ServerInfo {
    std::string name;
    absl::optional<std::string> version;
};

JSON ToJSON(const ServerInfo& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    OptionallySet(&result, "version", input.version);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceFolder {
    DocumentUri uri;
    std::string name;
};

JSON ToJSON(const WorkspaceFolder& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "name", input.name);
    return result;
}

struct InitializeParams {
    WorkDoneProgressParams underlying_wdpo;
    // nullopt == null here
    absl::optional<int32_t> process_id;
    absl::optional<ClientInfo> client_info;
    absl::optional<std::string> locale;
    absl::optional<absl::optional<std::string>> root_path;
    absl::optional<DocumentUri> root_uri;
    absl::optional<JSON> initialization_options;
    ClientCapabilities capabilities;
    absl::optional<TraceValue> trace;
    absl::optional<std::vector<WorkspaceFolder>> workspace_folders;
};

JSON ToJSON(const InitializeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    if (input.process_id.has_value()) {
        result["processId"] = ToJSON(input.process_id.value());
    } else {
        result["processId"] = JSON();
    }
    OptionallySet(&result, "clientInfo", input.client_info);
    OptionallySet(&result, "locale", input.locale);
    OptionallySet(&result, "rootPath", input.root_path);
    OptionallySet(&result, "rootUri", input.root_uri);
    OptionallySet(&result, "initializationOptions", input.initialization_options);
    Set(&result, "capabilities", input.capabilities);
    OptionallySet(&result, "trace", input.trace);
    OptionallySet(&result, "workspaceFolders", input.workspace_folders);
    return result;
}

struct InitializeResult {
    ServerCapabilities capabilities;
    absl::optional<ServerInfo> server_info;
};

JSON ToJSON(const InitializeResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "capabilities", input.capabilities);
    OptionallySet(&result, "serverInfo", input.server_info);
    return result;
}

enum class InitializeErrorCode : int32_t {
    UnknownProtocolVersion = 1
};

JSON ToJSON(const InitializeErrorCode& input) {
    return static_cast<int32_t>(input);
}

struct InitializeErrorData {
    bool retry;
};

JSON ToJSON(const InitializeErrorData& input) {
    JSON result(Json::objectValue);
    Set(&result, "retry", input.retry);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct ProgressParams {
    ProgressToken token;
    T value;
};

template<typename T>
JSON ToJSON(const ProgressParams<T>& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    Set(&result, "value", input.value);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct InitializedParams {
};

JSON ToJSON(const InitializedParams& input) {
    JSON result(Json::objectValue);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct LogTraceParams {
    std::string message;
    absl::optional<std::string> verbose;
};

JSON ToJSON(const LogTraceParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "message", input.message);
    OptionallySet(&result, "verbose", input.verbose);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct SetTraceParams {
    TraceValue value;
};

JSON ToJSON(const SetTraceParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "value", input.value);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

enum class MessageType : int32_t {
    Error = 1,
    Warning = 2,
    Info = 3,
    Log = 4
};

JSON ToJSON(const MessageType& input) {
    return static_cast<int32_t>(input);
}

struct ShowMessageParams {
    MessageType type;
    std::string message;
};

JSON ToJSON(const ShowMessageParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct MessageActionItem {
    std::string title;
};

JSON ToJSON(const MessageActionItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "title", input.title);
    return result;
}

struct ShowMessageRequestParams {
    MessageType type;
    std::string message;
    absl::optional<std::vector<MessageActionItem>> actions;
};

JSON ToJSON(const ShowMessageRequestParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    OptionallySet(&result, "actions", input.actions);
    return result;
}

struct ShowMessageRequestResult {
    // nullopt == null here
    absl::optional<MessageActionItem> action;
};

JSON ToJSON(const ShowMessageRequestResult& input) {
    if (input.action.has_value()) {
        return ToJSON(input.action);
    } else {
        return JSON();
    }
}

////////////////////////////////////////////////////////////////////////////////

struct ShowDocumentParams {
    URI uri;
    absl::optional<bool> external;
    absl::optional<bool> take_focus;
    absl::optional<Range> selection;
};

JSON ToJSON(const ShowDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "external", input.external);
    OptionallySet(&result, "takeFocus", input.take_focus);
    OptionallySet(&result, "selection", input.selection);
    return result;
}

struct ShowDocumentResult {
    bool success;
};

JSON ToJSON(const ShowDocumentResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "success", input.success);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct LogMessageParams {
    MessageType type;
    std::string message;
};

JSON ToJSON(const LogMessageParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct WorkDoneProgressCreateParams {
    ProgressToken token;
};

JSON ToJSON(const WorkDoneProgressCreateParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct WorkDoneProgressCancelParams {
    ProgressToken token;
};

JSON ToJSON(const WorkDoneProgressCancelParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct Registration {
    std::string id;
    std::string method;
    absl::optional<JSON> register_options;
};

JSON ToJSON(const Registration& input) {
    JSON result(Json::objectValue);
    Set(&result, "id", input.id);
    Set(&result, "method", input.method);
    OptionallySet(&result, "registerOptions", input.register_options);
    return result;
}

struct RegistrationParams {
    std::vector<Registration> registrations;
};

JSON ToJSON(const RegistrationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "registrations", input.registrations);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct Unregistration {
    std::string id;
    std::string method;
};

JSON ToJSON(const Unregistration& input) {
    JSON result(Json::objectValue);
    Set(&result, "id", input.id);
    Set(&result, "method", input.method);
    return result;
}

struct UnregistrationParams {
    std::vector<Unregistration> unregisterations; // sic
};

JSON ToJSON(const UnregistrationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "unregisterations", input.unregisterations);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceFoldersChangeEvent {
    std::vector<WorkspaceFolder> added;
    std::vector<WorkspaceFolder> removed;
};

JSON ToJSON(const WorkspaceFoldersChangeEvent& input) {
    JSON result(Json::objectValue);
    Set(&result, "added", input.added);
    Set(&result, "removed", input.removed);
    return result;
}

struct DidChangeWorkspaceFoldersParams {
    WorkspaceFoldersChangeEvent event;
};

JSON ToJSON(const DidChangeWorkspaceFoldersParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "event", input.event);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DidChangeConfigurationParams {
    JSON settings;
};

JSON ToJSON(const DidChangeConfigurationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "settings", input.settings);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct ConfigurationItem {
    absl::optional<DocumentUri> scope_uri;
    absl::optional<std::string> section;
};

JSON ToJSON(const ConfigurationItem& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "scopeUri", input.scope_uri);
    OptionallySet(&result, "section", input.section);
    return result;
}

struct ConfigurationParams {
    std::vector<ConfigurationItem> items;
};

JSON ToJSON(const ConfigurationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "items", input.items);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

enum class FileChangeType : int32_t {
    Created = 1, Changed = 2, Deleted = 3
};

JSON ToJSON(const FileChangeType& input) {
    return static_cast<int32_t>(input);
}

struct FileEvent {
    DocumentUri uri;
    FileChangeType type;
};

JSON ToJSON(const FileEvent& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "type", input.type);
    return result;
}

struct DidChangeWatchedFilesParams {
    std::vector<FileEvent> changes;
};

JSON ToJSON(const DidChangeWatchedFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "changes", input.changes);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceSymbolParams {
    WorkDoneProgressParams underlying_work_done_progress;
    PartialResultParams underlying_partial_result;
    std::string query;
};

JSON ToJSON(const WorkspaceSymbolParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Merge(&result, ToJSON(input.underlying_partial_result));
    Set(&result, "query", input.query);
    return result;
}

struct SymbolInformation {
    std::string name;
    SymbolKind kind;
    absl::optional<std::vector<SymbolTag>> tags;
    absl::optional<bool> deprecated;
    Location location;
    absl::optional<std::string> container_name;
};

JSON ToJSON(const SymbolInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    Set(&result, "kind", input.kind);
    OptionallySet(&result, "tags", input.tags);
    OptionallySet(&result, "deprecated", input.deprecated);
    Set(&result, "location", input.location);
    OptionallySet(&result, "containerName", input.container_name);
    return result;
}

struct WorkspaceSymbolResult {
    // nullopt == null here
    absl::optional<std::vector<SymbolInformation>> symbols;
};

JSON ToJSON(const WorkspaceSymbolResult& input) {
    JSON result(Json::objectValue);
    if (input.symbols.has_value()) {
        return ToJSON(input.symbols.value());
    } else {
        return JSON();
    }
}

////////////////////////////////////////////////////////////////////////////////

struct ExecuteCommandParams {
    WorkDoneProgressParams underlying_work_done_progress;
    std::string command;
    absl::optional<std::vector<JSON>> arguments;
};

JSON ToJSON(const ExecuteCommandParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Set(&result, "command", input.command);
    OptionallySet(&result, "arguments", input.arguments);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct ApplyWorkspaceEditParams {
    absl::optional<std::string> label;
    WorkspaceEdit edit;
};

JSON ToJSON(const ApplyWorkspaceEditParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "label", input.label);
    Set(&result, "edit", input.edit);
    return result;
}

struct ApplyWorkspaceEditResult {
    bool applied;
    absl::optional<std::string> failure_reason;
    absl::optional<uint32_t> failed_change;
};

JSON ToJSON(const ApplyWorkspaceEditResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "applied", input.applied);
    OptionallySet(&result, "failureReason", input.failure_reason);
    OptionallySet(&result, "failedChange", input.failed_change);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct FileCreate {
    std::string uri;
};

JSON ToJSON(const FileCreate& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

struct CreateFilesParams {
    std::vector<FileCreate> files;
};

JSON ToJSON(const CreateFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct FileRename {
    std::string old_uri;
    std::string new_uri;
};

JSON ToJSON(const FileRename& input) {
    JSON result(Json::objectValue);
    Set(&result, "oldUri", input.old_uri);
    Set(&result, "newUri", input.new_uri);
    return result;
}

struct RenameFilesParams {
    std::vector<FileRename> files;
};

JSON ToJSON(const RenameFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct FileDelete {
    std::string uri;
};

JSON ToJSON(const FileDelete& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

struct DeleteFilesParams {
    std::vector<FileDelete> files;
};

JSON ToJSON(const DeleteFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DidOpenTextDocumentParams {
    TextDocumentItem text_document;
};

JSON ToJSON(const DidOpenTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct TextDocumentChangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    TextDocumentSyncKind sync_kind;
};

JSON ToJSON(const TextDocumentChangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Set(&result, "syncKind", input.sync_kind);
    return result;
}

struct TextDocumentContentChangeEvent {
    absl::optional<Range> range;
    absl::optional<uint32_t> range_length;
    std::string text;
};

JSON ToJSON(const TextDocumentContentChangeEvent& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "range", input.range);
    OptionallySet(&result, "rangeLength", input.range_length);
    Set(&result, "text", input.text);
    return result;
}

struct DidChangeTextDocumentParams {
    VersionedTextDocumentIdentifier text_document;
    std::vector<TextDocumentContentChangeEvent> content_changes;
};

JSON ToJSON(const DidChangeTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "contentChanges", input.content_changes);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

enum class TextDocumentSaveReason : int32_t {
    Manual = 1, AfterDelay = 2, FocusOut = 3
};

JSON ToJSON(const TextDocumentSaveReason& input) {
    return static_cast<int32_t>(input);
}

struct WillSaveTextDocumentParams {
    TextDocumentIdentifier text_document;
    TextDocumentSaveReason reason;
};

JSON ToJSON(const WillSaveTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "reason", input.reason);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct TextDocumentSaveRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    absl::optional<bool> include_text;
};

JSON ToJSON(const TextDocumentSaveRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    OptionallySet(&result, "includeText", input.include_text);
    return result;
}

struct DidSaveTextDocumentParams {
    TextDocumentIdentifier text_document;
    absl::optional<std::string> text;
};

JSON ToJSON(const DidSaveTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    OptionallySet(&result, "text", input.text);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DidCloseTextDocumentParams {
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const DidCloseTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct PublishDiagnosticsParams {
    DocumentUri uri;
    absl::optional<uint32_t> version;
    std::vector<Diagnostic> diagnostics;
};

JSON ToJSON(const PublishDiagnosticsParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "version", input.version);
    Set(&result, "diagnostics", input.diagnostics);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

enum class CompletionTriggerKind : int32_t {
    Invoked = 1, TriggerCharacter = 2, TriggerForIncompleteCompletions = 3
};

JSON ToJSON(const CompletionTriggerKind& input) {
    return static_cast<int32_t>(input);
}

struct CompletionContext {
    CompletionTriggerKind trigger_kind;
    absl::optional<std::string> trigger_character;
};

JSON ToJSON(const CompletionContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "triggerKind", input.trigger_kind);
    OptionallySet(&result, "triggerCharacter", input.trigger_character);
    return result;
}

struct CompletionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    absl::optional<CompletionContext> context;
};

JSON ToJSON(const CompletionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    OptionallySet(&result, "context", input.context);
    return result;
}

enum class InsertTextFormat : int32_t {
    PlainText = 1, Snippet = 2
};

JSON ToJSON(const InsertTextFormat& input) {
    return static_cast<int32_t>(input);
}

struct InsertReplaceEdit {
    std::string new_text;
    Range insert;
    Range replace;
};

JSON ToJSON(const InsertReplaceEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "newText", input.new_text);
    Set(&result, "insert", input.insert);
    Set(&result, "replace", input.replace);
    return result;
}

struct CompletionItemLabelDetails {
    absl::optional<std::string> parameters;
    absl::optional<std::string> qualifier;
    absl::optional<std::string> type;
};

JSON ToJSON(const CompletionItemLabelDetails& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "parameters", input.parameters);
    OptionallySet(&result, "qualifier", input.qualifier);
    OptionallySet(&result, "type", input.type);
    return result;
}

struct CompletionItem {
    std::string label;
    absl::optional<CompletionItemLabelDetails> label_details;
    absl::optional<CompletionItemKind> kind;
    absl::optional<std::vector<CompletionItemTag>> tags;
    absl::optional<std::string> detail;
    absl::optional<absl::variant<std::string, MarkupContent>> documentation;
    absl::optional<bool> deprecated;
    absl::optional<bool> preselect;
    absl::optional<std::string> sort_text;
    absl::optional<std::string> filter_text;
    absl::optional<std::string> insert_text;
    absl::optional<InsertTextFormat> insert_text_format;
    absl::optional<InsertTextMode> insert_text_mode;
    absl::optional<absl::variant<TextEdit, InsertReplaceEdit>> text_edit;
    absl::optional<std::vector<TextEdit>> additional_text_edits;
    absl::optional<std::vector<std::string>> commit_characters;
    absl::optional<Command> command;
    absl::optional<JSON> data;
};

JSON ToJSON(const CompletionItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "labelDetails", input.label_details);
    OptionallySet(&result, "kind", input.kind);
    OptionallySet(&result, "tags", input.tags);
    OptionallySet(&result, "detail", input.detail);
    OptionallySet(&result, "documentation", input.documentation);
    OptionallySet(&result, "deprecated", input.deprecated);
    OptionallySet(&result, "preselect", input.preselect);
    OptionallySet(&result, "sortText", input.sort_text);
    OptionallySet(&result, "filterText", input.filter_text);
    OptionallySet(&result, "insertText", input.insert_text);
    OptionallySet(&result, "insertTextFormat", input.insert_text_format);
    OptionallySet(&result, "insertTextMode", input.insert_text_mode);
    OptionallySet(&result, "textEdit", input.text_edit);
    OptionallySet(&result, "additionalTextEdits", input.additional_text_edits);
    OptionallySet(&result, "commitCharacters", input.commit_characters);
    OptionallySet(&result, "command", input.command);
    OptionallySet(&result, "data", input.data);
    return result;
}

struct CompletionList {
    bool is_incomplete;
    std::vector<CompletionItem> items;
};

JSON ToJSON(const CompletionList& input) {
    JSON result(Json::objectValue);
    Set(&result, "isIncomplete", input.is_incomplete);
    Set(&result, "items", input.items);
    return result;
}

struct CompletionResult {
    absl::variant<std::vector<CompletionItem>, CompletionList, absl::monostate> result;
};

JSON ToJSON(const CompletionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct HoverParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

JSON ToJSON(const HoverParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

struct Hover {
    MarkupContent contents;
    absl::optional<Range> range;
};

JSON ToJSON(const Hover& input) {
    JSON result(Json::objectValue);
    Set(&result, "contents", input.contents);
    OptionallySet(&result, "range", input.range);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

enum class SignatureHelpTriggerKind : int32_t {
    Invoked = 1, TriggerCharacter = 2, ContentChange = 3
};

JSON ToJSON(const SignatureHelpTriggerKind& input) {
    return static_cast<int32_t>(input);
}

struct ParameterInformation {
    absl::variant<std::string, std::pair<uint32_t, uint32_t>> label;
    absl::optional<absl::variant<std::string, MarkupContent>> documentation;
};

JSON ToJSON(const ParameterInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "documentation", input.documentation);
    return result;
}

struct SignatureInformation {
    std::string label;
    absl::optional<absl::variant<std::string, MarkupContent>> documentation;
    absl::optional<std::vector<ParameterInformation>> parameters;
    absl::optional<uint32_t> active_parameter;
};

JSON ToJSON(const SignatureInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "documentation", input.documentation);
    OptionallySet(&result, "parameters", input.parameters);
    OptionallySet(&result, "activeParameter", input.active_parameter);
    return result;
}

struct SignatureHelp {
    std::vector<SignatureInformation> signatures;
    absl::optional<uint32_t> active_signature;
    absl::optional<uint32_t> active_parameter;
};

JSON ToJSON(const SignatureHelp& input) {
    JSON result(Json::objectValue);
    Set(&result, "signatures", input.signatures);
    OptionallySet(&result, "activeSignature", input.active_signature);
    OptionallySet(&result, "activeParameter", input.active_parameter);
    return result;
}

struct SignatureHelpContext {
    SignatureHelpTriggerKind trigger_kind;
    absl::optional<std::string> trigger_character;
    bool is_retrigger;
    absl::optional<SignatureHelp> active_signature_help;
};

JSON ToJSON(const SignatureHelpContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "triggerKind", input.trigger_kind);
    OptionallySet(&result, "triggerCharacter", input.trigger_character);
    Set(&result, "isRetrigger", input.is_retrigger);
    OptionallySet(&result, "activeSignatureHelp", input.active_signature_help);
    return result;
}

struct SignatureHelpParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    absl::optional<SignatureHelpContext> context;
};

JSON ToJSON(const SignatureHelpParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    OptionallySet(&result, "context", input.context);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DeclarationParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const DeclarationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

struct DeclarationResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

JSON ToJSON(const DeclarationResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DefinitionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const DefinitionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

struct DefinitionResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

JSON ToJSON(const DefinitionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct TypeDefinitionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const TypeDefinitionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

struct TypeDefinitionResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

JSON ToJSON(const TypeDefinitionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct ImplementationParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const ImplementationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

struct ImplementationResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

JSON ToJSON(const ImplementationResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct ReferenceContext {
    bool include_declaration;
};

JSON ToJSON(const ReferenceContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "includeDeclaration", input.include_declaration);
    return result;
}

struct ReferenceParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    ReferenceContext context;
};

JSON ToJSON(const ReferenceParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "context", input.context);
    return result;
}

struct ReferenceResult {
    absl::optional<std::vector<Location>> result;
};

JSON ToJSON(const ReferenceResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DocumentHighlightParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const DocumentHighlightParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

enum class DocumentHighlightKind : int32_t {
    Text = 1, Read = 2, Write = 3
};

JSON ToJSON(const DocumentHighlightKind& input) {
    return static_cast<int32_t>(input);
}

struct DocumentHighlight {
    Range range;
    absl::optional<DocumentHighlightKind> kind;
};

JSON ToJSON(const DocumentHighlight& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

////////////////////////////////////////////////////////////////////////////////

struct DocumentSymbolParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const DocumentSymbolParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

struct DocumentSymbol {
    std::string name;
    absl::optional<std::string> detail;
    SymbolKind kind;
    absl::optional<std::vector<SymbolTag>> tags;
    absl::optional<bool> deprecated;
    Range range;
    Range selection_range;
    absl::optional<std::vector<DocumentSymbol>> children;
};

JSON ToJSON(const DocumentSymbol& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    OptionallySet(&result, "detail", input.detail);
    Set(&result, "kind", input.kind);
    OptionallySet(&result, "tags", input.tags);
    OptionallySet(&result, "deprecated", input.deprecated);
    Set(&result, "range", input.range);
    Set(&result, "selectionRange", input.selection_range);
    OptionallySet(&result, "children", input.children);
    return result;
}

using DocumentSymbolResult =
    absl::variant<std::vector<DocumentSymbol>,
                  std::vector<SymbolInformation>,
                  absl::monostate>;

////////////////////////////////////////////////////////////////////////////////

struct CodeActionContext {
    std::vector<Diagnostic> diagnostics;
    absl::optional<std::vector<CodeActionKind>> only;
};

JSON ToJSON(const CodeActionContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "diagnostics", input.diagnostics);
    OptionallySet(&result, "only", input.only);
    return result;
}

struct CodeActionParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Range range;
    CodeActionContext context;
};

JSON ToJSON(const CodeActionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    Set(&result, "context", input.context);
    return result;
}

struct CodeAction {
    std::string title;
    absl::optional<CodeActionKind> kind;
    absl::optional<std::vector<Diagnostic>> diagnostics;
    absl::optional<bool> is_preferred;
    absl::optional<std::string> disabled; // encoded weirdly
    absl::optional<WorkspaceEdit> edit;
    absl::optional<Command> command;
    absl::optional<JSON> data;
};

JSON ToJSON(const CodeAction& input) {
    JSON result(Json::objectValue);
    Set(&result, "title", input.title);
    OptionallySet(&result, "kind", input.kind);
    OptionallySet(&result, "diagnostics", input.diagnostics);
    OptionallySet(&result, "isPreferred", input.is_preferred);
    if (input.disabled.has_value()) {
        JSON disabled(Json::objectValue);
        disabled["reason"] = ToJSON(input.disabled.value());
        result["disabled"] = disabled;
    }
    OptionallySet(&result, "edit", input.edit);
    OptionallySet(&result, "command", input.command);
    OptionallySet(&result, "data", input.data);
    return result;
}

using CodeActionResult =
    absl::optional<std::vector<absl::variant<Command, CodeAction>>>;

////////////////////////////////////////////////////////////////////////////////

struct CodeLensParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const CodeLensParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

struct CodeLens {
    Range range;
    absl::optional<Command> command;
    absl::optional<JSON> data;
};

JSON ToJSON(const CodeLens& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "command", input.command);
    OptionallySet(&result, "data", input.data);
    return result;
}

using CodeLensResult = absl::optional<std::vector<CodeLens>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentLinkParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const DocumentLinkParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

struct DocumentLink {
    Range range;
    absl::optional<DocumentUri> target;
    absl::optional<std::string> tooltip;
    absl::optional<JSON> data;
};

JSON ToJSON(const DocumentLink& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "target", input.target);
    OptionallySet(&result, "tooltip", input.tooltip);
    OptionallySet(&result, "data", input.data);
    return result;
}

using DocumentLinkResult = absl::optional<std::vector<DocumentLink>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentColorParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const DocumentColorParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

struct Color {
    double red;
    double green;
    double blue;
    double alpha;
};

JSON ToJSON(const Color& input) {
    JSON result(Json::objectValue);
    Set(&result, "red", input.red);
    Set(&result, "green", input.green);
    Set(&result, "blue", input.blue);
    Set(&result, "alpha", input.alpha);
    return result;
}

struct ColorInformation {
    Range range;
    Color color;
};

JSON ToJSON(const ColorInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    Set(&result, "color", input.color);
    return result;
}

using DocumentColorResult = std::vector<ColorInformation>;

////////////////////////////////////////////////////////////////////////////////

struct ColorPresentationParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Color color;
    Range range;
};

JSON ToJSON(const ColorPresentationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "color", input.color);
    Set(&result, "range", input.range);
    return result;
}

struct ColorPresentation {
    std::string label;
    absl::optional<TextEdit> text_edit;
    absl::optional<std::vector<TextEdit>> additional_text_edits;
};

JSON ToJSON(const ColorPresentation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "textEdit", input.text_edit);
    OptionallySet(&result, "additionalTextEdits", input.additional_text_edits);
    return result;
}

using ColorPresentationResult = std::vector<ColorPresentation>;

////////////////////////////////////////////////////////////////////////////////

struct FormattingOptions {
    uint32_t tab_size;
    bool insert_spaces;
    absl::optional<bool> trim_trailing_whitespace;
    absl::optional<bool> insert_final_newline;
    absl::optional<bool> trim_final_newlines;

    absl::btree_map<std::string, absl::variant<bool, int32_t, std::string>>
    extra_properties; // encoded weirdly
};

JSON ToJSON(const FormattingOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "tabSize", input.tab_size);
    Set(&result, "insertSpaces", input.insert_spaces);
    OptionallySet(&result, "trimTrailingWhitespace", input.trim_trailing_whitespace);
    OptionallySet(&result, "insertFinalNewline", input.insert_final_newline);
    OptionallySet(&result, "trimFinalNewlines", input.trim_final_newlines);
    Merge(&result, ToJSON(input.extra_properties));
    return result;
}

struct DocumentFormattingParams {
    WorkDoneProgressParams underlying_wdpp;
    TextDocumentIdentifier text_document;
    FormattingOptions options;
};

JSON ToJSON(const DocumentFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "options", input.options);
    return result;
}

using DocumentFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentRangeFormattingParams {
    WorkDoneProgressParams underlying_wdpp;
    TextDocumentIdentifier text_document;
    Range range;
    FormattingOptions options;
};

JSON ToJSON(const DocumentRangeFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    Set(&result, "options", input.options);
    return result;
}

using DocumentRangeFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentOnTypeFormattingParams {
    TextDocumentPositionParams underlying_tdpp;
    std::string ch;
    FormattingOptions options;
};

JSON ToJSON(const DocumentOnTypeFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Set(&result, "ch", input.ch);
    Set(&result, "options", input.options);
    return result;
}

using DocumentOnTypeFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct RenameParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    std::string new_name;
};

JSON ToJSON(const RenameParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "newName", input.new_name);
    return result;
}

using RenameResult = absl::optional<WorkspaceEdit>;

////////////////////////////////////////////////////////////////////////////////

struct PrepareRenameParams {
    TextDocumentPositionParams underlying_tdpp;
};

JSON ToJSON(const PrepareRenameParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    return result;
}

// encoded weirdly
struct PrepareRenameResult {
    absl::optional<Range> range;
    absl::optional<std::string> placeholder;
    absl::optional<bool> default_behavior;
};

JSON ToJSON(const PrepareRenameResult& input) {
    if (input.range.has_value() && input.placeholder.has_value()) {
        RDSS_CHECK(!input.default_behavior.has_value());
        JSON result(Json::objectValue);
        result["range"] = ToJSON(input.range.value());
        result["placeholder"] = ToJSON(input.placeholder.value());
        return result;
    } else if (input.range.has_value()) {
        RDSS_CHECK(!input.default_behavior.has_value());
        return ToJSON(input.range.value());
    } else if (input.default_behavior.has_value()) {
        JSON result(Json::objectValue);
        result["defaultBehavior"] = ToJSON(input.default_behavior.value());
        return result;
    } else {
        return JSON();
    }
}

////////////////////////////////////////////////////////////////////////////////

struct FoldingRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const FoldingRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

enum class FoldingRangeKind {
    Comment, Imports, Region
};

JSON ToJSON(const FoldingRangeKind& input) {
    switch (input) {
    case FoldingRangeKind::Comment: return "comment";
    case FoldingRangeKind::Imports: return "imports";
    case FoldingRangeKind::Region:  return "region";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct FoldingRange {
    uint32_t start_line;
    absl::optional<uint32_t> start_character;
    uint32_t end_line;
    absl::optional<uint32_t> end_character;
    absl::optional<std::string> kind;
};

JSON ToJSON(const FoldingRange& input) {
    JSON result(Json::objectValue);
    Set(&result, "startLine", input.start_line);
    OptionallySet(&result, "startCharacter", input.start_character);
    Set(&result, "endLine", input.end_line);
    OptionallySet(&result, "endCharacter", input.end_character);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

using FoldingRangeResult = absl::optional<std::vector<FoldingRange>>;

////////////////////////////////////////////////////////////////////////////////

struct SelectionRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    std::vector<Position> positions;
};

JSON ToJSON(const SelectionRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "positions", input.positions);
    return result;
}

struct SelectionRange {
    Range range;
    absl::optional<SelectionRange*> parent;
};

JSON ToJSON(const SelectionRange& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    if (input.parent.has_value()) {
        result["parent"] = ToJSON(*(input.parent.value()));
    }
    return result;
}

using SelectionRangeResult = absl::optional<std::vector<SelectionRange>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyPrepareParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

JSON ToJSON(const CallHierarchyPrepareParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

struct CallHierarchyItem {
    std::string name;
    SymbolKind kind;
    absl::optional<std::vector<SymbolTag>> tags;
    absl::optional<std::string> detail;
    DocumentUri uri;
    Range range;
    Range selection_range;
    absl::optional<JSON> data;
};

JSON ToJSON(const CallHierarchyItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    Set(&result, "kind", input.kind);
    OptionallySet(&result, "tags", input.tags);
    OptionallySet(&result, "detail", input.detail);
    Set(&result, "uri", input.uri);
    Set(&result, "range", input.range);
    Set(&result, "selectionRange", input.selection_range);
    OptionallySet(&result, "data", input.data);
    return result;
}

using CallHierarchyPrepareResult =
    absl::optional<std::vector<CallHierarchyItem>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyIncomingCallsParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    CallHierarchyItem item;
};

JSON ToJSON(const CallHierarchyIncomingCallsParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "item", input.item);
    return result;
}

struct CallHierarchyIncomingCall {
    CallHierarchyItem from;
    std::vector<Range> from_ranges;
};

JSON ToJSON(const CallHierarchyIncomingCall& input) {
    JSON result(Json::objectValue);
    Set(&result, "from", input.from);
    Set(&result, "fromRanges", input.from_ranges);
    return result;
}

using CallHierarchyIncomingCallsResult =
    absl::optional<std::vector<CallHierarchyIncomingCall>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyOutgoingCallsParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    CallHierarchyItem item;
};

JSON ToJSON(const CallHierarchyOutgoingCallsParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "item", input.item);
    return result;
}

struct CallHierarchyOutgoingCall {
    CallHierarchyItem to;
    std::vector<Range> from_ranges;
};

JSON ToJSON(const CallHierarchyOutgoingCall& input) {
    JSON result(Json::objectValue);
    Set(&result, "to", input.to);
    Set(&result, "fromRanges", input.from_ranges);
    return result;
}

using CallHierarchyOutgoingCallsResult =
    absl::optional<std::vector<CallHierarchyOutgoingCall>>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

JSON ToJSON(const SemanticTokensParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

struct SemanticTokens {
    absl::optional<std::string> result_id;
    std::vector<uint32_t> data;
};

JSON ToJSON(const SemanticTokens& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "resultId", input.result_id);
    Set(&result, "data", input.data);
    return result;
}

using SemanticTokensResult = absl::optional<SemanticTokens>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensDeltaParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    std::string previous_result_id;
};

JSON ToJSON(const SemanticTokensDeltaParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "previousResultId", input.previous_result_id);
    return result;
}

struct SemanticTokensEdit {
    uint32_t start;
    uint32_t delete_count;
    absl::optional<std::vector<uint32_t>> data;
};

JSON ToJSON(const SemanticTokensEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "start", input.start);
    Set(&result, "deleteCount", input.delete_count);
    OptionallySet(&result, "data", input.data);
    return result;
}

struct SemanticTokensDelta {
    absl::optional<std::string> result_id;
    std::vector<SemanticTokensEdit> edits;
};

JSON ToJSON(const SemanticTokensDelta& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "resultId", input.result_id);
    Set(&result, "edits", input.edits);
    return result;
}

using SemanticTokensDeltaResult =
    absl::variant<SemanticTokens, SemanticTokensDelta, absl::monostate>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Range range;
};

JSON ToJSON(const SemanticTokensRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    return result;
}

using SemanticTokensRangeResult = absl::optional<SemanticTokens>;

////////////////////////////////////////////////////////////////////////////////

struct LinkedEditingRangeParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

JSON ToJSON(const LinkedEditingRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

struct LinkedEditingRanges {
    std::vector<Range> ranges;
    absl::optional<std::string> word_pattern;
};

JSON ToJSON(const LinkedEditingRanges& input) {
    JSON result(Json::objectValue);
    Set(&result, "ranges", input.ranges);
    OptionallySet(&result, "wordPattern", input.word_pattern);
    return result;
}

using LinkedEditingRangeResult = absl::optional<LinkedEditingRanges>;

////////////////////////////////////////////////////////////////////////////////

struct MonikerParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

JSON ToJSON(const MonikerParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

enum class UniquenessLevel {
    Document, Project, Group, Scheme, Global
};

JSON ToJSON(const UniquenessLevel& input) {
    switch (input) {
    case UniquenessLevel::Document: return "document";
    case UniquenessLevel::Project:  return "project";
    case UniquenessLevel::Group:    return "group";
    case UniquenessLevel::Scheme:   return "scheme";
    case UniquenessLevel::Global:   return "global";
    }
    RDSS_CHECK(false);
    return JSON();
}

enum class MonikerKind {
    Import, Export, Local
};

JSON ToJSON(const MonikerKind& input) {
    switch (input) {
    case MonikerKind::Import: return "import";
    case MonikerKind::Export: return "export";
    case MonikerKind::Local:  return "local";
    }
    RDSS_CHECK(false);
    return JSON();
}

struct Moniker {
    std::string scheme;
    std::string identifier;
    UniquenessLevel unique;
    absl::optional<MonikerKind> kind;
};

JSON ToJSON(const Moniker& input) {
    JSON result(Json::objectValue);
    Set(&result, "scheme", input.scheme);
    Set(&result, "identifier", input.identifier);
    Set(&result, "unique", input.unique);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

using MonikerResult = absl::optional<std::vector<Moniker>>;

////////////////////////////////////////////////////////////////////////////////

class ServerToClient {
    void Progress(const ProgressParams<JSON>& params);

    void ShowMessage(const ShowMessageParams& params);

    void LogMessage(const LogMessageParams& params);

    Response<ShowMessageRequestResult>
    ShowMessageRequest(const ShowMessageRequestParams& params);

    Response<absl::monostate>
    WorkDoneProgressCreate(const WorkDoneProgressCreateParams& params);

    void Telemetry(const JSON& params);

    Response<absl::monostate>
    ClientRegisterCapability(const RegistrationParams& params);

    Response<absl::monostate>
    ClientUnregisterCapability(const UnregistrationParams& params);

    Response<absl::optional<std::vector<WorkspaceFolder>>>
    WorkspaceWorkspaceFolders();

    Response<std::vector<JSON>>
    WorkspaceConfiguration(const ConfigurationParams& params);

    Response<ApplyWorkspaceEditResult>
    WorkspaceApplyEdit(const ApplyWorkspaceEditParams& params);

    Response<absl::monostate>
    WorkspaceCodeLensRefresh();

    Response<absl::monostate>
    WorkspaceSemanticTokensRefresh();
};

class ClientToServer {
    void SetTrace(const SetTraceParams& params);

    void LogTrace(const LogTraceParams& params);

    void Exit();

    Response<absl::monostate> Shutdown();

    void Initialized(const InitializedParams& params);

    Response<InitializeResult> Initialize(const InitializeParams& params);

    void Progress(const ProgressParams<WorkDoneProgressBegin>& params);
    void Progress(const ProgressParams<WorkDoneProgressReport>& params);
    void Progress(const ProgressParams<WorkDoneProgressEnd>& params);

    void CancelRequest(RequestId id);

    void WorkDoneProgressCancel(const WorkDoneProgressCancelParams& params);

    void WorkspaceDidChangeWorkspaceFolders(
        const DidChangeWorkspaceFoldersParams& params);

    void WorkspaceDidChangeConfiguration(
        const DidChangeConfigurationParams& params);

    void WorkspaceDidChangeWatchedFiles(
        const DidChangeWatchedFilesParams& params);

    Response<WorkspaceSymbolResult>
    WorkspaceSymbol(const WorkspaceSymbolParams& params);

    Response<JSON> WorkspaceExecuteCommand(const ExecuteCommandParams& params);

    Response<absl::optional<WorkspaceEdit>>
    WorkspaceWillCreateFiles(const CreateFilesParams& params);

    void WorkspaceDidCreateFiles(const CreateFilesParams& params);

    Response<absl::optional<WorkspaceEdit>>
    WorkspaceWillRenameFiles(const RenameFilesParams& params);

    void WorkspaceDidRenameFiles(const RenameFilesParams& params);

    Response<absl::optional<WorkspaceEdit>>
    WorkspaceWillDeleteFiles(const DeleteFilesParams& params);

    void WorkspaceDidDeleteFiles(const DeleteFilesParams& params);

    void TextDocumentDidOpen(const DidOpenTextDocumentParams& params);

    void TextDocumentDidChange(const DidChangeTextDocumentParams& params);

    void TextDocumentWillSave(const WillSaveTextDocumentParams& params);

    Response<absl::optional<std::vector<TextEdit>>>
    TextDocumentWillSaveWaitUntil(const WillSaveTextDocumentParams& params);

    void TextDocumentDidSave(const DidSaveTextDocumentParams& params);

    void TextDocumentDidClose(const DidCloseTextDocumentParams& params);

    void TextDocumentPublishDiagnostics(const PublishDiagnosticsParams& params);

    Response<CompletionResult>
    TextDocumentCompletion(const CompletionParams& params);

    Response<CompletionItem>
    CompletionItemResolve(const CompletionItem& params);

    Response<absl::optional<Hover>>
    TextDocumentHover(const HoverParams& params);

    Response<absl::optional<SignatureHelp>>
    TextDocumentSignatureHelp(const SignatureHelpParams& params);

    Response<DeclarationResult>
    TextDocumentDeclaration(const DeclarationParams& params);

    Response<DefinitionResult>
    TextDocumentDefinition(const DefinitionParams& params);

    Response<TypeDefinitionResult>
    TextDocumentTypeDefinition(const TypeDefinitionParams& params);

    Response<ImplementationResult>
    TextDocumentImplementation(const ImplementationParams& params);

    Response<ReferenceResult>
    TextDocumentReferences(const ReferenceParams& params);

    Response<absl::optional<std::vector<DocumentHighlight>>>
    TextDocumentDocumentHighlight(const DocumentHighlightParams& params);

    Response<DocumentSymbolResult>
    TextDocumentDocumentSymbol(const DocumentSymbolParams& params);

    Response<CodeActionResult>
    TextDocumentCodeAction(const CodeActionParams& params);

    Response<CodeAction>
    CodeActionResolve(const CodeAction& params);

    Response<CodeLensResult>
    TextDocumentCodeLens(const CodeLensParams& params);

    Response<CodeLens>
    CodeLensResolve(const CodeLens& params);

    Response<DocumentLinkResult>
    TextDocumentDocumentLink(const DocumentLinkParams& params);

    Response<DocumentLink>
    DocumentLinkResolve(const DocumentLink& params);

    Response<DocumentColorResult>
    TextDocumentDocumentColor(const DocumentColorParams& params);

    Response<ColorPresentationResult>
    TextDocumentColorPresentation(const ColorPresentationParams& params);

    Response<DocumentFormattingResult>
    TextDocumentFormatting(const DocumentFormattingParams& params);

    Response<DocumentRangeFormattingResult>
    TextDocumentRangeFormatting(const DocumentRangeFormattingParams& params);

    Response<DocumentOnTypeFormattingResult>
    TextDocumentOnTypeFormatting(const DocumentOnTypeFormattingParams& params);

    Response<RenameResult>
    TextDocumentRename(const RenameParams& params);

    Response<PrepareRenameResult>
    TextDocumentPrepareRename(const PrepareRenameParams& params);

    Response<FoldingRangeResult>
    TextDocumentFoldingRange(const FoldingRangeParams& params);

    Response<SelectionRangeResult>
    TextDocumentSelectionRange(const SelectionRangeParams& params);

    Response<CallHierarchyPrepareResult>
    TextDocumentPrepareCallHierarchy(const CallHierarchyPrepareParams& params);

    Response<CallHierarchyIncomingCallsResult>
    CallHierarchyIncomingCalls(const CallHierarchyIncomingCallsParams& params);

    Response<CallHierarchyOutgoingCallsResult>
    CallHierarchyOutgoingCalls(const CallHierarchyOutgoingCallsParams& params);

    Response<SemanticTokensResult>
    TextDocumentSemanticTokensFull(const SemanticTokensParams& params);

    Response<SemanticTokensDeltaResult>
    TextDocumentSemanticTokensFullDelta(const SemanticTokensDeltaParams& params);

    Response<SemanticTokensRangeResult>
    TextDocumentSemanticTokensRange(const SemanticTokensRangeParams& params);

    Response<LinkedEditingRangeResult>
    TextDocumentLinkedEditingRange(const LinkedEditingRangeParams& params);

    Response<MonikerResult>
    TextDocumentMoniker(const MonikerParams& params);
};

}  // namespace lsp

}  // namespace rdss

#endif  // RDSS_LSP_H_
