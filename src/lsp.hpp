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

using RequestId = absl::variant<int32_t, std::string>;

struct ResponseError {
    int32_t code;
    std::string message;
    absl::optional<JSON> data;
};

struct ResponseMessage {
    absl::optional<RequestId> id;
    absl::optional<JSON> result;
    absl::optional<ResponseError> error;
};

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

struct Position {
    uint32_t line;
    uint32_t character;
};

struct Range {
    Position start;
    Position end;
};

struct Location {
    DocumentUri uri;
    Range range;
};

struct LocationLink {
    absl::optional<Range> origin_selection_range;
    DocumentUri target_uri;
    Range target_range;
    Range target_selection_range;
};

enum class DiagnosticSeverity : int32_t {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4
};

struct DiagnosticCode {
    absl::variant<int32_t, std::string> code;
};

struct CodeDescription {
    URI href;
};

enum class DiagnosticTag : int32_t {
    Unnecessary = 1,
    Deprecated = 2
};

struct DiagnosticRelatedInformation {
    Location location;
    std::string message;
};

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

struct Command {
    std::string title;
    std::string command;
    absl::optional<std::vector<JSON>> arguments;
};

struct TextEdit {
    Range range;
    std::string new_text;
};

struct ChangeAnnotation {
    std::string label;
    absl::optional<bool> needs_confirmation;
    absl::optional<std::string> description;
};

using ChangeAnnotationIdentifier = std::string;

struct TextDocumentIdentifier {
    DocumentUri uri;
};

struct TextDocumentItem {
    DocumentUri uri;
    std::string language_id;
    int32_t version;
    std::string text;
};

struct VersionedTextDocumentIdentifier {
    TextDocumentIdentifier underlying;
    int32_t version;
};

struct OptionalVersionedTextDocumentIdentifier {
    TextDocumentIdentifier underlying;
    absl::optional<int32_t> version;
};

struct AnnotatedTextEdit {
    TextEdit underlying;
    ChangeAnnotationIdentifier annotation_id;
};

struct TextDocumentEdit {
    OptionalVersionedTextDocumentIdentifier text_document;
    std::vector<absl::variant<TextEdit, AnnotatedTextEdit>> edits;
};

struct CreateFileOptions {
    absl::optional<bool> overwrite;
    absl::optional<bool> ignore_if_exists;
};

struct CreateFile {
    // kind: 'create'
    DocumentUri uri;
    absl::optional<CreateFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

struct RenameFileOptions {
    absl::optional<bool> overwrite;
    absl::optional<bool> ignore_if_exists;
};

struct RenameFile {
    // kind: 'rename'
    DocumentUri old_uri;
    DocumentUri new_uri;
    absl::optional<RenameFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

struct DeleteFileOptions {
    absl::optional<bool> recursive;
    absl::optional<bool> ignore_if_not_exists;
};

struct DeleteFile {
    // kind: 'delete'
    DocumentUri uri;
    absl::optional<DeleteFileOptions> options;
    absl::optional<ChangeAnnotationIdentifier> annotation_id;
};

struct DocumentChangeOperation {
    absl::variant<TextDocumentEdit,
                  CreateFile,
                  RenameFile,
                  DeleteFile> operation;
};

struct DocumentChanges {
    absl::variant<std::vector<TextDocumentEdit>,
                  std::vector<DocumentChangeOperation>> changes;
};

struct WorkspaceEdit {
    absl::optional<absl::btree_map<DocumentUri, std::vector<TextEdit>>> changes;
    absl::optional<DocumentChanges> document_changes;
    absl::optional<absl::btree_map<ChangeAnnotationIdentifier,
                                   ChangeAnnotation>> change_annotations;
};

enum class ResourceOperationKind {
    Create, Rename, Delete
};

enum class FailureHandlingKind {
    Abort, Transactional, Undo, TextOnlyTransactional
};

struct ChangeAnnotationSupport {
    absl::optional<bool> groups_on_label;
};

struct WorkspaceEditClientCapabilities {
    absl::optional<bool> document_changes;
    absl::optional<std::vector<ResourceOperationKind>> resource_operations;
    absl::optional<FailureHandlingKind> failure_handling;
    absl::optional<bool> normalizes_line_endings;
    absl::optional<ChangeAnnotationSupport> change_annotation_support;
};

struct TextDocumentPositionParams {
    TextDocumentIdentifier text_document;
    Position position;
};

struct DocumentFilter {
    absl::optional<std::string> language;
    absl::optional<std::string> scheme;
    absl::optional<std::string> pattern;
};

using DocumentSelector = std::vector<DocumentFilter>;

struct StaticRegistrationOptions {
    absl::optional<std::string> id;
};

struct TextDocumentRegistrationOptions {
    // nullopt == null here
    absl::optional<DocumentSelector> document_selector;
};

enum class MarkupKind {
    PlainText, Markdown
};

struct MarkupContent {
    MarkupKind kind;
    std::string value;
};

struct MarkdownClientCapabilities {
    std::string parser;
    absl::optional<std::string> version;
};

using ProgressToken = absl::variant<int32_t, std::string>;

struct WorkDoneProgressBegin {
    // kind: 'begin'
    std::string title;
    absl::optional<bool> cancellable;
    absl::optional<std::string> message;
    absl::optional<uint8_t> percentage;
};

struct WorkDoneProgressReport {
    // kind: 'report'
    absl::optional<bool> cancellable;
    absl::optional<std::string> message;
    absl::optional<uint8_t> percentage;
};

struct WorkDoneProgressEnd {
    // kind: 'end'
    absl::optional<std::string> message;
};

struct WorkDoneProgressParams {
    absl::optional<ProgressToken> work_done_token;
};

struct PartialResultParams {
    absl::optional<ProgressToken> partial_result_token;
};

enum class TraceValue {
    Off, Messages, Verbose
};

struct ClientInfo {
    std::string name;
    absl::optional<std::string> version;
};

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

struct SymbolKindClientCapabilities {
    absl::optional<std::vector<SymbolKind>> value_set;
};

enum class SymbolTag : int32_t {
    Deprecated = 1
};

struct SymbolTagSupportClientCapabilities {
    absl::optional<std::vector<SymbolTag>> value_set;
};

struct TextDocumentSyncClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> will_save;
    absl::optional<bool> will_save_wait_until;
    absl::optional<bool> did_save;
};

struct DiagnosticTagSupportClientCapabilities {
    std::vector<DiagnosticTag> value_set;
};

struct PublishDiagnosticsClientCapabilities {
    absl::optional<bool> related_information;
    absl::optional<DiagnosticTagSupportClientCapabilities> tag_support;
    absl::optional<bool> version_support;
    absl::optional<bool> code_description_support;
    absl::optional<bool> data_support;
};

enum class CompletionItemTag : int32_t {
    Deprecated = 1
};

struct CompletionItemTagSupportClientCapabilities {
    std::vector<CompletionItemTag> value_set;
};

struct CompletionItemResolveSupportClientCapabilities {
    std::vector<std::string> properties;
};

enum class InsertTextMode : int32_t {
    AsIs = 1, AdjustIndentation = 2
};

struct CompletionItemInsertTextModeSupportClientCapabilities {
    std::vector<InsertTextMode> value_set;
};

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

struct CompletionItemKindClientCapabilities {
    absl::optional<std::vector<CompletionItemKind>> value_set;
};

struct CompletionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<CompletionItemClientCapabilities> completion_item;
    absl::optional<CompletionItemKindClientCapabilities> completion_item_kind;
    absl::optional<bool> context_support;
    absl::optional<InsertTextMode> insert_text_mode;
};

struct HoverClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<std::vector<MarkupKind>> content_format;
};

struct ParameterInformationClientCapabilities {
    absl::optional<bool> label_offset_support;
};

struct SignatureInformationClientCapabilities {
    absl::optional<std::vector<MarkupKind>> documentation_format;
    absl::optional<ParameterInformationClientCapabilities> parameter_information;
    absl::optional<bool> active_parameter_support;
};

struct SignatureHelpClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SignatureInformationClientCapabilities> signature_information;
    absl::optional<bool> context_support;
};

struct DeclarationClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

struct DefinitionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

struct TypeDefinitionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

struct ImplementationClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> link_support;
};

struct ReferenceClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentHighlightClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentSymbolClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SymbolKindClientCapabilities> symbol_kind;
    absl::optional<bool> hierarchical_document_symbol_support;
    absl::optional<SymbolTagSupportClientCapabilities> tag_support;
    absl::optional<bool> label_support;
};

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

struct CodeActionKindClientCapabilities {
    std::vector<CodeActionKind> value_set;
};

struct CodeActionLiteralClientCapabilities {
    CodeActionKindClientCapabilities code_action_kind;
};

struct CodeActionResolveClientCapabilities {
    std::vector<std::string> properties;
};

struct CodeActionClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<CodeActionLiteralClientCapabilities> code_action_literal_support;
    absl::optional<bool> is_preferred_support;
    absl::optional<bool> disabled_support;
    absl::optional<bool> data_support;
    absl::optional<CodeActionResolveClientCapabilities> resolve_support;
    absl::optional<bool> honors_change_annotations;
};

struct CodeLensClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentLinkClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> tooltip_support;
};

struct DocumentColorClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentRangeFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DocumentOnTypeFormattingClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

enum class PrepareSupportDefaultBehavior : int32_t {
    Identifier = 1
};

struct RenameClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> prepare_support;
    absl::optional<PrepareSupportDefaultBehavior> prepare_support_default_behavior;
    absl::optional<bool> honors_change_annotations;
};

struct FoldingRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<uint32_t> range_limit;
    absl::optional<bool> line_folding_only;
};

struct SelectionRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct CallHierarchyClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct SemanticTokensRangeClientCapabilities {
};

struct SemanticTokensFullClientCapabilities {
    absl::optional<bool> delta;
};

enum class TokenFormat {
    Relative
};

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

struct LinkedEditingRangeClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct MonikerClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

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

struct FileOperationsWorkspaceClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<bool> did_create;
    absl::optional<bool> will_create;
    absl::optional<bool> did_rename;
    absl::optional<bool> will_rename;
    absl::optional<bool> did_delete;
    absl::optional<bool> will_delete;
};

struct DidChangeConfigurationClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct DidChangeWatchedFilesClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct WorkspaceSymbolClientCapabilities {
    absl::optional<bool> dynamic_registration;
    absl::optional<SymbolKindClientCapabilities> symbol_kind;
    absl::optional<SymbolTagSupportClientCapabilities> tag_support;
};

struct ExecuteCommandClientCapabilities {
    absl::optional<bool> dynamic_registration;
};

struct CodeLensWorkspaceClientCapabilities {
    absl::optional<bool> refresh_support;
};

struct SemanticTokensWorkspaceClientCapabilities {
    absl::optional<bool> refresh_support;
};

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

struct MessageActionItemClientCapabilities {
    absl::optional<bool> additional_properties_support;
};

struct ShowMessageRequestClientCapabilities {
    absl::optional<MessageActionItemClientCapabilities> message_action_item;
};

struct ShowDocumentClientCapabilities {
    bool support;
};

struct WindowSpecificClientCapabilities {
    absl::optional<bool> work_done_progress;
    absl::optional<ShowMessageRequestClientCapabilities> show_message;
    absl::optional<ShowDocumentClientCapabilities> show_document;
};

struct GeneralClientCapabilities {
    absl::optional<RegularExpressionsClientCapabilities> regular_expressions;
    absl::optional<MarkdownClientCapabilities> markdown;
};

struct ClientCapabilities {
    absl::optional<WorkspaceSpecificClientCapabilities> workspace;
    absl::optional<TextDocumentClientCapabilities> text_document;
    absl::optional<WindowSpecificClientCapabilities> window;
    absl::optional<GeneralClientCapabilities> general;
    absl::optional<JSON> experimental;
};

enum class FileOperationPatternKind {
    File, Folder
};

struct FileOperationPatternOptions {
    absl::optional<bool> ignore_case;
};

struct FileOperationPattern {
    std::string glob;
    absl::optional<FileOperationPatternKind> matches;
    absl::optional<FileOperationPatternOptions> options;
};

struct FileOperationFilter {
    absl::optional<std::string> scheme;
    FileOperationPattern pattern;
};

struct FileOperationRegistrationOptions {
    std::vector<FileOperationFilter> filters;
};

struct FileOperationsWorkspaceServerCapabilities {
    absl::optional<FileOperationRegistrationOptions> did_create;
    absl::optional<FileOperationRegistrationOptions> will_create;
    absl::optional<FileOperationRegistrationOptions> did_rename;
    absl::optional<FileOperationRegistrationOptions> will_rename;
    absl::optional<FileOperationRegistrationOptions> did_delete;
    absl::optional<FileOperationRegistrationOptions> will_delete;
};

struct WorkspaceFoldersServerCapabilities {
    absl::optional<bool> supported;
    absl::optional<absl::variant<bool, std::string>> change_notifications;
};

struct WorkspaceSpecificServerCapabilities {
    absl::optional<WorkspaceFoldersServerCapabilities> workspace_folders;
    absl::optional<FileOperationsWorkspaceServerCapabilities> file_operations;
};

enum class WatchKind : int32_t {
    Create = 1,
    Change = 2,
    Delete = 4
};

struct FileSystemWatcher {
    std::string glob_pattern;
    absl::optional<WatchKind> kind;
};

struct DidChangeWatchedFilesRegistrationOptions {
    std::vector<FileSystemWatcher> watchers;
};

struct WorkDoneProgressOptions {
    absl::optional<bool> work_done_progress;
};

struct WorkspaceSymbolOptions {
    WorkDoneProgressOptions underlying_work_done_progress;
};

struct WorkspaceSymbolRegistrationOptions {
    WorkspaceSymbolOptions underlying_workspace_symbol_options;
};

struct ExecuteCommandOptions {
    WorkDoneProgressOptions underlying_work_done_progress;
    std::vector<std::string> commands;
};

struct ExecuteCommandRegistrationOptions {
    ExecuteCommandOptions underlying_execute_command_options;
};

enum class TextDocumentSyncKind : int32_t {
    None = 0, Full = 1, Incremental = 2
};

struct SaveOptions {
    absl::optional<bool> include_text;
};

struct TextDocumentSyncOptions {
    absl::optional<bool> open_close;
    absl::optional<TextDocumentSyncKind> change;
    absl::optional<bool> will_save;
    absl::optional<bool> will_save_wait_until;
    absl::optional<absl::variant<bool, SaveOptions>> save;
};

struct CompletionItemOptions {
    absl::optional<bool> label_details_support;
};

struct CompletionOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<std::string>> trigger_characters;
    absl::optional<std::vector<std::string>> all_commit_characters;
    absl::optional<bool> resolve_provider;
    absl::optional<CompletionItemOptions> completion_item;
};

struct CompletionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CompletionOptions underlying_co;
};

struct HoverOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct HoverRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    HoverOptions underlying_ho;
};

struct SignatureHelpOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<std::string>> trigger_characters;
    absl::optional<std::vector<std::string>> retrigger_characters;
};

struct SignatureHelpRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    SignatureHelpOptions underlying_sho;
};

struct DeclarationOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DeclarationRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    DeclarationOptions underlying_do;
};

struct DefinitionOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DefinitionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DefinitionOptions underlying_do;
};

struct TypeDefinitionOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct TypeDefinitionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    TypeDefinitionOptions underlying_tdo;
};

struct ImplementationOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct ImplementationRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    ImplementationOptions underlying_io;
};

struct ReferenceOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct ReferenceRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    ReferenceOptions underlying_ro;
};

struct DocumentHighlightOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DocumentHighlightRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentHighlightOptions underlying_dho;
};

struct DocumentSymbolOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::string> label;
};

struct DocumentSymbolRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentSymbolOptions underlying_dso;
};

struct CodeActionOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<std::vector<CodeActionKind>> code_action_kinds;
    absl::optional<bool> resolve_provider;
};

struct CodeActionRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CodeActionOptions underlying_cao;
};

struct CodeLensOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> resolve_provider;
};

struct CodeLensRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    CodeLensOptions underlying_clo;
};

struct DocumentLinkOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> resolve_provider;
};

struct DocumentLinkRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentLinkOptions underlying_dlo;
};

struct DocumentColorOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DocumentColorRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    DocumentColorOptions underlying_dco;
};

struct DocumentFormattingOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DocumentFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentFormattingOptions underlying_dfo;
};

struct DocumentRangeFormattingOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct DocumentRangeFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentRangeFormattingOptions underlying_drfo;
};

struct DocumentOnTypeFormattingOptions {
    std::string first_trigger_character;
    absl::optional<std::vector<std::string>> more_trigger_character;
};

struct DocumentOnTypeFormattingRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    DocumentOnTypeFormattingOptions underlying_dotfo;
};

struct RenameOptions {
    WorkDoneProgressOptions underlying_wdpo;
    absl::optional<bool> prepare_provider;
};

struct RenameRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    RenameOptions underlying_ro;
};

struct FoldingRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct FoldingRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    FoldingRangeOptions underlying_fro;
};

struct SelectionRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct SelectionRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    SelectionRangeOptions underlying_selro;
};

struct CallHierarchyOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct CallHierarchyRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    CallHierarchyOptions underlying_cho;
};

struct SemanticTokensLegend {
    std::vector<std::string> token_types;
    std::vector<std::string> token_modifiers;
};

struct SemanticTokensRangeOptions {
};

struct SemanticTokensFullOptions {
    absl::optional<bool> delta;
};

struct SemanticTokensOptions {
    WorkDoneProgressOptions underlying_wdpo;
    SemanticTokensLegend legend;
    absl::optional<absl::variant<bool, SemanticTokensRangeOptions>> range;
    absl::optional<absl::variant<bool, SemanticTokensFullOptions>> full;
};

struct SemanticTokensRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    SemanticTokensOptions underlying_sto;
};

struct LinkedEditingRangeOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct LinkedEditingRangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    StaticRegistrationOptions underlying_sro;
    LinkedEditingRangeOptions underlying_lero;
};

struct MonikerOptions {
    WorkDoneProgressOptions underlying_wdpo;
};

struct MonikerRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    MonikerOptions underlying_mo;
};

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

struct ServerInfo {
    std::string name;
    absl::optional<std::string> version;
};

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceFolder {
    DocumentUri uri;
    std::string name;
};

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

struct InitializeResult {
    ServerCapabilities capabilities;
    absl::optional<ServerInfo> server_info;
};

enum class InitializeErrorCode : int32_t {
    UnknownProtocolVersion = 1
};

struct InitializeErrorData {
    bool retry;
};

////////////////////////////////////////////////////////////////////////////////

template<typename T>
struct ProgressParams {
    ProgressToken token;
    T value;
};

////////////////////////////////////////////////////////////////////////////////

struct InitializedParams {
};

////////////////////////////////////////////////////////////////////////////////

struct LogTraceParams {
    std::string message;
    absl::optional<std::string> verbose;
};

////////////////////////////////////////////////////////////////////////////////

struct SetTraceParams {
    TraceValue value;
};

////////////////////////////////////////////////////////////////////////////////

enum class MessageType : int32_t {
    Error = 1,
    Warning = 2,
    Info = 3,
    Log = 4
};

struct ShowMessageParams {
    MessageType type;
    std::string message;
};

////////////////////////////////////////////////////////////////////////////////

struct MessageActionItem {
    std::string title;
};

struct ShowMessageRequestParams {
    MessageType type;
    std::string message;
    absl::optional<std::vector<MessageActionItem>> actions;
};

struct ShowMessageRequestResult {
    // nullopt == null here
    absl::optional<MessageActionItem> action;
};

////////////////////////////////////////////////////////////////////////////////

struct ShowDocumentParams {
    URI uri;
    absl::optional<bool> external;
    absl::optional<bool> take_focus;
    absl::optional<Range> selection;
};

struct ShowDocumentResult {
    bool success;
};

////////////////////////////////////////////////////////////////////////////////

struct LogMessageParams {
    MessageType type;
    std::string message;
};

////////////////////////////////////////////////////////////////////////////////

struct WorkDoneProgressCreateParams {
    ProgressToken token;
};

////////////////////////////////////////////////////////////////////////////////

struct WorkDoneProgressCancelParams {
    ProgressToken token;
};

////////////////////////////////////////////////////////////////////////////////

struct Registration {
    std::string id;
    std::string method;
    absl::optional<JSON> register_options;
};

struct RegistrationParams {
    std::vector<Registration> registrations;
};

////////////////////////////////////////////////////////////////////////////////

struct Unregistration {
    std::string id;
    std::string method;
};

struct UnregistrationParams {
    std::vector<Unregistration> unregisterations; // sic
};

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceFoldersChangeEvent {
    std::vector<WorkspaceFolder> added;
    std::vector<WorkspaceFolder> removed;
};

struct DidChangeWorkspaceFoldersParams {
    WorkspaceFoldersChangeEvent event;
};

////////////////////////////////////////////////////////////////////////////////

struct DidChangeConfigurationParams {
    JSON settings;
};

////////////////////////////////////////////////////////////////////////////////

struct ConfigurationItem {
    absl::optional<DocumentUri> scope_uri;
    absl::optional<std::string> section;
};

struct ConfigurationParams {
    std::vector<ConfigurationItem> items;
};

////////////////////////////////////////////////////////////////////////////////

enum class FileChangeType : int32_t {
    Created = 1, Changed = 2, Deleted = 3
};

struct FileEvent {
    DocumentUri uri;
    FileChangeType type;
};

struct DidChangeWatchedFilesParams {
    std::vector<FileEvent> changes;
};

////////////////////////////////////////////////////////////////////////////////

struct WorkspaceSymbolParams {
    WorkDoneProgressParams underlying_work_done_progress;
    PartialResultParams underlying_partial_result;
    std::string query;
};

struct SymbolInformation {
    std::string name;
    SymbolKind kind;
    absl::optional<std::vector<SymbolTag>> tags;
    absl::optional<bool> deprecated;
    Location location;
    absl::optional<std::string> container_name;
};

struct WorkspaceSymbolResult {
    // nullopt == null here
    absl::optional<std::vector<SymbolInformation>> symbols;
};

////////////////////////////////////////////////////////////////////////////////

struct ExecuteCommandParams {
    WorkDoneProgressParams underlying_work_done_progress;
    std::string command;
    absl::optional<std::vector<JSON>> arguments;
};

////////////////////////////////////////////////////////////////////////////////

struct ApplyWorkspaceEditParams {
    absl::optional<std::string> label;
    WorkspaceEdit edit;
};

struct ApplyWorkspaceEditResult {
    bool applied;
    absl::optional<std::string> failure_reason;
    absl::optional<uint32_t> failed_change;
};

////////////////////////////////////////////////////////////////////////////////

struct FileCreate {
    std::string uri;
};

struct CreateFilesParams {
    std::vector<FileCreate> files;
};

////////////////////////////////////////////////////////////////////////////////

struct FileRename {
    std::string old_uri;
    std::string new_uri;
};

struct RenameFilesParams {
    std::vector<FileRename> files;
};

////////////////////////////////////////////////////////////////////////////////

struct FileDelete {
    std::string uri;
};

struct DeleteFilesParams {
    std::vector<FileDelete> files;
};

////////////////////////////////////////////////////////////////////////////////

struct DidOpenTextDocumentParams {
    TextDocumentItem text_document;
};

////////////////////////////////////////////////////////////////////////////////

struct TextDocumentChangeRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    TextDocumentSyncKind sync_kind;
};

struct TextDocumentContentChangeEvent {
    absl::optional<Range> range;
    absl::optional<uint32_t> range_length;
    std::string text;
};

struct DidChangeTextDocumentParams {
    VersionedTextDocumentIdentifier text_document;
    std::vector<TextDocumentContentChangeEvent> content_changes;
};

////////////////////////////////////////////////////////////////////////////////

enum class TextDocumentSaveReason : int32_t {
    Manual = 1, AfterDelay = 2, FocusOut = 3
};

struct WillSaveTextDocumentParams {
    TextDocumentIdentifier text_document;
    TextDocumentSaveReason reason;
};

////////////////////////////////////////////////////////////////////////////////

struct TextDocumentSaveRegistrationOptions {
    TextDocumentRegistrationOptions underlying_tdro;
    absl::optional<bool> include_text;
};

struct DidSaveTextDocumentParams {
    TextDocumentIdentifier text_document;
    absl::optional<std::string> text;
};

////////////////////////////////////////////////////////////////////////////////

struct DidCloseTextDocumentParams {
    TextDocumentIdentifier text_document;
};

////////////////////////////////////////////////////////////////////////////////

struct PublishDiagnosticsParams {
    DocumentUri uri;
    absl::optional<uint32_t> version;
    std::vector<Diagnostic> diagnostics;
};

////////////////////////////////////////////////////////////////////////////////

enum class CompletionTriggerKind : int32_t {
    Invoked = 1, TriggerCharacter = 2, TriggerForIncompleteCompletions = 3
};

struct CompletionContext {
    CompletionTriggerKind trigger_kind;
    absl::optional<std::string> trigger_character;
};

struct CompletionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    absl::optional<CompletionContext> context;
};

enum class InsertTextFormat : int32_t {
    PlainText = 1, Snippet = 2
};

struct InsertReplaceEdit {
    std::string new_text;
    Range insert;
    Range replace;
};

struct CompletionItemLabelDetails {
    absl::optional<std::string> parameters;
    absl::optional<std::string> qualifier;
    absl::optional<std::string> type;
};

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

struct CompletionList {
    bool is_incomplete;
    std::vector<CompletionItem> items;
};

struct CompletionResult {
    absl::variant<std::vector<CompletionItem>, CompletionList, absl::monostate> result;
};

////////////////////////////////////////////////////////////////////////////////

struct HoverParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

struct Hover {
    MarkupContent contents;
    absl::optional<Range> range;
};

////////////////////////////////////////////////////////////////////////////////

enum class SignatureHelpTriggerKind : int32_t {
    Invoked = 1, TriggerCharacter = 2, ContentChange = 3
};

struct ParameterInformation {
    absl::variant<std::string, std::pair<uint32_t, uint32_t>> label;
    absl::optional<absl::variant<std::string, MarkupContent>> documentation;
};

struct SignatureInformation {
    std::string label;
    absl::optional<absl::variant<std::string, MarkupContent>> documentation;
    absl::optional<std::vector<ParameterInformation>> parameters;
    absl::optional<uint32_t> active_parameter;
};

struct SignatureHelp {
    std::vector<SignatureInformation> signatures;
    absl::optional<uint32_t> active_signature;
    absl::optional<uint32_t> active_parameter;
};

struct SignatureHelpContext {
    SignatureHelpTriggerKind trigger_kind;
    absl::optional<std::string> trigger_character;
    bool is_retrigger;
    absl::optional<SignatureHelp> active_signature_help;
};

struct SignatureHelpParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    absl::optional<SignatureHelpContext> context;
};

////////////////////////////////////////////////////////////////////////////////

struct DeclarationParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

struct DeclarationResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

////////////////////////////////////////////////////////////////////////////////

struct DefinitionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

struct DefinitionResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

////////////////////////////////////////////////////////////////////////////////

struct TypeDefinitionParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

struct TypeDefinitionResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

////////////////////////////////////////////////////////////////////////////////

struct ImplementationParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

struct ImplementationResult {
    absl::variant<Location, std::vector<Location>, std::vector<LocationLink>, absl::monostate> result;
};

////////////////////////////////////////////////////////////////////////////////

struct ReferenceContext {
    bool include_declaration;
};


struct ReferenceParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    ReferenceContext context;
};

struct ReferenceResult {
    absl::optional<std::vector<Location>> result;
};

////////////////////////////////////////////////////////////////////////////////

struct DocumentHighlightParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

enum class DocumentHighlightKind : int32_t {
    Text = 1, Read = 2, Write = 3
};

struct DocumentHighlight {
    Range range;
    absl::optional<DocumentHighlightKind> kind;
};

////////////////////////////////////////////////////////////////////////////////

struct DocumentSymbolParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

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

using DocumentSymbolResult =
    absl::variant<std::vector<DocumentSymbol>,
                  std::vector<SymbolInformation>,
                  absl::monostate>;

////////////////////////////////////////////////////////////////////////////////

struct CodeActionContext {
    std::vector<Diagnostic> diagnostics;
    absl::optional<std::vector<CodeActionKind>> only;
};

struct CodeActionParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Range range;
    CodeActionContext context;
};

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

using CodeActionResult =
    absl::optional<std::vector<absl::variant<Command, CodeAction>>>;

////////////////////////////////////////////////////////////////////////////////

struct CodeLensParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

struct CodeLens {
    Range range;
    absl::optional<Command> command;
    absl::optional<JSON> data;
};

using CodeLensResult = absl::optional<std::vector<CodeLens>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentLinkParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

struct DocumentLink {
    Range range;
    absl::optional<DocumentUri> target;
    absl::optional<std::string> tooltip;
    absl::optional<JSON> data;
};

using DocumentLinkResult = absl::optional<std::vector<DocumentLink>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentColorParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

struct Color {
    double red;
    double green;
    double blue;
    double alpha;
};

struct ColorInformation {
    Range range;
    Color color;
};

using DocumentColorResult = std::vector<ColorInformation>;

////////////////////////////////////////////////////////////////////////////////

struct ColorPresentationParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Color color;
    Range range;
};

struct ColorPresentation {
    std::string label;
    absl::optional<TextEdit> text_edit;
    absl::optional<std::vector<TextEdit>> additional_text_edits;
};

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

struct DocumentFormattingParams {
    WorkDoneProgressParams underlying_wdpp;
    TextDocumentIdentifier text_document;
    FormattingOptions options;
};

using DocumentFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentRangeFormattingParams {
    WorkDoneProgressParams underlying_wdpp;
    TextDocumentIdentifier text_document;
    Range range;
    FormattingOptions options;
};

using DocumentRangeFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct DocumentOnTypeFormattingParams {
    TextDocumentPositionParams underlying_tdpp;
    std::string ch;
    FormattingOptions options;
};

using DocumentOnTypeFormattingResult = absl::optional<std::vector<TextEdit>>;

////////////////////////////////////////////////////////////////////////////////

struct RenameParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    std::string new_name;
};

using RenameResult = absl::optional<WorkspaceEdit>;

////////////////////////////////////////////////////////////////////////////////

struct PrepareRenameParams {
    TextDocumentPositionParams underlying_tdpp;
};

// encoded weirdly
struct PrepareRenameResult {
    absl::optional<Range> range;
    absl::optional<std::string> placeholder;
    absl::optional<bool> default_behavior;
};

////////////////////////////////////////////////////////////////////////////////

struct FoldingRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

enum class FoldingRangeKind {
    Comment, Imports, Region
};

struct FoldingRange {
    uint32_t start_line;
    absl::optional<uint32_t> start_character;
    uint32_t end_line;
    absl::optional<uint32_t> end_character;
    absl::optional<std::string> kind;
};

using FoldingRangeResult = absl::optional<std::vector<FoldingRange>>;

////////////////////////////////////////////////////////////////////////////////

struct SelectionRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    std::vector<Position> positions;
};

struct SelectionRange {
    Range range;
    absl::optional<SelectionRange*> parent;
};

using SelectionRangeResult = absl::optional<std::vector<SelectionRange>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyPrepareParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

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

using CallHierarchyPrepareResult =
    absl::optional<std::vector<CallHierarchyItem>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyIncomingCallsParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    CallHierarchyItem item;
};

struct CallHierarchyIncomingCall {
    CallHierarchyItem from;
    std::vector<Range> from_ranges;
};

using CallHierarchyIncomingCallsResult =
    absl::optional<std::vector<CallHierarchyIncomingCall>>;

////////////////////////////////////////////////////////////////////////////////

struct CallHierarchyOutgoingCallsParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    CallHierarchyItem item;
};

struct CallHierarchyOutgoingCall {
    CallHierarchyItem to;
    std::vector<Range> from_ranges;
};

using CallHierarchyOutgoingCallsResult =
    absl::optional<std::vector<CallHierarchyOutgoingCall>>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
};

struct SemanticTokens {
    absl::optional<std::string> result_id;
    std::vector<uint32_t> data;
};

using SemanticTokensResult = absl::optional<SemanticTokens>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensDeltaParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    std::string previous_result_id;
};

struct SemanticTokensEdit {
    uint32_t start;
    uint32_t delete_count;
    absl::optional<std::vector<uint32_t>> data;
};

struct SemanticTokensDelta {
    absl::optional<std::string> result_id;
    std::vector<SemanticTokensEdit> edits;
};

using SemanticTokensDeltaResult =
    absl::variant<SemanticTokens, SemanticTokensDelta, absl::monostate>;

////////////////////////////////////////////////////////////////////////////////

struct SemanticTokensRangeParams {
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
    TextDocumentIdentifier text_document;
    Range range;
};

using SemanticTokensRangeResult = absl::optional<SemanticTokens>;

////////////////////////////////////////////////////////////////////////////////

struct LinkedEditingRangeParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
};

struct LinkedEditingRanges {
    std::vector<Range> ranges;
    absl::optional<std::string> word_pattern;
};

using LinkedEditingRangeResult = absl::optional<LinkedEditingRanges>;

////////////////////////////////////////////////////////////////////////////////

struct MonikerParams {
    TextDocumentPositionParams underlying_tdpp;
    WorkDoneProgressParams underlying_wdpp;
    PartialResultParams underlying_prp;
};

enum class UniquenessLevel {
    Document, Project, Group, Scheme, Global
};

enum class MonikerKind {
    Import, Export, Local
};

struct Moniker {
    std::string scheme;
    std::string identifier;
    UniquenessLevel unique;
    absl::optional<MonikerKind> kind;
};

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

////////////////////////////////////////////////////////////////////////////////

JSON ToJSON(const ErrorCodes& input);
JSON ToJSON(const ResponseError& input);
JSON ToJSON(const ResponseMessage& input);
JSON ToJSON(const RegularExpressionsClientCapabilities& input);
JSON ToJSON(const Position& input);
JSON ToJSON(const Range& input);
JSON ToJSON(const Location& input);
JSON ToJSON(const LocationLink& input);
JSON ToJSON(const DiagnosticSeverity& input);
JSON ToJSON(const DiagnosticCode& input);
JSON ToJSON(const CodeDescription& input);
JSON ToJSON(const DiagnosticTag& input);
JSON ToJSON(const DiagnosticRelatedInformation& input);
JSON ToJSON(const Diagnostic& input);
JSON ToJSON(const Command& input);
JSON ToJSON(const TextEdit& input);
JSON ToJSON(const ChangeAnnotation& input);
JSON ToJSON(const TextDocumentIdentifier& input);
JSON ToJSON(const TextDocumentItem& input);
JSON ToJSON(const VersionedTextDocumentIdentifier& input);
JSON ToJSON(const OptionalVersionedTextDocumentIdentifier& input);
JSON ToJSON(const AnnotatedTextEdit& input);
JSON ToJSON(const TextDocumentEdit& input);
JSON ToJSON(const CreateFileOptions& input);
JSON ToJSON(const CreateFile& input);
JSON ToJSON(const RenameFileOptions& input);
JSON ToJSON(const RenameFile& input);
JSON ToJSON(const DeleteFileOptions& input);
JSON ToJSON(const DeleteFile& input);
JSON ToJSON(const DocumentChangeOperation& input);
JSON ToJSON(const DocumentChanges& input);
JSON ToJSON(const WorkspaceEdit& input);
JSON ToJSON(const ResourceOperationKind& input);
JSON ToJSON(const FailureHandlingKind& input);
JSON ToJSON(const ChangeAnnotationSupport& input);
JSON ToJSON(const WorkspaceEditClientCapabilities& input);
JSON ToJSON(const TextDocumentPositionParams& input);
JSON ToJSON(const DocumentFilter& input);
JSON ToJSON(const StaticRegistrationOptions& input);
JSON ToJSON(const TextDocumentRegistrationOptions& input);
JSON ToJSON(const MarkupKind& input);
JSON ToJSON(const MarkupContent& input);
JSON ToJSON(const MarkdownClientCapabilities& input);
JSON ToJSON(const WorkDoneProgressBegin& input);
JSON ToJSON(const WorkDoneProgressReport& input);
JSON ToJSON(const WorkDoneProgressEnd& input);
JSON ToJSON(const WorkDoneProgressParams& input);
JSON ToJSON(const PartialResultParams& input);
JSON ToJSON(const TraceValue& input);
JSON ToJSON(const ClientInfo& input);
JSON ToJSON(const SymbolKind& input);
JSON ToJSON(const SymbolKindClientCapabilities& input);
JSON ToJSON(const SymbolTag& input);
JSON ToJSON(const SymbolTagSupportClientCapabilities& input);
JSON ToJSON(const TextDocumentSyncClientCapabilities& input);
JSON ToJSON(const DiagnosticTagSupportClientCapabilities& input);
JSON ToJSON(const PublishDiagnosticsClientCapabilities& input);
JSON ToJSON(const CompletionItemTag& input);
JSON ToJSON(const CompletionItemTagSupportClientCapabilities& input);
JSON ToJSON(const CompletionItemResolveSupportClientCapabilities& input);
JSON ToJSON(const InsertTextMode& input);
JSON ToJSON(const CompletionItemInsertTextModeSupportClientCapabilities& input);
JSON ToJSON(const CompletionItemClientCapabilities& input);
JSON ToJSON(const CompletionItemKind& input);
JSON ToJSON(const CompletionItemKindClientCapabilities& input);
JSON ToJSON(const CompletionClientCapabilities& input);
JSON ToJSON(const HoverClientCapabilities& input);
JSON ToJSON(const ParameterInformationClientCapabilities& input);
JSON ToJSON(const SignatureInformationClientCapabilities& input);
JSON ToJSON(const SignatureHelpClientCapabilities& input);
JSON ToJSON(const DeclarationClientCapabilities& input);
JSON ToJSON(const DefinitionClientCapabilities& input);
JSON ToJSON(const TypeDefinitionClientCapabilities& input);
JSON ToJSON(const ImplementationClientCapabilities& input);
JSON ToJSON(const ReferenceClientCapabilities& input);
JSON ToJSON(const DocumentHighlightClientCapabilities& input);
JSON ToJSON(const DocumentSymbolClientCapabilities& input);
JSON ToJSON(const CodeActionKind& input);
JSON ToJSON(const CodeActionKindClientCapabilities& input);
JSON ToJSON(const CodeActionLiteralClientCapabilities& input);
JSON ToJSON(const CodeActionResolveClientCapabilities& input);
JSON ToJSON(const CodeActionClientCapabilities& input);
JSON ToJSON(const CodeLensClientCapabilities& input);
JSON ToJSON(const DocumentLinkClientCapabilities& input);
JSON ToJSON(const DocumentColorClientCapabilities& input);
JSON ToJSON(const DocumentFormattingClientCapabilities& input);
JSON ToJSON(const DocumentRangeFormattingClientCapabilities& input);
JSON ToJSON(const DocumentOnTypeFormattingClientCapabilities& input);
JSON ToJSON(const PrepareSupportDefaultBehavior& input);
JSON ToJSON(const RenameClientCapabilities& input);
JSON ToJSON(const FoldingRangeClientCapabilities& input);
JSON ToJSON(const SelectionRangeClientCapabilities& input);
JSON ToJSON(const CallHierarchyClientCapabilities& input);
JSON ToJSON(const SemanticTokensRangeClientCapabilities& input);
JSON ToJSON(const SemanticTokensFullClientCapabilities& input);
JSON ToJSON(const TokenFormat& input);
JSON ToJSON(const SemanticTokensClientCapabilities& input);
JSON ToJSON(const LinkedEditingRangeClientCapabilities& input);
JSON ToJSON(const MonikerClientCapabilities& input);
JSON ToJSON(const TextDocumentClientCapabilities& input);
JSON ToJSON(const FileOperationsWorkspaceClientCapabilities& input);
JSON ToJSON(const DidChangeConfigurationClientCapabilities& input);
JSON ToJSON(const DidChangeWatchedFilesClientCapabilities& input);
JSON ToJSON(const WorkspaceSymbolClientCapabilities& input);
JSON ToJSON(const ExecuteCommandClientCapabilities& input);
JSON ToJSON(const CodeLensWorkspaceClientCapabilities& input);
JSON ToJSON(const SemanticTokensWorkspaceClientCapabilities& input);
JSON ToJSON(const WorkspaceSpecificClientCapabilities& input);
JSON ToJSON(const MessageActionItemClientCapabilities& input);
JSON ToJSON(const ShowMessageRequestClientCapabilities& input);
JSON ToJSON(const ShowDocumentClientCapabilities& input);
JSON ToJSON(const WindowSpecificClientCapabilities& input);
JSON ToJSON(const GeneralClientCapabilities& input);
JSON ToJSON(const ClientCapabilities& input);
JSON ToJSON(const FileOperationPatternKind& input);
JSON ToJSON(const FileOperationPatternOptions& input);
JSON ToJSON(const FileOperationPattern& input);
JSON ToJSON(const FileOperationFilter& input);
JSON ToJSON(const FileOperationRegistrationOptions& input);
JSON ToJSON(const FileOperationsWorkspaceServerCapabilities& input);
JSON ToJSON(const WorkspaceFoldersServerCapabilities& input);
JSON ToJSON(const WorkspaceSpecificServerCapabilities& input);
JSON ToJSON(const WatchKind& input);
JSON ToJSON(const FileSystemWatcher& input);
JSON ToJSON(const DidChangeWatchedFilesRegistrationOptions& input);
JSON ToJSON(const WorkDoneProgressOptions& input);
JSON ToJSON(const WorkspaceSymbolOptions& input);
JSON ToJSON(const WorkspaceSymbolRegistrationOptions& input);
JSON ToJSON(const ExecuteCommandOptions& input);
JSON ToJSON(const ExecuteCommandRegistrationOptions& input);
JSON ToJSON(const TextDocumentSyncKind& input);
JSON ToJSON(const SaveOptions& input);
JSON ToJSON(const TextDocumentSyncOptions& input);
JSON ToJSON(const CompletionItemOptions& input);
JSON ToJSON(const CompletionOptions& input);
JSON ToJSON(const CompletionRegistrationOptions& input);
JSON ToJSON(const HoverOptions& input);
JSON ToJSON(const HoverRegistrationOptions& input);
JSON ToJSON(const SignatureHelpOptions& input);
JSON ToJSON(const SignatureHelpRegistrationOptions& input);
JSON ToJSON(const DeclarationOptions& input);
JSON ToJSON(const DeclarationRegistrationOptions& input);
JSON ToJSON(const DefinitionOptions& input);
JSON ToJSON(const DefinitionRegistrationOptions& input);
JSON ToJSON(const TypeDefinitionOptions& input);
JSON ToJSON(const TypeDefinitionRegistrationOptions& input);
JSON ToJSON(const ImplementationOptions& input);
JSON ToJSON(const ImplementationRegistrationOptions& input);
JSON ToJSON(const ReferenceOptions& input);
JSON ToJSON(const ReferenceRegistrationOptions& input);
JSON ToJSON(const DocumentHighlightOptions& input);
JSON ToJSON(const DocumentHighlightRegistrationOptions& input);
JSON ToJSON(const DocumentSymbolOptions& input);
JSON ToJSON(const DocumentSymbolRegistrationOptions& input);
JSON ToJSON(const CodeActionOptions& input);
JSON ToJSON(const CodeActionRegistrationOptions& input);
JSON ToJSON(const CodeLensOptions& input);
JSON ToJSON(const CodeLensRegistrationOptions& input);
JSON ToJSON(const DocumentLinkOptions& input);
JSON ToJSON(const DocumentLinkRegistrationOptions& input);
JSON ToJSON(const DocumentColorOptions& input);
JSON ToJSON(const DocumentColorRegistrationOptions& input);
JSON ToJSON(const DocumentFormattingOptions& input);
JSON ToJSON(const DocumentFormattingRegistrationOptions& input);
JSON ToJSON(const DocumentRangeFormattingOptions& input);
JSON ToJSON(const DocumentRangeFormattingRegistrationOptions& input);
JSON ToJSON(const DocumentOnTypeFormattingOptions& input);
JSON ToJSON(const DocumentOnTypeFormattingRegistrationOptions& input);
JSON ToJSON(const RenameOptions& input);
JSON ToJSON(const RenameRegistrationOptions& input);
JSON ToJSON(const FoldingRangeOptions& input);
JSON ToJSON(const FoldingRangeRegistrationOptions& input);
JSON ToJSON(const SelectionRangeOptions& input);
JSON ToJSON(const SelectionRangeRegistrationOptions& input);
JSON ToJSON(const CallHierarchyOptions& input);
JSON ToJSON(const CallHierarchyRegistrationOptions& input);
JSON ToJSON(const SemanticTokensLegend& input);
JSON ToJSON(const SemanticTokensRangeOptions& input);
JSON ToJSON(const SemanticTokensFullOptions& input);
JSON ToJSON(const SemanticTokensOptions& input);
JSON ToJSON(const SemanticTokensRegistrationOptions& input);
JSON ToJSON(const LinkedEditingRangeOptions& input);
JSON ToJSON(const LinkedEditingRangeRegistrationOptions& input);
JSON ToJSON(const MonikerOptions& input);
JSON ToJSON(const MonikerRegistrationOptions& input);
JSON ToJSON(const ServerCapabilities& input);
JSON ToJSON(const ServerInfo& input);
JSON ToJSON(const WorkspaceFolder& input);
JSON ToJSON(const InitializeParams& input);
JSON ToJSON(const InitializeResult& input);
JSON ToJSON(const InitializeErrorCode& input);
JSON ToJSON(const InitializeErrorData& input);
template<typename T>
JSON ToJSON(const ProgressParams<T>& result);
JSON ToJSON(const InitializedParams& input);
JSON ToJSON(const LogTraceParams& input);
JSON ToJSON(const SetTraceParams& input);
JSON ToJSON(const MessageType& input);
JSON ToJSON(const ShowMessageParams& input);
JSON ToJSON(const MessageActionItem& input);
JSON ToJSON(const ShowMessageRequestParams& input);
JSON ToJSON(const ShowMessageRequestResult& input);
JSON ToJSON(const ShowDocumentParams& input);
JSON ToJSON(const ShowDocumentResult& input);
JSON ToJSON(const LogMessageParams& input);
JSON ToJSON(const WorkDoneProgressCreateParams& input);
JSON ToJSON(const WorkDoneProgressCancelParams& input);
JSON ToJSON(const Registration& input);
JSON ToJSON(const RegistrationParams& input);
JSON ToJSON(const Unregistration& input);
JSON ToJSON(const UnregistrationParams& input);
JSON ToJSON(const WorkspaceFoldersChangeEvent& input);
JSON ToJSON(const DidChangeWorkspaceFoldersParams& input);
JSON ToJSON(const DidChangeConfigurationParams& input);
JSON ToJSON(const ConfigurationItem& input);
JSON ToJSON(const ConfigurationParams& input);
JSON ToJSON(const FileChangeType& input);
JSON ToJSON(const FileEvent& input);
JSON ToJSON(const DidChangeWatchedFilesParams& input);
JSON ToJSON(const WorkspaceSymbolParams& input);
JSON ToJSON(const SymbolInformation& input);
JSON ToJSON(const WorkspaceSymbolResult& input);
JSON ToJSON(const ExecuteCommandParams& input);
JSON ToJSON(const ApplyWorkspaceEditParams& input);
JSON ToJSON(const ApplyWorkspaceEditResult& input);
JSON ToJSON(const FileCreate& input);
JSON ToJSON(const CreateFilesParams& input);
JSON ToJSON(const FileRename& input);
JSON ToJSON(const RenameFilesParams& input);
JSON ToJSON(const FileDelete& input);
JSON ToJSON(const DeleteFilesParams& input);
JSON ToJSON(const DidOpenTextDocumentParams& input);
JSON ToJSON(const TextDocumentChangeRegistrationOptions& input);
JSON ToJSON(const TextDocumentContentChangeEvent& input);
JSON ToJSON(const DidChangeTextDocumentParams& input);
JSON ToJSON(const TextDocumentSaveReason& input);
JSON ToJSON(const WillSaveTextDocumentParams& input);
JSON ToJSON(const TextDocumentSaveRegistrationOptions& input);
JSON ToJSON(const DidSaveTextDocumentParams& input);
JSON ToJSON(const DidCloseTextDocumentParams& input);
JSON ToJSON(const PublishDiagnosticsParams& input);
JSON ToJSON(const CompletionTriggerKind& input);
JSON ToJSON(const CompletionContext& input);
JSON ToJSON(const CompletionParams& input);
JSON ToJSON(const InsertTextFormat& input);
JSON ToJSON(const InsertReplaceEdit& input);
JSON ToJSON(const CompletionItemLabelDetails& input);
JSON ToJSON(const CompletionItem& input);
JSON ToJSON(const CompletionList& input);
JSON ToJSON(const CompletionResult& input);
JSON ToJSON(const HoverParams& input);
JSON ToJSON(const Hover& input);
JSON ToJSON(const SignatureHelpTriggerKind& input);
JSON ToJSON(const ParameterInformation& input);
JSON ToJSON(const SignatureInformation& input);
JSON ToJSON(const SignatureHelp& input);
JSON ToJSON(const SignatureHelpContext& input);
JSON ToJSON(const SignatureHelpParams& input);
JSON ToJSON(const DeclarationParams& input);
JSON ToJSON(const DeclarationResult& input);
JSON ToJSON(const DefinitionParams& input);
JSON ToJSON(const DefinitionResult& input);
JSON ToJSON(const TypeDefinitionParams& input);
JSON ToJSON(const TypeDefinitionResult& input);
JSON ToJSON(const ImplementationParams& input);
JSON ToJSON(const ImplementationResult& input);
JSON ToJSON(const ReferenceContext& input);
JSON ToJSON(const ReferenceParams& input);
JSON ToJSON(const ReferenceResult& input);
JSON ToJSON(const DocumentHighlightParams& input);
JSON ToJSON(const DocumentHighlightKind& input);
JSON ToJSON(const DocumentHighlight& input);
JSON ToJSON(const DocumentSymbolParams& input);
JSON ToJSON(const DocumentSymbol& input);
JSON ToJSON(const CodeActionContext& input);
JSON ToJSON(const CodeActionParams& input);
JSON ToJSON(const CodeAction& input);
JSON ToJSON(const CodeLensParams& input);
JSON ToJSON(const CodeLens& input);
JSON ToJSON(const DocumentLinkParams& input);
JSON ToJSON(const DocumentLink& input);
JSON ToJSON(const DocumentColorParams& input);
JSON ToJSON(const Color& input);
JSON ToJSON(const ColorInformation& input);
JSON ToJSON(const ColorPresentationParams& input);
JSON ToJSON(const ColorPresentation& input);
JSON ToJSON(const FormattingOptions& input);
JSON ToJSON(const DocumentFormattingParams& input);
JSON ToJSON(const DocumentRangeFormattingParams& input);
JSON ToJSON(const DocumentOnTypeFormattingParams& input);
JSON ToJSON(const RenameParams& input);
JSON ToJSON(const PrepareRenameParams& input);
JSON ToJSON(const PrepareRenameResult& input);
JSON ToJSON(const FoldingRangeParams& input);
JSON ToJSON(const FoldingRangeKind& input);
JSON ToJSON(const FoldingRange& input);
JSON ToJSON(const SelectionRangeParams& input);
JSON ToJSON(const SelectionRange& input);
JSON ToJSON(const CallHierarchyPrepareParams& input);
JSON ToJSON(const CallHierarchyItem& input);
JSON ToJSON(const CallHierarchyIncomingCallsParams& input);
JSON ToJSON(const CallHierarchyIncomingCall& input);
JSON ToJSON(const CallHierarchyOutgoingCallsParams& input);
JSON ToJSON(const CallHierarchyOutgoingCall& input);
JSON ToJSON(const SemanticTokensParams& input);
JSON ToJSON(const SemanticTokens& input);
JSON ToJSON(const SemanticTokensDeltaParams& input);
JSON ToJSON(const SemanticTokensEdit& input);
JSON ToJSON(const SemanticTokensDelta& input);
JSON ToJSON(const SemanticTokensRangeParams& input);
JSON ToJSON(const LinkedEditingRangeParams& input);
JSON ToJSON(const LinkedEditingRanges& input);
JSON ToJSON(const MonikerParams& input);
JSON ToJSON(const UniquenessLevel& input);
JSON ToJSON(const MonikerKind& input);
JSON ToJSON(const Moniker& input);

absl::Status FromJSON(const JSON& input, ErrorCodes* result);
absl::Status FromJSON(const JSON& input, ResponseError* result);
absl::Status FromJSON(const JSON& input, ResponseMessage* result);
absl::Status FromJSON(const JSON& input, RegularExpressionsClientCapabilities* result);
absl::Status FromJSON(const JSON& input, Position* result);
absl::Status FromJSON(const JSON& input, Range* result);
absl::Status FromJSON(const JSON& input, Location* result);
absl::Status FromJSON(const JSON& input, LocationLink* result);
absl::Status FromJSON(const JSON& input, DiagnosticSeverity* result);
absl::Status FromJSON(const JSON& input, DiagnosticCode* result);
absl::Status FromJSON(const JSON& input, CodeDescription* result);
absl::Status FromJSON(const JSON& input, DiagnosticTag* result);
absl::Status FromJSON(const JSON& input, DiagnosticRelatedInformation* result);
absl::Status FromJSON(const JSON& input, Diagnostic* result);
absl::Status FromJSON(const JSON& input, Command* result);
absl::Status FromJSON(const JSON& input, TextEdit* result);
absl::Status FromJSON(const JSON& input, ChangeAnnotation* result);
absl::Status FromJSON(const JSON& input, TextDocumentIdentifier* result);
absl::Status FromJSON(const JSON& input, TextDocumentItem* result);
absl::Status FromJSON(const JSON& input, VersionedTextDocumentIdentifier* result);
absl::Status FromJSON(const JSON& input, OptionalVersionedTextDocumentIdentifier* result);
absl::Status FromJSON(const JSON& input, AnnotatedTextEdit* result);
absl::Status FromJSON(const JSON& input, TextDocumentEdit* result);
absl::Status FromJSON(const JSON& input, CreateFileOptions* result);
absl::Status FromJSON(const JSON& input, CreateFile* result);
absl::Status FromJSON(const JSON& input, RenameFileOptions* result);
absl::Status FromJSON(const JSON& input, RenameFile* result);
absl::Status FromJSON(const JSON& input, DeleteFileOptions* result);
absl::Status FromJSON(const JSON& input, DeleteFile* result);
absl::Status FromJSON(const JSON& input, DocumentChangeOperation* result);
absl::Status FromJSON(const JSON& input, DocumentChanges* result);
absl::Status FromJSON(const JSON& input, WorkspaceEdit* result);
absl::Status FromJSON(const JSON& input, ResourceOperationKind* result);
absl::Status FromJSON(const JSON& input, FailureHandlingKind* result);
absl::Status FromJSON(const JSON& input, ChangeAnnotationSupport* result);
absl::Status FromJSON(const JSON& input, WorkspaceEditClientCapabilities* result);
absl::Status FromJSON(const JSON& input, TextDocumentPositionParams* result);
absl::Status FromJSON(const JSON& input, DocumentFilter* result);
absl::Status FromJSON(const JSON& input, StaticRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, TextDocumentRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, MarkupKind* result);
absl::Status FromJSON(const JSON& input, MarkupContent* result);
absl::Status FromJSON(const JSON& input, MarkdownClientCapabilities* result);
absl::Status FromJSON(const JSON& input, WorkDoneProgressParams* result);
absl::Status FromJSON(const JSON& input, PartialResultParams* result);
absl::Status FromJSON(const JSON& input, TraceValue* result);
absl::Status FromJSON(const JSON& input, ClientInfo* result);
absl::Status FromJSON(const JSON& input, SymbolKind* result);
absl::Status FromJSON(const JSON& input, SymbolKindClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SymbolTag* result);
absl::Status FromJSON(const JSON& input, SymbolTagSupportClientCapabilities* result);
absl::Status FromJSON(const JSON& input, TextDocumentSyncClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DiagnosticTagSupportClientCapabilities* result);
absl::Status FromJSON(const JSON& input, PublishDiagnosticsClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CompletionItemTag* result);
absl::Status FromJSON(const JSON& input, CompletionItemTagSupportClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CompletionItemResolveSupportClientCapabilities* result);
absl::Status FromJSON(const JSON& input, InsertTextMode* result);
absl::Status FromJSON(const JSON& input, CompletionItemInsertTextModeSupportClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CompletionItemClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CompletionItemKind* result);
absl::Status FromJSON(const JSON& input, CompletionItemKindClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CompletionClientCapabilities* result);
absl::Status FromJSON(const JSON& input, HoverClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ParameterInformationClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SignatureInformationClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SignatureHelpClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DeclarationClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DefinitionClientCapabilities* result);
absl::Status FromJSON(const JSON& input, TypeDefinitionClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ImplementationClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ReferenceClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentHighlightClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentSymbolClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeActionKind* result);
absl::Status FromJSON(const JSON& input, CodeActionKindClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeActionLiteralClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeActionResolveClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeActionClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeLensClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentLinkClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentColorClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentFormattingClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentRangeFormattingClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingClientCapabilities* result);
absl::Status FromJSON(const JSON& input, PrepareSupportDefaultBehavior* result);
absl::Status FromJSON(const JSON& input, RenameClientCapabilities* result);
absl::Status FromJSON(const JSON& input, FoldingRangeClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SelectionRangeClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CallHierarchyClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SemanticTokensRangeClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SemanticTokensFullClientCapabilities* result);
absl::Status FromJSON(const JSON& input, TokenFormat* result);
absl::Status FromJSON(const JSON& input, SemanticTokensClientCapabilities* result);
absl::Status FromJSON(const JSON& input, LinkedEditingRangeClientCapabilities* result);
absl::Status FromJSON(const JSON& input, MonikerClientCapabilities* result);
absl::Status FromJSON(const JSON& input, TextDocumentClientCapabilities* result);
absl::Status FromJSON(const JSON& input, FileOperationsWorkspaceClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DidChangeConfigurationClientCapabilities* result);
absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesClientCapabilities* result);
absl::Status FromJSON(const JSON& input, WorkspaceSymbolClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ExecuteCommandClientCapabilities* result);
absl::Status FromJSON(const JSON& input, CodeLensWorkspaceClientCapabilities* result);
absl::Status FromJSON(const JSON& input, SemanticTokensWorkspaceClientCapabilities* result);
absl::Status FromJSON(const JSON& input, WorkspaceSpecificClientCapabilities* result);
absl::Status FromJSON(const JSON& input, MessageActionItemClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ShowMessageRequestClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ShowDocumentClientCapabilities* result);
absl::Status FromJSON(const JSON& input, WindowSpecificClientCapabilities* result);
absl::Status FromJSON(const JSON& input, GeneralClientCapabilities* result);
absl::Status FromJSON(const JSON& input, ClientCapabilities* result);
absl::Status FromJSON(const JSON& input, FileOperationPatternKind* result);
absl::Status FromJSON(const JSON& input, FileOperationPatternOptions* result);
absl::Status FromJSON(const JSON& input, FileOperationPattern* result);
absl::Status FromJSON(const JSON& input, FileOperationFilter* result);
absl::Status FromJSON(const JSON& input, FileOperationRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, FileOperationsWorkspaceServerCapabilities* result);
absl::Status FromJSON(const JSON& input, WorkspaceFoldersServerCapabilities* result);
absl::Status FromJSON(const JSON& input, WorkspaceSpecificServerCapabilities* result);
absl::Status FromJSON(const JSON& input, WatchKind* result);
absl::Status FromJSON(const JSON& input, FileSystemWatcher* result);
absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, WorkDoneProgressOptions* result);
absl::Status FromJSON(const JSON& input, WorkspaceSymbolOptions* result);
absl::Status FromJSON(const JSON& input, WorkspaceSymbolRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, ExecuteCommandOptions* result);
absl::Status FromJSON(const JSON& input, ExecuteCommandRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, TextDocumentSyncKind* result);
absl::Status FromJSON(const JSON& input, SaveOptions* result);
absl::Status FromJSON(const JSON& input, TextDocumentSyncOptions* result);
absl::Status FromJSON(const JSON& input, CompletionItemOptions* result);
absl::Status FromJSON(const JSON& input, CompletionOptions* result);
absl::Status FromJSON(const JSON& input, CompletionRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, HoverOptions* result);
absl::Status FromJSON(const JSON& input, HoverRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, SignatureHelpOptions* result);
absl::Status FromJSON(const JSON& input, SignatureHelpRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DeclarationOptions* result);
absl::Status FromJSON(const JSON& input, DeclarationRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DefinitionOptions* result);
absl::Status FromJSON(const JSON& input, DefinitionRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, TypeDefinitionOptions* result);
absl::Status FromJSON(const JSON& input, TypeDefinitionRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, ImplementationOptions* result);
absl::Status FromJSON(const JSON& input, ImplementationRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, ReferenceOptions* result);
absl::Status FromJSON(const JSON& input, ReferenceRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentHighlightOptions* result);
absl::Status FromJSON(const JSON& input, DocumentHighlightRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentSymbolOptions* result);
absl::Status FromJSON(const JSON& input, DocumentSymbolRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, CodeActionOptions* result);
absl::Status FromJSON(const JSON& input, CodeActionRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, CodeLensOptions* result);
absl::Status FromJSON(const JSON& input, CodeLensRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentLinkOptions* result);
absl::Status FromJSON(const JSON& input, DocumentLinkRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentColorOptions* result);
absl::Status FromJSON(const JSON& input, DocumentColorRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentFormattingOptions* result);
absl::Status FromJSON(const JSON& input, DocumentFormattingRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentRangeFormattingOptions* result);
absl::Status FromJSON(const JSON& input, DocumentRangeFormattingRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingOptions* result);
absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, RenameOptions* result);
absl::Status FromJSON(const JSON& input, RenameRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, FoldingRangeOptions* result);
absl::Status FromJSON(const JSON& input, FoldingRangeRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, SelectionRangeOptions* result);
absl::Status FromJSON(const JSON& input, SelectionRangeRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, CallHierarchyOptions* result);
absl::Status FromJSON(const JSON& input, CallHierarchyRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, SemanticTokensLegend* result);
absl::Status FromJSON(const JSON& input, SemanticTokensRangeOptions* result);
absl::Status FromJSON(const JSON& input, SemanticTokensFullOptions* result);
absl::Status FromJSON(const JSON& input, SemanticTokensOptions* result);
absl::Status FromJSON(const JSON& input, SemanticTokensRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, LinkedEditingRangeOptions* result);
absl::Status FromJSON(const JSON& input, LinkedEditingRangeRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, MonikerOptions* result);
absl::Status FromJSON(const JSON& input, MonikerRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, ServerCapabilities* result);
absl::Status FromJSON(const JSON& input, ServerInfo* result);
absl::Status FromJSON(const JSON& input, WorkspaceFolder* result);
absl::Status FromJSON(const JSON& input, InitializeResult* result);
absl::Status FromJSON(const JSON& input, InitializeErrorCode* result);
absl::Status FromJSON(const JSON& input, InitializeErrorData* result);
template<typename T>
absl::Status FromJSON(const JSON& input, ProgressParams<T>* result);
absl::Status FromJSON(const JSON& input, InitializedParams* result);
absl::Status FromJSON(const JSON& input, LogTraceParams* result);
absl::Status FromJSON(const JSON& input, SetTraceParams* result);
absl::Status FromJSON(const JSON& input, MessageType* result);
absl::Status FromJSON(const JSON& input, ShowMessageParams* result);
absl::Status FromJSON(const JSON& input, MessageActionItem* result);
absl::Status FromJSON(const JSON& input, ShowMessageRequestParams* result);
absl::Status FromJSON(const JSON& input, ShowDocumentParams* result);
absl::Status FromJSON(const JSON& input, ShowDocumentResult* result);
absl::Status FromJSON(const JSON& input, LogMessageParams* result);
absl::Status FromJSON(const JSON& input, WorkDoneProgressCreateParams* result);
absl::Status FromJSON(const JSON& input, WorkDoneProgressCancelParams* result);
absl::Status FromJSON(const JSON& input, Registration* result);
absl::Status FromJSON(const JSON& input, RegistrationParams* result);
absl::Status FromJSON(const JSON& input, Unregistration* result);
absl::Status FromJSON(const JSON& input, UnregistrationParams* result);
absl::Status FromJSON(const JSON& input, WorkspaceFoldersChangeEvent* result);
absl::Status FromJSON(const JSON& input, DidChangeWorkspaceFoldersParams* result);
absl::Status FromJSON(const JSON& input, DidChangeConfigurationParams* result);
absl::Status FromJSON(const JSON& input, ConfigurationItem* result);
absl::Status FromJSON(const JSON& input, ConfigurationParams* result);
absl::Status FromJSON(const JSON& input, FileChangeType* result);
absl::Status FromJSON(const JSON& input, FileEvent* result);
absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesParams* result);
absl::Status FromJSON(const JSON& input, WorkspaceSymbolParams* result);
absl::Status FromJSON(const JSON& input, SymbolInformation* result);
absl::Status FromJSON(const JSON& input, ExecuteCommandParams* result);
absl::Status FromJSON(const JSON& input, ApplyWorkspaceEditParams* result);
absl::Status FromJSON(const JSON& input, ApplyWorkspaceEditResult* result);
absl::Status FromJSON(const JSON& input, FileCreate* result);
absl::Status FromJSON(const JSON& input, CreateFilesParams* result);
absl::Status FromJSON(const JSON& input, FileRename* result);
absl::Status FromJSON(const JSON& input, RenameFilesParams* result);
absl::Status FromJSON(const JSON& input, FileDelete* result);
absl::Status FromJSON(const JSON& input, DeleteFilesParams* result);
absl::Status FromJSON(const JSON& input, DidOpenTextDocumentParams* result);
absl::Status FromJSON(const JSON& input, TextDocumentChangeRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, TextDocumentContentChangeEvent* result);
absl::Status FromJSON(const JSON& input, DidChangeTextDocumentParams* result);
absl::Status FromJSON(const JSON& input, TextDocumentSaveReason* result);
absl::Status FromJSON(const JSON& input, WillSaveTextDocumentParams* result);
absl::Status FromJSON(const JSON& input, TextDocumentSaveRegistrationOptions* result);
absl::Status FromJSON(const JSON& input, DidSaveTextDocumentParams* result);
absl::Status FromJSON(const JSON& input, DidCloseTextDocumentParams* result);
absl::Status FromJSON(const JSON& input, PublishDiagnosticsParams* result);
absl::Status FromJSON(const JSON& input, CompletionTriggerKind* result);
absl::Status FromJSON(const JSON& input, CompletionContext* result);
absl::Status FromJSON(const JSON& input, CompletionParams* result);
absl::Status FromJSON(const JSON& input, InsertTextFormat* result);
absl::Status FromJSON(const JSON& input, InsertReplaceEdit* result);
absl::Status FromJSON(const JSON& input, CompletionItemLabelDetails* result);
absl::Status FromJSON(const JSON& input, CompletionItem* result);
absl::Status FromJSON(const JSON& input, CompletionList* result);
absl::Status FromJSON(const JSON& input, CompletionResult* result);
absl::Status FromJSON(const JSON& input, HoverParams* result);
absl::Status FromJSON(const JSON& input, Hover* result);
absl::Status FromJSON(const JSON& input, SignatureHelpTriggerKind* result);
absl::Status FromJSON(const JSON& input, ParameterInformation* result);
absl::Status FromJSON(const JSON& input, SignatureInformation* result);
absl::Status FromJSON(const JSON& input, SignatureHelp* result);
absl::Status FromJSON(const JSON& input, SignatureHelpContext* result);
absl::Status FromJSON(const JSON& input, SignatureHelpParams* result);
absl::Status FromJSON(const JSON& input, DeclarationParams* result);
absl::Status FromJSON(const JSON& input, DeclarationResult* result);
absl::Status FromJSON(const JSON& input, DefinitionParams* result);
absl::Status FromJSON(const JSON& input, DefinitionResult* result);
absl::Status FromJSON(const JSON& input, TypeDefinitionParams* result);
absl::Status FromJSON(const JSON& input, TypeDefinitionResult* result);
absl::Status FromJSON(const JSON& input, ImplementationParams* result);
absl::Status FromJSON(const JSON& input, ImplementationResult* result);
absl::Status FromJSON(const JSON& input, ReferenceContext* result);
absl::Status FromJSON(const JSON& input, ReferenceParams* result);
absl::Status FromJSON(const JSON& input, ReferenceResult* result);
absl::Status FromJSON(const JSON& input, DocumentHighlightParams* result);
absl::Status FromJSON(const JSON& input, DocumentHighlightKind* result);
absl::Status FromJSON(const JSON& input, DocumentHighlight* result);
absl::Status FromJSON(const JSON& input, DocumentSymbolParams* result);
absl::Status FromJSON(const JSON& input, DocumentSymbol* result);
absl::Status FromJSON(const JSON& input, CodeActionContext* result);
absl::Status FromJSON(const JSON& input, CodeActionParams* result);
absl::Status FromJSON(const JSON& input, CodeLensParams* result);
absl::Status FromJSON(const JSON& input, CodeLens* result);
absl::Status FromJSON(const JSON& input, DocumentLinkParams* result);
absl::Status FromJSON(const JSON& input, DocumentLink* result);
absl::Status FromJSON(const JSON& input, DocumentColorParams* result);
absl::Status FromJSON(const JSON& input, Color* result);
absl::Status FromJSON(const JSON& input, ColorInformation* result);
absl::Status FromJSON(const JSON& input, ColorPresentationParams* result);
absl::Status FromJSON(const JSON& input, ColorPresentation* result);
absl::Status FromJSON(const JSON& input, FormattingOptions* result);
absl::Status FromJSON(const JSON& input, DocumentFormattingParams* result);
absl::Status FromJSON(const JSON& input, DocumentRangeFormattingParams* result);
absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingParams* result);
absl::Status FromJSON(const JSON& input, RenameParams* result);
absl::Status FromJSON(const JSON& input, PrepareRenameParams* result);
absl::Status FromJSON(const JSON& input, FoldingRangeParams* result);
absl::Status FromJSON(const JSON& input, FoldingRangeKind* result);
absl::Status FromJSON(const JSON& input, FoldingRange* result);
absl::Status FromJSON(const JSON& input, SelectionRangeParams* result);
absl::Status FromJSON(const JSON& input, CallHierarchyPrepareParams* result);
absl::Status FromJSON(const JSON& input, CallHierarchyItem* result);
absl::Status FromJSON(const JSON& input, CallHierarchyIncomingCallsParams* result);
absl::Status FromJSON(const JSON& input, CallHierarchyIncomingCall* result);
absl::Status FromJSON(const JSON& input, CallHierarchyOutgoingCallsParams* result);
absl::Status FromJSON(const JSON& input, CallHierarchyOutgoingCall* result);
absl::Status FromJSON(const JSON& input, SemanticTokensParams* result);
absl::Status FromJSON(const JSON& input, SemanticTokens* result);
absl::Status FromJSON(const JSON& input, SemanticTokensDeltaParams* result);
absl::Status FromJSON(const JSON& input, SemanticTokensEdit* result);
absl::Status FromJSON(const JSON& input, SemanticTokensDelta* result);
absl::Status FromJSON(const JSON& input, SemanticTokensRangeParams* result);
absl::Status FromJSON(const JSON& input, LinkedEditingRangeParams* result);
absl::Status FromJSON(const JSON& input, LinkedEditingRanges* result);
absl::Status FromJSON(const JSON& input, MonikerParams* result);
absl::Status FromJSON(const JSON& input, UniquenessLevel* result);
absl::Status FromJSON(const JSON& input, MonikerKind* result);
absl::Status FromJSON(const JSON& input, Moniker* result);

////////////////////////////////////////////////////////////////////////////////

}  // namespace lsp

}  // namespace rdss

#endif  // RDSS_LSP_H_
