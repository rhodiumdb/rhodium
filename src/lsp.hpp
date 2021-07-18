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

#include <cstdint>
#include <string>
#include <vector>

#include <absl/container/btree_map.h>
#include <absl/types/optional.h>
#include <absl/types/variant.h>

namespace rdss {

namespace lsp {

using JSON = void*; // FIXME

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
    WorkDoneProgressParams underlying;
    // nullopt == null here
    absl::optional<int32_t> process_id;
    absl::optional<ClientInfo> client_info;
    absl::optional<std::string> locale;
    absl::optional<absl::optional<std::string>> root_path;
    absl::optional<DocumentUri> root_uri;
    absl::optional<JSON> intiialization_options;
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

enum class FileChangeType : uint32_t {
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

struct WorkspaceSymbolPartialResult {
    std::vector<SymbolInformation> symbols;
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

    Response<absl::variant<WorkspaceSymbolResult, WorkspaceSymbolPartialResult>>
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
