#include "lsp.hpp"

#include <absl/strings/str_format.h>

namespace rdss {

namespace lsp {

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

JSON ToJSON(const ErrorCodes& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const ResponseError& input) {
    JSON result(Json::objectValue);
    Set(&result, "code", input.code);
    Set(&result, "message", input.message);
    OptionallySet(&result, "data", input.data);
    return result;
}

JSON ToJSON(const ResponseMessage& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "id", input.id);
    OptionallySet(&result, "result", input.result);
    OptionallySet(&result, "error", input.error);
    return result;
}

JSON ToJSON(const RegularExpressionsClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "engine", input.engine);
    OptionallySet(&result, "version", input.version);
    return result;
}

JSON ToJSON(const Position& input) {
    JSON result(Json::objectValue);
    Set(&result, "line", input.line);
    Set(&result, "character", input.character);
    return result;
}

JSON ToJSON(const Range& input) {
    JSON result(Json::objectValue);
    Set(&result, "start", input.start);
    Set(&result, "end", input.end);
    return result;
}

JSON ToJSON(const Location& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "range", input.range);
    return result;
}

JSON ToJSON(const LocationLink& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "originSelectionRange", input.origin_selection_range);
    Set(&result, "targetUri", input.target_uri);
    Set(&result, "targetRange", input.target_range);
    Set(&result, "targetSelectionRange", input.target_selection_range);
    return result;
}

JSON ToJSON(const DiagnosticSeverity& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const DiagnosticCode& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.code));
    return result;
}

JSON ToJSON(const CodeDescription& input) {
    JSON result(Json::objectValue);
    Set(&result, "href", input.href);
    return result;
}

JSON ToJSON(const DiagnosticTag& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const DiagnosticRelatedInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "location", input.location);
    Set(&result, "message", input.message);
    return result;
}

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

JSON ToJSON(const Command& input) {
    JSON result(Json::objectValue);
    Set(&result, "title", input.title);
    Set(&result, "command", input.command);
    OptionallySet(&result, "arguments", input.arguments);
    return result;
}

JSON ToJSON(const TextEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    Set(&result, "newText", input.new_text);
    return result;
}

JSON ToJSON(const ChangeAnnotation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "needsConfirmation", input.needs_confirmation);
    OptionallySet(&result, "description", input.description);
    return result;
}

JSON ToJSON(const TextDocumentIdentifier& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

JSON ToJSON(const TextDocumentItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "languageId", input.language_id);
    Set(&result, "version", input.version);
    Set(&result, "text", input.text);
    return result;
}

JSON ToJSON(const VersionedTextDocumentIdentifier& input) {
    JSON result = ToJSON(input.underlying);
    Set(&result, "version", input.version);
    return result;
}

JSON ToJSON(const OptionalVersionedTextDocumentIdentifier& input) {
    JSON result = ToJSON(input.underlying);
    OptionallySet(&result, "version", input.version);
    return result;
}

JSON ToJSON(const AnnotatedTextEdit& input) {
    JSON result = ToJSON(input.underlying);
    Set(&result, "annotationId", input.annotation_id);
    return result;
}

JSON ToJSON(const TextDocumentEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "edits", input.edits);
    return result;
}

JSON ToJSON(const CreateFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "overwrite", input.overwrite);
    OptionallySet(&result, "ignoreIfExists", input.ignore_if_exists);
    return result;
}

JSON ToJSON(const CreateFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "create";
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

JSON ToJSON(const RenameFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "overwrite", input.overwrite);
    OptionallySet(&result, "ignoreIfExists", input.ignore_if_exists);
    return result;
}

JSON ToJSON(const RenameFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "rename";
    Set(&result, "oldUri", input.old_uri);
    Set(&result, "newUri", input.new_uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

JSON ToJSON(const DeleteFileOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "recursive", input.recursive);
    OptionallySet(&result, "ignoreIfNotExists", input.ignore_if_not_exists);
    return result;
}

JSON ToJSON(const DeleteFile& input) {
    JSON result(Json::objectValue);
    result["kind"] = "delete";
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "options", input.options);
    OptionallySet(&result, "annotationId", input.annotation_id);
    return result;
}

JSON ToJSON(const DocumentChangeOperation& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.operation));
    return result;
}

JSON ToJSON(const DocumentChanges& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.changes));
    return result;
}

JSON ToJSON(const WorkspaceEdit& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "changes", input.changes);
    OptionallySet(&result, "documentChanges", input.document_changes);
    OptionallySet(&result, "changeAnnotations", input.change_annotations);
    return result;
}

JSON ToJSON(const ResourceOperationKind& input) {
    switch (input) {
    case ResourceOperationKind::Create: return "create";
    case ResourceOperationKind::Rename: return "rename";
    case ResourceOperationKind::Delete: return "delete";
    }
    RDSS_CHECK(false);
    return JSON();
}

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

JSON ToJSON(const ChangeAnnotationSupport& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "groupsOnLabel", input.groups_on_label);
    return result;
}

JSON ToJSON(const WorkspaceEditClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "documentChanges", input.document_changes);
    OptionallySet(&result, "resourceOperations", input.resource_operations);
    OptionallySet(&result, "failureHandling", input.failure_handling);
    OptionallySet(&result, "normalizesLineEndings", input.normalizes_line_endings);
    OptionallySet(&result, "changeAnnotationSupport", input.change_annotation_support);
    return result;
}

JSON ToJSON(const TextDocumentPositionParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "position", input.position);
    return result;
}

JSON ToJSON(const DocumentFilter& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "language", input.language);
    OptionallySet(&result, "scheme", input.scheme);
    OptionallySet(&result, "pattern", input.pattern);
    return result;
}

JSON ToJSON(const StaticRegistrationOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "id", input.id);
    return result;
}

JSON ToJSON(const TextDocumentRegistrationOptions& input) {
    JSON result(Json::objectValue);
    if (input.document_selector.has_value()) {
        result["documentSelector"] = ToJSON(input.document_selector.value());
    } else {
        result["documentSelector"] = JSON();
    }
    return result;
}

JSON ToJSON(const MarkupKind& input) {
    switch (input) {
    case MarkupKind::PlainText: return "plaintext";
    case MarkupKind::Markdown:  return "markdown";
    }
    RDSS_CHECK(false);
    return JSON();
}

JSON ToJSON(const MarkupContent& input) {
    JSON result(Json::objectValue);
    Set(&result, "kind", input.kind);
    Set(&result, "value", input.value);
    return result;
}

JSON ToJSON(const MarkdownClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "parser", input.parser);
    OptionallySet(&result, "version", input.version);
    return result;
}

JSON ToJSON(const WorkDoneProgressBegin& input) {
    JSON result(Json::objectValue);
    result["kind"] = "begin";
    Set(&result, "title", input.title);
    OptionallySet(&result, "cancellable", input.cancellable);
    OptionallySet(&result, "message", input.message);
    OptionallySet(&result, "percentage", input.percentage);
    return result;
}

JSON ToJSON(const WorkDoneProgressReport& input) {
    JSON result(Json::objectValue);
    result["kind"] = "report";
    OptionallySet(&result, "cancellable", input.cancellable);
    OptionallySet(&result, "message", input.message);
    OptionallySet(&result, "percentage", input.percentage);
    return result;
}

JSON ToJSON(const WorkDoneProgressEnd& input) {
    JSON result(Json::objectValue);
    result["kind"] = "end";
    OptionallySet(&result, "message", input.message);
    return result;
}

JSON ToJSON(const WorkDoneProgressParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneToken", input.work_done_token);
    return result;
}

JSON ToJSON(const PartialResultParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "partialResultToken", input.partial_result_token);
    return result;
}

JSON ToJSON(const TraceValue& input) {
    switch (input) {
    case TraceValue::Off:      return "off";
    case TraceValue::Messages: return "messages";
    case TraceValue::Verbose:  return "verbose";
    }
    RDSS_CHECK(false);
    return JSON();
}

JSON ToJSON(const ClientInfo& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    OptionallySet(&result, "version", input.version);
    return result;
}

JSON ToJSON(const SymbolKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const SymbolKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const SymbolTag& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const SymbolTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const TextDocumentSyncClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "willSave", input.will_save);
    OptionallySet(&result, "willSaveWaitUntil", input.will_save_wait_until);
    OptionallySet(&result, "didSave", input.did_save);
    return result;
}

JSON ToJSON(const DiagnosticTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const PublishDiagnosticsClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "relatedInformation", input.related_information);
    OptionallySet(&result, "tagSupport", input.tag_support);
    OptionallySet(&result, "versionSupport", input.version_support);
    OptionallySet(&result, "codeDescriptionSupport", input.code_description_support);
    OptionallySet(&result, "dataSupport", input.data_support);
    return result;
}

JSON ToJSON(const CompletionItemTag& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const CompletionItemTagSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const CompletionItemResolveSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "properties", input.properties);
    return result;
}

JSON ToJSON(const InsertTextMode& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const CompletionItemInsertTextModeSupportClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

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

JSON ToJSON(const CompletionItemKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const CompletionItemKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const CompletionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "completionItem", input.completion_item);
    OptionallySet(&result, "completionItemKind", input.completion_item_kind);
    OptionallySet(&result, "contextSupport", input.context_support);
    OptionallySet(&result, "insertTextMode", input.insert_text_mode);
    return result;
}

JSON ToJSON(const HoverClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "contentFormat", input.content_format);
    return result;
}

JSON ToJSON(const ParameterInformationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "labelOffsetSupport", input.label_offset_support);
    return result;
}

JSON ToJSON(const SignatureInformationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "documentationFormat", input.documentation_format);
    OptionallySet(&result, "parameterInformation", input.parameter_information);
    OptionallySet(&result, "activeParameterSupport", input.active_parameter_support);
    return result;
}

JSON ToJSON(const SignatureHelpClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "signatureInformation", input.signature_information);
    OptionallySet(&result, "contextSupport", input.context_support);
    return result;
}

JSON ToJSON(const DeclarationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

JSON ToJSON(const DefinitionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

JSON ToJSON(const TypeDefinitionClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

JSON ToJSON(const ImplementationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "linkSupport", input.link_support);
    return result;
}

JSON ToJSON(const ReferenceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentHighlightClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentSymbolClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "symbolKind", input.symbol_kind);
    OptionallySet(&result, "hierarchicalDocumentSymbolSupport", input.hierarchical_document_symbol_support);
    OptionallySet(&result, "tagSupport", input.tag_support);
    OptionallySet(&result, "labelSupport", input.label_support);
    return result;
}

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

JSON ToJSON(const CodeActionKindClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "valueSet", input.value_set);
    return result;
}

JSON ToJSON(const CodeActionLiteralClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "codeActionKind", input.code_action_kind);
    return result;
}

JSON ToJSON(const CodeActionResolveClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "properties", input.properties);
    return result;
}

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

JSON ToJSON(const CodeLensClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentLinkClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "tooltipSupport", input.tooltip_support);
    return result;
}

JSON ToJSON(const DocumentColorClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentRangeFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DocumentOnTypeFormattingClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const PrepareSupportDefaultBehavior& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const RenameClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "prepareSupport", input.prepare_support);
    OptionallySet(&result, "prepareSupportDefaultBehavior", input.prepare_support_default_behavior);
    OptionallySet(&result, "honorsChangeAnnotations", input.honors_change_annotations);
    return result;
}

JSON ToJSON(const FoldingRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "rangeLimit", input.range_limit);
    OptionallySet(&result, "lineFoldingOnly", input.line_folding_only);
    return result;
}

JSON ToJSON(const SelectionRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const CallHierarchyClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const SemanticTokensRangeClientCapabilities& input) {
    return JSON(Json::objectValue);
}

JSON ToJSON(const SemanticTokensFullClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "delta", input.delta);
    return result;
}

JSON ToJSON(const TokenFormat& input) {
    switch (input) {
    case TokenFormat::Relative: return "relative";
    }
    RDSS_CHECK(false);
    return JSON();
}

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

JSON ToJSON(const LinkedEditingRangeClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const MonikerClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

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

JSON ToJSON(const DidChangeConfigurationClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const DidChangeWatchedFilesClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const WorkspaceSymbolClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    OptionallySet(&result, "symbolKind", input.symbol_kind);
    OptionallySet(&result, "tagSupport", input.tag_support);
    return result;
}

JSON ToJSON(const ExecuteCommandClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "dynamicRegistration", input.dynamic_registration);
    return result;
}

JSON ToJSON(const CodeLensWorkspaceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "refreshSupport", input.refresh_support);
    return result;
}

JSON ToJSON(const SemanticTokensWorkspaceClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "refreshSupport", input.refresh_support);
    return result;
}

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

JSON ToJSON(const MessageActionItemClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "additionalPropertiesSupport", input.additional_properties_support);
    return result;
}

JSON ToJSON(const ShowMessageRequestClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "messageActionItem", input.message_action_item);
    return result;
}

JSON ToJSON(const ShowDocumentClientCapabilities& input) {
    JSON result(Json::objectValue);
    Set(&result, "support", input.support);
    return result;
}

JSON ToJSON(const WindowSpecificClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneProgress", input.work_done_progress);
    OptionallySet(&result, "showMessage", input.show_message);
    OptionallySet(&result, "showDocument", input.show_document);
    return result;
}

JSON ToJSON(const GeneralClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "regularExpressions", input.regular_expressions);
    OptionallySet(&result, "markdown", input.markdown);
    return result;
}

JSON ToJSON(const ClientCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workspace", input.workspace);
    OptionallySet(&result, "textDocument", input.text_document);
    OptionallySet(&result, "window", input.window);
    OptionallySet(&result, "general", input.general);
    OptionallySet(&result, "experimental", input.experimental);
    return result;
}

JSON ToJSON(const FileOperationPatternKind& input) {
    switch (input) {
    case FileOperationPatternKind::File:   return "file";
    case FileOperationPatternKind::Folder: return "folder";
    }
    RDSS_CHECK(false);
    return JSON();
}

JSON ToJSON(const FileOperationPatternOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "ignoreCase", input.ignore_case);
    return result;
}

JSON ToJSON(const FileOperationPattern& input) {
    JSON result(Json::objectValue);
    Set(&result, "glob", input.glob);
    OptionallySet(&result, "matches", input.matches);
    OptionallySet(&result, "options", input.options);
    return result;
}

JSON ToJSON(const FileOperationFilter& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "scheme", input.scheme);
    Set(&result, "pattern", input.pattern);
    return result;
}

JSON ToJSON(const FileOperationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "filters", input.filters);
    return result;
}

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

JSON ToJSON(const WorkspaceFoldersServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "supported", input.supported);
    OptionallySet(&result, "changeNotifications", input.change_notifications);
    return result;
}

JSON ToJSON(const WorkspaceSpecificServerCapabilities& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workspaceFolders", input.workspace_folders);
    OptionallySet(&result, "fileOperations", input.file_operations);
    return result;
}

JSON ToJSON(const WatchKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const FileSystemWatcher& input) {
    JSON result(Json::objectValue);
    Set(&result, "globPattern", input.glob_pattern);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

JSON ToJSON(const DidChangeWatchedFilesRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "watchers", input.watchers);
    return result;
}

JSON ToJSON(const WorkDoneProgressOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "workDoneProgress", input.work_done_progress);
    return result;
}

JSON ToJSON(const WorkspaceSymbolOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    return result;
}

JSON ToJSON(const WorkspaceSymbolRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_workspace_symbol_options));
    return result;
}

JSON ToJSON(const ExecuteCommandOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Set(&result, "commands", input.commands);
    return result;
}

JSON ToJSON(const ExecuteCommandRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_execute_command_options));
    return result;
}

JSON ToJSON(const TextDocumentSyncKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const SaveOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "includeText", input.include_text);
    return result;
}

JSON ToJSON(const TextDocumentSyncOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "openClose", input.open_close);
    OptionallySet(&result, "change", input.change);
    OptionallySet(&result, "willSave", input.will_save);
    OptionallySet(&result, "willSaveWaitUntil", input.will_save_wait_until);
    OptionallySet(&result, "save", input.save);
    return result;
}

JSON ToJSON(const CompletionItemOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "labelDetailsSupport", input.label_details_support);
    return result;
}

JSON ToJSON(const CompletionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "triggerCharacters", input.trigger_characters);
    OptionallySet(&result, "allCommitCharacters", input.all_commit_characters);
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    OptionallySet(&result, "completionItem", input.completion_item);
    return result;
}

JSON ToJSON(const CompletionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_co));
    return result;
}

JSON ToJSON(const HoverOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const HoverRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ho));
    return result;
}

JSON ToJSON(const SignatureHelpOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "triggerCharacters", input.trigger_characters);
    OptionallySet(&result, "retriggerCharacters", input.retrigger_characters);
    return result;
}

JSON ToJSON(const SignatureHelpRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sho));
    return result;
}

JSON ToJSON(const DeclarationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DeclarationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_do));
    return result;
}

JSON ToJSON(const DefinitionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DefinitionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_do));
    return result;
}

JSON ToJSON(const TypeDefinitionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const TypeDefinitionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_tdo));
    return result;
}

JSON ToJSON(const ImplementationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const ImplementationRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_io));
    return result;
}

JSON ToJSON(const ReferenceOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const ReferenceRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ro));
    return result;
}

JSON ToJSON(const DocumentHighlightOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DocumentHighlightRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dho));
    return result;
}

JSON ToJSON(const DocumentSymbolOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "label", input.label);
    return result;
}

JSON ToJSON(const DocumentSymbolRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dso));
    return result;
}

JSON ToJSON(const CodeActionOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "codeActionKinds", input.code_action_kinds);
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

JSON ToJSON(const CodeActionRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_cao));
    return result;
}

JSON ToJSON(const CodeLensOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

JSON ToJSON(const CodeLensRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_clo));
    return result;
}

JSON ToJSON(const DocumentLinkOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "resolveProvider", input.resolve_provider);
    return result;
}

JSON ToJSON(const DocumentLinkRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dlo));
    return result;
}

JSON ToJSON(const DocumentColorOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DocumentColorRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_dco));
    return result;
}

JSON ToJSON(const DocumentFormattingOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DocumentFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dfo));
    return result;
}

JSON ToJSON(const DocumentRangeFormattingOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const DocumentRangeFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_drfo));
    return result;
}

JSON ToJSON(const DocumentOnTypeFormattingOptions& input) {
    JSON result(Json::objectValue);
    Set(&result, "firstTriggerCharacter", input.first_trigger_character);
    OptionallySet(&result, "moreTriggerCharacter", input.more_trigger_character);
    return result;
}

JSON ToJSON(const DocumentOnTypeFormattingRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_dotfo));
    return result;
}

JSON ToJSON(const RenameOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    OptionallySet(&result, "prepareProvider", input.prepare_provider);
    return result;
}

JSON ToJSON(const RenameRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_ro));
    return result;
}

JSON ToJSON(const FoldingRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const FoldingRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_fro));
    return result;
}

JSON ToJSON(const SelectionRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const SelectionRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_selro));
    return result;
}

JSON ToJSON(const CallHierarchyOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const CallHierarchyRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_cho));
    return result;
}

JSON ToJSON(const SemanticTokensLegend& input) {
    JSON result(Json::objectValue);
    Set(&result, "tokenTypes", input.token_types);
    Set(&result, "tokenModifiers", input.token_modifiers);
    return result;
}

JSON ToJSON(const SemanticTokensRangeOptions& input) {
    JSON result(Json::objectValue);
    return result;
}

JSON ToJSON(const SemanticTokensFullOptions& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "delta", input.delta);
    return result;
}

JSON ToJSON(const SemanticTokensOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    Set(&result, "legend", input.legend);
    OptionallySet(&result, "range", input.range);
    OptionallySet(&result, "full", input.full);
    return result;
}

JSON ToJSON(const SemanticTokensRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_sto));
    return result;
}

JSON ToJSON(const LinkedEditingRangeOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const LinkedEditingRangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_sro));
    Merge(&result, ToJSON(input.underlying_lero));
    return result;
}

JSON ToJSON(const MonikerOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpo));
    return result;
}

JSON ToJSON(const MonikerRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Merge(&result, ToJSON(input.underlying_mo));
    return result;
}

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

JSON ToJSON(const ServerInfo& input) {
    JSON result(Json::objectValue);
    Set(&result, "name", input.name);
    OptionallySet(&result, "version", input.version);
    return result;
}

JSON ToJSON(const WorkspaceFolder& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "name", input.name);
    return result;
}

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

JSON ToJSON(const InitializeResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "capabilities", input.capabilities);
    OptionallySet(&result, "serverInfo", input.server_info);
    return result;
}

JSON ToJSON(const InitializeErrorCode& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const InitializeErrorData& input) {
    JSON result(Json::objectValue);
    Set(&result, "retry", input.retry);
    return result;
}

template<typename T>
JSON ToJSON(const ProgressParams<T>& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    Set(&result, "value", input.value);
    return result;
}

template JSON ToJSON(const ProgressParams<JSON>& input);
template JSON ToJSON(const ProgressParams<WorkDoneProgressBegin>& input);
template JSON ToJSON(const ProgressParams<WorkDoneProgressReport>& input);
template JSON ToJSON(const ProgressParams<WorkDoneProgressEnd>& input);

JSON ToJSON(const InitializedParams& input) {
    JSON result(Json::objectValue);
    return result;
}

JSON ToJSON(const LogTraceParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "message", input.message);
    OptionallySet(&result, "verbose", input.verbose);
    return result;
}

JSON ToJSON(const SetTraceParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "value", input.value);
    return result;
}

JSON ToJSON(const MessageType& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const ShowMessageParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    return result;
}

JSON ToJSON(const MessageActionItem& input) {
    JSON result(Json::objectValue);
    Set(&result, "title", input.title);
    return result;
}

JSON ToJSON(const ShowMessageRequestParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    OptionallySet(&result, "actions", input.actions);
    return result;
}

JSON ToJSON(const ShowMessageRequestResult& input) {
    if (input.action.has_value()) {
        return ToJSON(input.action);
    } else {
        return JSON();
    }
}

JSON ToJSON(const ShowDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "external", input.external);
    OptionallySet(&result, "takeFocus", input.take_focus);
    OptionallySet(&result, "selection", input.selection);
    return result;
}

JSON ToJSON(const ShowDocumentResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "success", input.success);
    return result;
}

JSON ToJSON(const LogMessageParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "type", input.type);
    Set(&result, "message", input.message);
    return result;
}

JSON ToJSON(const WorkDoneProgressCreateParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    return result;
}

JSON ToJSON(const WorkDoneProgressCancelParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "token", input.token);
    return result;
}

JSON ToJSON(const Registration& input) {
    JSON result(Json::objectValue);
    Set(&result, "id", input.id);
    Set(&result, "method", input.method);
    OptionallySet(&result, "registerOptions", input.register_options);
    return result;
}

JSON ToJSON(const RegistrationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "registrations", input.registrations);
    return result;
}

JSON ToJSON(const Unregistration& input) {
    JSON result(Json::objectValue);
    Set(&result, "id", input.id);
    Set(&result, "method", input.method);
    return result;
}

JSON ToJSON(const UnregistrationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "unregisterations", input.unregisterations);
    return result;
}

JSON ToJSON(const WorkspaceFoldersChangeEvent& input) {
    JSON result(Json::objectValue);
    Set(&result, "added", input.added);
    Set(&result, "removed", input.removed);
    return result;
}

JSON ToJSON(const DidChangeWorkspaceFoldersParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "event", input.event);
    return result;
}

JSON ToJSON(const DidChangeConfigurationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "settings", input.settings);
    return result;
}

JSON ToJSON(const ConfigurationItem& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "scopeUri", input.scope_uri);
    OptionallySet(&result, "section", input.section);
    return result;
}

JSON ToJSON(const ConfigurationParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "items", input.items);
    return result;
}

JSON ToJSON(const FileChangeType& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const FileEvent& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    Set(&result, "type", input.type);
    return result;
}

JSON ToJSON(const DidChangeWatchedFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "changes", input.changes);
    return result;
}

JSON ToJSON(const WorkspaceSymbolParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Merge(&result, ToJSON(input.underlying_partial_result));
    Set(&result, "query", input.query);
    return result;
}

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

JSON ToJSON(const WorkspaceSymbolResult& input) {
    JSON result(Json::objectValue);
    if (input.symbols.has_value()) {
        return ToJSON(input.symbols.value());
    } else {
        return JSON();
    }
}

JSON ToJSON(const ExecuteCommandParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_work_done_progress));
    Set(&result, "command", input.command);
    OptionallySet(&result, "arguments", input.arguments);
    return result;
}

JSON ToJSON(const ApplyWorkspaceEditParams& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "label", input.label);
    Set(&result, "edit", input.edit);
    return result;
}

JSON ToJSON(const ApplyWorkspaceEditResult& input) {
    JSON result(Json::objectValue);
    Set(&result, "applied", input.applied);
    OptionallySet(&result, "failureReason", input.failure_reason);
    OptionallySet(&result, "failedChange", input.failed_change);
    return result;
}

JSON ToJSON(const FileCreate& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

JSON ToJSON(const CreateFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

JSON ToJSON(const FileRename& input) {
    JSON result(Json::objectValue);
    Set(&result, "oldUri", input.old_uri);
    Set(&result, "newUri", input.new_uri);
    return result;
}

JSON ToJSON(const RenameFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

JSON ToJSON(const FileDelete& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    return result;
}

JSON ToJSON(const DeleteFilesParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "files", input.files);
    return result;
}

JSON ToJSON(const DidOpenTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const TextDocumentChangeRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    Set(&result, "syncKind", input.sync_kind);
    return result;
}

JSON ToJSON(const TextDocumentContentChangeEvent& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "range", input.range);
    OptionallySet(&result, "rangeLength", input.range_length);
    Set(&result, "text", input.text);
    return result;
}

JSON ToJSON(const DidChangeTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "contentChanges", input.content_changes);
    return result;
}

JSON ToJSON(const TextDocumentSaveReason& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const WillSaveTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    Set(&result, "reason", input.reason);
    return result;
}

JSON ToJSON(const TextDocumentSaveRegistrationOptions& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdro));
    OptionallySet(&result, "includeText", input.include_text);
    return result;
}

JSON ToJSON(const DidSaveTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    OptionallySet(&result, "text", input.text);
    return result;
}

JSON ToJSON(const DidCloseTextDocumentParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const PublishDiagnosticsParams& input) {
    JSON result(Json::objectValue);
    Set(&result, "uri", input.uri);
    OptionallySet(&result, "version", input.version);
    Set(&result, "diagnostics", input.diagnostics);
    return result;
}

JSON ToJSON(const CompletionTriggerKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const CompletionContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "triggerKind", input.trigger_kind);
    OptionallySet(&result, "triggerCharacter", input.trigger_character);
    return result;
}

JSON ToJSON(const CompletionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    OptionallySet(&result, "context", input.context);
    return result;
}

JSON ToJSON(const InsertTextFormat& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const InsertReplaceEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "newText", input.new_text);
    Set(&result, "insert", input.insert);
    Set(&result, "replace", input.replace);
    return result;
}

JSON ToJSON(const CompletionItemLabelDetails& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "parameters", input.parameters);
    OptionallySet(&result, "qualifier", input.qualifier);
    OptionallySet(&result, "type", input.type);
    return result;
}

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

JSON ToJSON(const CompletionList& input) {
    JSON result(Json::objectValue);
    Set(&result, "isIncomplete", input.is_incomplete);
    Set(&result, "items", input.items);
    return result;
}

JSON ToJSON(const CompletionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const HoverParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

JSON ToJSON(const Hover& input) {
    JSON result(Json::objectValue);
    Set(&result, "contents", input.contents);
    OptionallySet(&result, "range", input.range);
    return result;
}

JSON ToJSON(const SignatureHelpTriggerKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const ParameterInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "documentation", input.documentation);
    return result;
}

JSON ToJSON(const SignatureInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "documentation", input.documentation);
    OptionallySet(&result, "parameters", input.parameters);
    OptionallySet(&result, "activeParameter", input.active_parameter);
    return result;
}

JSON ToJSON(const SignatureHelp& input) {
    JSON result(Json::objectValue);
    Set(&result, "signatures", input.signatures);
    OptionallySet(&result, "activeSignature", input.active_signature);
    OptionallySet(&result, "activeParameter", input.active_parameter);
    return result;
}

JSON ToJSON(const SignatureHelpContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "triggerKind", input.trigger_kind);
    OptionallySet(&result, "triggerCharacter", input.trigger_character);
    Set(&result, "isRetrigger", input.is_retrigger);
    OptionallySet(&result, "activeSignatureHelp", input.active_signature_help);
    return result;
}

JSON ToJSON(const SignatureHelpParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    OptionallySet(&result, "context", input.context);
    return result;
}

JSON ToJSON(const DeclarationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

JSON ToJSON(const DeclarationResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const DefinitionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

JSON ToJSON(const DefinitionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const TypeDefinitionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

JSON ToJSON(const TypeDefinitionResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const ImplementationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

JSON ToJSON(const ImplementationResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const ReferenceContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "includeDeclaration", input.include_declaration);
    return result;
}

JSON ToJSON(const ReferenceParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "context", input.context);
    return result;
}

JSON ToJSON(const ReferenceResult& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.result));
    return result;
}

JSON ToJSON(const DocumentHighlightParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

JSON ToJSON(const DocumentHighlightKind& input) {
    return static_cast<int32_t>(input);
}

JSON ToJSON(const DocumentHighlight& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

JSON ToJSON(const DocumentSymbolParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

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

JSON ToJSON(const CodeActionContext& input) {
    JSON result(Json::objectValue);
    Set(&result, "diagnostics", input.diagnostics);
    OptionallySet(&result, "only", input.only);
    return result;
}

JSON ToJSON(const CodeActionParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    Set(&result, "context", input.context);
    return result;
}

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

JSON ToJSON(const CodeLensParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const CodeLens& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "command", input.command);
    OptionallySet(&result, "data", input.data);
    return result;
}

JSON ToJSON(const DocumentLinkParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const DocumentLink& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    OptionallySet(&result, "target", input.target);
    OptionallySet(&result, "tooltip", input.tooltip);
    OptionallySet(&result, "data", input.data);
    return result;
}

JSON ToJSON(const DocumentColorParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const Color& input) {
    JSON result(Json::objectValue);
    Set(&result, "red", input.red);
    Set(&result, "green", input.green);
    Set(&result, "blue", input.blue);
    Set(&result, "alpha", input.alpha);
    return result;
}

JSON ToJSON(const ColorInformation& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    Set(&result, "color", input.color);
    return result;
}

JSON ToJSON(const ColorPresentationParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "color", input.color);
    Set(&result, "range", input.range);
    return result;
}

JSON ToJSON(const ColorPresentation& input) {
    JSON result(Json::objectValue);
    Set(&result, "label", input.label);
    OptionallySet(&result, "textEdit", input.text_edit);
    OptionallySet(&result, "additionalTextEdits", input.additional_text_edits);
    return result;
}

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

JSON ToJSON(const DocumentFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "options", input.options);
    return result;
}

JSON ToJSON(const DocumentRangeFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    Set(&result, "options", input.options);
    return result;
}

JSON ToJSON(const DocumentOnTypeFormattingParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Set(&result, "ch", input.ch);
    Set(&result, "options", input.options);
    return result;
}

JSON ToJSON(const RenameParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Set(&result, "newName", input.new_name);
    return result;
}

JSON ToJSON(const PrepareRenameParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    return result;
}

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

JSON ToJSON(const FoldingRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const FoldingRangeKind& input) {
    switch (input) {
    case FoldingRangeKind::Comment: return "comment";
    case FoldingRangeKind::Imports: return "imports";
    case FoldingRangeKind::Region:  return "region";
    }
    RDSS_CHECK(false);
    return JSON();
}

JSON ToJSON(const FoldingRange& input) {
    JSON result(Json::objectValue);
    Set(&result, "startLine", input.start_line);
    OptionallySet(&result, "startCharacter", input.start_character);
    Set(&result, "endLine", input.end_line);
    OptionallySet(&result, "endCharacter", input.end_character);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

JSON ToJSON(const SelectionRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "positions", input.positions);
    return result;
}

JSON ToJSON(const SelectionRange& input) {
    JSON result(Json::objectValue);
    Set(&result, "range", input.range);
    if (input.parent.has_value()) {
        result["parent"] = ToJSON(*(input.parent.value()));
    }
    return result;
}

JSON ToJSON(const CallHierarchyPrepareParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

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

JSON ToJSON(const CallHierarchyIncomingCallsParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "item", input.item);
    return result;
}

JSON ToJSON(const CallHierarchyIncomingCall& input) {
    JSON result(Json::objectValue);
    Set(&result, "from", input.from);
    Set(&result, "fromRanges", input.from_ranges);
    return result;
}

JSON ToJSON(const CallHierarchyOutgoingCallsParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "item", input.item);
    return result;
}

JSON ToJSON(const CallHierarchyOutgoingCall& input) {
    JSON result(Json::objectValue);
    Set(&result, "to", input.to);
    Set(&result, "fromRanges", input.from_ranges);
    return result;
}

JSON ToJSON(const SemanticTokensParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    return result;
}

JSON ToJSON(const SemanticTokens& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "resultId", input.result_id);
    Set(&result, "data", input.data);
    return result;
}

JSON ToJSON(const SemanticTokensDeltaParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "previousResultId", input.previous_result_id);
    return result;
}

JSON ToJSON(const SemanticTokensEdit& input) {
    JSON result(Json::objectValue);
    Set(&result, "start", input.start);
    Set(&result, "deleteCount", input.delete_count);
    OptionallySet(&result, "data", input.data);
    return result;
}

JSON ToJSON(const SemanticTokensDelta& input) {
    JSON result(Json::objectValue);
    OptionallySet(&result, "resultId", input.result_id);
    Set(&result, "edits", input.edits);
    return result;
}

JSON ToJSON(const SemanticTokensRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    Set(&result, "textDocument", input.text_document);
    Set(&result, "range", input.range);
    return result;
}

JSON ToJSON(const LinkedEditingRangeParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    return result;
}

JSON ToJSON(const LinkedEditingRanges& input) {
    JSON result(Json::objectValue);
    Set(&result, "ranges", input.ranges);
    OptionallySet(&result, "wordPattern", input.word_pattern);
    return result;
}

JSON ToJSON(const MonikerParams& input) {
    JSON result(Json::objectValue);
    Merge(&result, ToJSON(input.underlying_tdpp));
    Merge(&result, ToJSON(input.underlying_wdpp));
    Merge(&result, ToJSON(input.underlying_prp));
    return result;
}

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

JSON ToJSON(const MonikerKind& input) {
    switch (input) {
    case MonikerKind::Import: return "import";
    case MonikerKind::Export: return "export";
    case MonikerKind::Local:  return "local";
    }
    RDSS_CHECK(false);
    return JSON();
}

JSON ToJSON(const Moniker& input) {
    JSON result(Json::objectValue);
    Set(&result, "scheme", input.scheme);
    Set(&result, "identifier", input.identifier);
    Set(&result, "unique", input.unique);
    OptionallySet(&result, "kind", input.kind);
    return result;
}

absl::Status FromJSON(const JSON& input, JSON* result) {
    *result = input;
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, bool* result) {
    if (!input.isBool()) {
        return absl::InternalError("Expected bool, got " + input.asString());
    }
    *result = input.asBool();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, std::string* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    *result = input.asString();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, int32_t* result) {
    if (!input.isInt()) {
        return absl::InternalError("Expected int32, got " + input.asString());
    }
    *result = input.asInt();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, int64_t* result) {
    if (!input.isInt64()) {
        return absl::InternalError("Expected int64, got " + input.asString());
    }
    *result = input.asInt64();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, uint8_t* result) {
    if (!input.isUInt()) {
        return absl::InternalError("Expected uint8, got " + input.asString());
    }
    // FIXME: add overflow detection here
    *result = input.asUInt();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, uint32_t* result) {
    if (!input.isUInt()) {
        return absl::InternalError("Expected uint32, got " + input.asString());
    }
    *result = input.asUInt();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, uint64_t* result) {
    if (!input.isUInt64()) {
        return absl::InternalError("Expected uint64, got " + input.asString());
    }
    *result = input.asUInt64();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, double* result) {
    if (!input.isDouble()) {
        return absl::InternalError("Expected double, got " + input.asString());
    }
    *result = input.asDouble();
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, absl::monostate* result) {
    if (input == JSON()) {
        *result = absl::monostate();
        return absl::OkStatus();
    }
    return absl::InternalError("Expected null, got " + input.asString());
}

template<typename A, typename B>
absl::Status FromJSON(const JSON& input, std::pair<A, B>* result);
template<typename A, typename B>
absl::Status FromJSON(const JSON& input, absl::variant<A, B>* result);
template<typename A, typename B, typename C>
absl::Status FromJSON(const JSON& input, absl::variant<A, B, C>* result);
template<typename A, typename B, typename C, typename D>
absl::Status FromJSON(const JSON& input, absl::variant<A, B, C, D>* result);
template<typename T>
absl::Status FromJSON(const JSON& input, absl::optional<T>* result);
template<typename T>
absl::Status FromJSON(const JSON& input, std::vector<T>* result);
template<typename K, typename V>
absl::Status FromJSON(const JSON& input, absl::btree_map<K, V>* result);

template<typename A, typename B>
absl::Status FromJSON(const JSON& input, std::pair<A, B>* result) {
    if (!input.isArray()) {
        return absl::InternalError("Expected array, got " + input.asString());
    }
    if (input.size() != 2) {
        return absl::InternalError(
            "Expected array to have size 2, got " + input.asString());
    }

    RETURN_IF_ERROR(FromJSON(input[0], &result->first));
    RETURN_IF_ERROR(FromJSON(input[1], &result->second));
    return absl::OkStatus();
}

template<typename A, typename B>
absl::Status FromJSON(const JSON& input, absl::variant<A, B>* result) {
    A a; B b;
    absl::Status a_s = FromJSON(input, &a);
    absl::Status b_s = FromJSON(input, &b);

    if (!a_s.ok() && !b_s.ok()) {
        return absl::InternalError(
            "Failed to parse variant: " + input.asString());
    } else if (a_s.ok() && !b_s.ok()) {
        *result = a;
    } else if (!a_s.ok() && b_s.ok()) {
        *result = b;
    } else {
        return absl::InternalError("Parse of 2-variant was ambiguous");
    }
    return absl::OkStatus();
}

template<typename A, typename B, typename C>
absl::Status FromJSON(const JSON& input, absl::variant<A, B, C>* result) {
    A a; B b; C c;
    absl::Status a_s = FromJSON(input, &a);
    absl::Status b_s = FromJSON(input, &b);
    absl::Status c_s = FromJSON(input, &c);

    if (!a_s.ok() && !b_s.ok() && !c_s.ok()) {
        return absl::InternalError(
            "Failed to parse variant: " + input.asString());
    } else if (a_s.ok() && !b_s.ok() && !c_s.ok()) {
        *result = a;
    } else if (!a_s.ok() && b_s.ok() && !c_s.ok()) {
        *result = b;
    } else if (!a_s.ok() && !b_s.ok() && c_s.ok()) {
        *result = c;
    } else {
        return absl::InternalError("Parse of 3-variant was ambiguous");
    }
    return absl::OkStatus();
}

template<typename A, typename B, typename C, typename D>
absl::Status FromJSON(const JSON& input, absl::variant<A, B, C, D>* result) {
    A a; B b; C c; D d;
    absl::Status a_s = FromJSON(input, &a);
    absl::Status b_s = FromJSON(input, &b);
    absl::Status c_s = FromJSON(input, &c);
    absl::Status d_s = FromJSON(input, &d);

    if (!a_s.ok() && !b_s.ok() && !c_s.ok() && !d_s.ok()) {
        return absl::InternalError(
            "Failed to parse variant: " + input.asString());
    } else if (a_s.ok() && !b_s.ok() && !c_s.ok() && !d_s.ok()) {
        *result = a;
    } else if (!a_s.ok() && b_s.ok() && !c_s.ok() && !d_s.ok()) {
        *result = b;
    } else if (!a_s.ok() && !b_s.ok() && c_s.ok() && !d_s.ok()) {
        *result = c;
    } else if (!a_s.ok() && !b_s.ok() && !c_s.ok() && d_s.ok()) {
        *result = d;
    } else {
        return absl::InternalError("Parse of 4-variant was ambiguous");
    }
    return absl::OkStatus();
}

template<typename T>
absl::Status FromJSON(const JSON& input, absl::optional<T>* result) {
    if (input == JSON()) {
        *result = absl::nullopt;
    } else {
        T t;
        RETURN_IF_ERROR(FromJSON(input, &t));
        *result = t;
    }
    return absl::OkStatus();
}

template<typename T>
absl::Status FromJSON(const JSON& input, std::vector<T>* result) {
    if (!input.isArray()) {
        return absl::InternalError("Expected array, got " + input.asString());
    }
    *result = std::vector<T>();
    for (int32_t i = 0; i < input.size(); i++) {
        T temp;
        RETURN_IF_ERROR(FromJSON(input[i], &temp));
        result->push_back(temp);
    }
    return absl::OkStatus();
}

template<typename K, typename V>
absl::Status FromJSON(const JSON& input, absl::btree_map<K, V>* result) {
    if (!input.isObject()) {
        return absl::InternalError("Expected object, got " + input.asString());
    }
    *result = absl::btree_map<K, V>();
    for (const std::string& key : input.getMemberNames()) {
        RETURN_IF_ERROR(FromJSON(input[key], &((*result)[key])));
    }
    return absl::OkStatus();
}

template<typename T>
absl::Status Get(T* field, const JSON& input, const std::string& entry) {
    return FromJSON(input[entry], field);
}

template<typename T>
absl::Status OptionallyGet(absl::optional<T>* field,
                           const JSON& input,
                           const std::string& entry) {
    return FromJSON(input.get(entry, JSON()), field);
}

absl::StatusOr<int32_t> JsonToInt32(const JSON& input) {
    if (!input.isInt()) {
        return absl::InternalError("Expected integral, got " + input.asString());
    }
    return input.asInt();
}

absl::Status FromJSON(const JSON& input, ErrorCodes* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    bool valid = false;
    valid |= (code == -32700);
    valid |= (code == -32600);
    valid |= (code == -32601);
    valid |= (code == -32602);
    valid |= (code == -32603);
    valid |= (code == -32002);
    valid |= (code == -32001);
    valid |= (code == -32801);
    valid |= (code == -32800);
    if (!valid) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse ErrorCodes: code was %d", code));
    }
    *result = static_cast<ErrorCodes>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ResponseError* result) {
    RETURN_IF_ERROR(Get(&result->code, input, "code"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ResponseMessage* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->id, input, "id"));
    RETURN_IF_ERROR(OptionallyGet(&result->result, input, "result"));
    RETURN_IF_ERROR(OptionallyGet(&result->error, input, "error"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RegularExpressionsClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->engine, input, "engine"));
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Position* result) {
    RETURN_IF_ERROR(Get(&result->line, input, "line"));
    RETURN_IF_ERROR(Get(&result->character, input, "character"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Range* result) {
    RETURN_IF_ERROR(Get(&result->start, input, "start"));
    RETURN_IF_ERROR(Get(&result->end, input, "end"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Location* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LocationLink* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->origin_selection_range, input, "originSelectionRange"));
    RETURN_IF_ERROR(Get(&result->target_uri, input, "targetUri"));
    RETURN_IF_ERROR(Get(&result->target_range, input, "targetRange"));
    RETURN_IF_ERROR(Get(&result->target_selection_range, input, "targetSelectionRange"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DiagnosticSeverity* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DiagnosticSeverity: code was %d < 1", code));
    }
    if (code > 4) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DiagnosticSeverity: code was %d > 4", code));
    }
    *result = static_cast<DiagnosticSeverity>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DiagnosticCode* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->code));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeDescription* result) {
    RETURN_IF_ERROR(Get(&result->href, input, "href"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DiagnosticTag* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DiagnosticTag: code was %d < 1", code));
    }
    if (code > 2) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DiagnosticTag: code was %d > 2", code));
    }
    *result = static_cast<DiagnosticTag>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DiagnosticRelatedInformation* result) {
    RETURN_IF_ERROR(Get(&result->location, input, "location"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Diagnostic* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->severity, input, "severity"));
    RETURN_IF_ERROR(OptionallyGet(&result->code, input, "code"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_description, input, "codeDescription"));
    RETURN_IF_ERROR(OptionallyGet(&result->source, input, "source"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    RETURN_IF_ERROR(Get(&result->tags, input, "tags"));
    RETURN_IF_ERROR(Get(&result->related_information, input, "relatedInformation"));
    RETURN_IF_ERROR(Get(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Command* result) {
    RETURN_IF_ERROR(Get(&result->title, input, "title"));
    RETURN_IF_ERROR(Get(&result->command, input, "command"));
    RETURN_IF_ERROR(OptionallyGet(&result->arguments, input, "arguments"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextEdit* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->new_text, input, "newText"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ChangeAnnotation* result) {
    RETURN_IF_ERROR(Get(&result->label, input, "label"));
    RETURN_IF_ERROR(OptionallyGet(&result->needs_confirmation, input, "needsConfirmation"));
    RETURN_IF_ERROR(OptionallyGet(&result->description, input, "description"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentIdentifier* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentItem* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(Get(&result->language_id, input, "languageId"));
    RETURN_IF_ERROR(Get(&result->version, input, "version"));
    RETURN_IF_ERROR(Get(&result->text, input, "text"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, VersionedTextDocumentIdentifier* result) {
    RETURN_IF_ERROR(Get(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, OptionalVersionedTextDocumentIdentifier* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, AnnotatedTextEdit* result) {
    RETURN_IF_ERROR(Get(&result->annotation_id, input, "annotationId"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentEdit* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->edits, input, "edits"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CreateFileOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->overwrite, input, "overwrite"));
    RETURN_IF_ERROR(OptionallyGet(&result->ignore_if_exists, input, "ignoreIfExists"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CreateFile* result) {
    if (input["kind"] != "create") {
        return absl::InternalError("CreateFile needs {kind: \"create\", ...}");
    }
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(OptionallyGet(&result->options, input, "options"));
    RETURN_IF_ERROR(OptionallyGet(&result->annotation_id, input, "annotationId"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameFileOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->overwrite, input, "overwrite"));
    RETURN_IF_ERROR(OptionallyGet(&result->ignore_if_exists, input, "ignoreIfExists"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameFile* result) {
    if (input["kind"] != "rename") {
        return absl::InternalError("RenameFile needs {kind: \"rename\", ...}");
    }
    RETURN_IF_ERROR(Get(&result->old_uri, input, "oldUri"));
    RETURN_IF_ERROR(Get(&result->new_uri, input, "newUri"));
    RETURN_IF_ERROR(OptionallyGet(&result->options, input, "options"));
    RETURN_IF_ERROR(OptionallyGet(&result->annotation_id, input, "annotationId"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeleteFileOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->recursive, input, "recursive"));
    RETURN_IF_ERROR(OptionallyGet(&result->ignore_if_not_exists, input, "ignoreIfNotExists"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeleteFile* result) {
    if (input["kind"] != "delete") {
        return absl::InternalError("DeleteFile needs {kind: \"delete\", ...}");
    }
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(OptionallyGet(&result->options, input, "options"));
    RETURN_IF_ERROR(OptionallyGet(&result->annotation_id, input, "annotationId"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentChangeOperation* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->operation));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentChanges* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->changes));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceEdit* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->changes, input, "changes"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_changes, input, "documentChanges"));
    RETURN_IF_ERROR(OptionallyGet(&result->change_annotations, input, "changeAnnotations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ResourceOperationKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "create") {
        *result = ResourceOperationKind::Create;
    } else if (input == "rename") {
        *result = ResourceOperationKind::Rename;
    } else if (input == "delete") {
        *result = ResourceOperationKind::Delete;
    } else {
        return absl::InternalError(
            "Invalid string for ResourceOperationKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FailureHandlingKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "abort") {
        *result = FailureHandlingKind::Abort;
    } else if (input == "transactional") {
        *result = FailureHandlingKind::Transactional;
    } else if (input == "undo") {
        *result = FailureHandlingKind::Undo;
    } else if (input == "textOnlyTransactional") {
        *result = FailureHandlingKind::TextOnlyTransactional;
    } else {
        return absl::InternalError(
            "Invalid string for FailureHandlingKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ChangeAnnotationSupport* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->groups_on_label, input, "groupsOnLabel"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceEditClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->document_changes, input, "documentChanges"));
    RETURN_IF_ERROR(OptionallyGet(&result->resource_operations, input, "resourceOperations"));
    RETURN_IF_ERROR(OptionallyGet(&result->failure_handling, input, "failureHandling"));
    RETURN_IF_ERROR(OptionallyGet(&result->normalizes_line_endings, input, "normalizesLineEndings"));
    RETURN_IF_ERROR(OptionallyGet(&result->change_annotation_support, input, "changeAnnotationSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentPositionParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->position, input, "position"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentFilter* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->language, input, "language"));
    RETURN_IF_ERROR(OptionallyGet(&result->scheme, input, "scheme"));
    RETURN_IF_ERROR(OptionallyGet(&result->pattern, input, "pattern"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, StaticRegistrationOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->id, input, "id"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentRegistrationOptions* result) {
    RETURN_IF_ERROR(Get(&result->document_selector, input, "documentSelector"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MarkupKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "plaintext") {
        *result = MarkupKind::PlainText;
    } else if (input == "markdown") {
        *result = MarkupKind::Markdown;
    } else {
        return absl::InternalError(
            "Invalid string for MarkupKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MarkupContent* result) {
    RETURN_IF_ERROR(Get(&result->kind, input, "kind"));
    RETURN_IF_ERROR(Get(&result->value, input, "value"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MarkdownClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->parser, input, "parser"));
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressBegin* result) {
    if (input["kind"] != "begin") {
        return absl::InternalError(
            "WorkDoneProgressBegin needs {kind: \"begin\", ...}");
    }
    RETURN_IF_ERROR(Get(&result->title, input, "title"));
    RETURN_IF_ERROR(OptionallyGet(&result->cancellable, input, "cancellable"));
    RETURN_IF_ERROR(OptionallyGet(&result->message, input, "message"));
    RETURN_IF_ERROR(OptionallyGet(&result->percentage, input, "percentage"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressReport* result) {
    if (input["kind"] != "report") {
        return absl::InternalError(
            "WorkDoneProgressReport needs {kind: \"report\", ...}");
    }
    RETURN_IF_ERROR(OptionallyGet(&result->cancellable, input, "cancellable"));
    RETURN_IF_ERROR(OptionallyGet(&result->message, input, "message"));
    RETURN_IF_ERROR(OptionallyGet(&result->percentage, input, "percentage"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressEnd* result) {
    if (input["kind"] != "end") {
        return absl::InternalError(
            "WorkDoneProgressEnd needs {kind: \"end\", ...}");
    }
    RETURN_IF_ERROR(OptionallyGet(&result->message, input, "message"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressParams* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->work_done_token, input, "workDoneToken"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PartialResultParams* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->partial_result_token, input, "partialResultToken"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TraceValue* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "off") {
        *result = TraceValue::Off;
    } else if (input == "messages") {
        *result = TraceValue::Messages;
    } else if (input == "verbose") {
        *result = TraceValue::Verbose;
    } else {
        return absl::InternalError(
            "Invalid string for TraceValue: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ClientInfo* result) {
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SymbolKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SymbolKind: code was %d < 1", code));
    }
    if (code > 26) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SymbolKind: code was %d > 26", code));
    }
    *result = static_cast<SymbolKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SymbolKindClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SymbolTag* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SymbolTag: code was %d < 1", code));
    }
    if (code > 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SymbolTag: code was %d > 1", code));
    }
    *result = static_cast<SymbolTag>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SymbolTagSupportClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentSyncClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_save, input, "willSave"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_save_wait_until, input, "willSaveWaitUntil"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_save, input, "didSave"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DiagnosticTagSupportClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PublishDiagnosticsClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->related_information, input, "relatedInformation"));
    RETURN_IF_ERROR(OptionallyGet(&result->tag_support, input, "tagSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->version_support, input, "versionSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_description_support, input, "codeDescriptionSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->data_support, input, "dataSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemTag* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionItemTag: code was %d < 1", code));
    }
    if (code > 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionItemTag: code was %d > 1", code));
    }
    *result = static_cast<CompletionItemTag>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemTagSupportClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemResolveSupportClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->properties, input, "properties"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InsertTextMode* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InsertTextMode: code was %d < 1", code));
    }
    if (code > 2) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InsertTextMode: code was %d > 2", code));
    }
    *result = static_cast<InsertTextMode>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemInsertTextModeSupportClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->snippet_support, input, "snippetSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->commit_characters_support, input, "commitCharactersSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->documentation_format, input, "documentationFormat"));
    RETURN_IF_ERROR(OptionallyGet(&result->deprecated_support, input, "deprecatedSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->preselect_support, input, "preselectSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->tag_support, input, "tagSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_replace_support, input, "insertReplaceSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_support, input, "resolveSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_text_mode_support, input, "insertTextModeSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->label_details_support, input, "labelDetailsSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionItemKind: code was %d < 1", code));
    }
    if (code > 25) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionItemKind: code was %d > 25", code));
    }
    *result = static_cast<CompletionItemKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemKindClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->completion_item, input, "completionItem"));
    RETURN_IF_ERROR(OptionallyGet(&result->completion_item_kind, input, "completionItemKind"));
    RETURN_IF_ERROR(OptionallyGet(&result->context_support, input, "contextSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_text_mode, input, "insertTextMode"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, HoverClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->content_format, input, "contentFormat"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ParameterInformationClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->label_offset_support, input, "labelOffsetSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureInformationClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->documentation_format, input, "documentationFormat"));
    RETURN_IF_ERROR(OptionallyGet(&result->parameter_information, input, "parameterInformation"));
    RETURN_IF_ERROR(OptionallyGet(&result->active_parameter_support, input, "activeParameterSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->signature_information, input, "signatureInformation"));
    RETURN_IF_ERROR(OptionallyGet(&result->context_support, input, "contextSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeclarationClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->link_support, input, "linkSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DefinitionClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->link_support, input, "linkSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TypeDefinitionClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->link_support, input, "linkSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ImplementationClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->link_support, input, "linkSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlightClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentSymbolClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->symbol_kind, input, "symbolKind"));
    RETURN_IF_ERROR(OptionallyGet(&result->hierarchical_document_symbol_support, input, "hierarchicalDocumentSymbolSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->tag_support, input, "tagSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->label_support, input, "labelSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "") {
        *result = CodeActionKind::Empty;
    } else if (input == "quickfix") {
        *result = CodeActionKind::QuickFix;
    } else if (input == "refactor") {
        *result = CodeActionKind::Refactor;
    } else if (input == "refactor.extract") {
        *result = CodeActionKind::RefactorExtract;
    } else if (input == "refactor.inline") {
        *result = CodeActionKind::RefactorInline;
    } else if (input == "refactor.rewrite") {
        *result = CodeActionKind::RefactorRewrite;
    } else if (input == "source") {
        *result = CodeActionKind::Source;
    } else if (input == "source.organizeImports") {
        *result = CodeActionKind::SourceOrganizeImports;
    } else if (input == "source.fixAll") {
        *result = CodeActionKind::SourceFixAll;
    } else {
        return absl::InternalError(
            "Invalid string for CodeActionKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionKindClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->value_set, input, "valueSet"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionLiteralClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->code_action_kind, input, "codeActionKind"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionResolveClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->properties, input, "properties"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_action_literal_support, input, "codeActionLiteralSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->is_preferred_support, input, "isPreferredSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->disabled_support, input, "disabledSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->data_support, input, "dataSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_support, input, "resolveSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->honors_change_annotations, input, "honorsChangeAnnotations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLensClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentLinkClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->tooltip_support, input, "tooltipSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentColorClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentFormattingClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentRangeFormattingClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PrepareSupportDefaultBehavior* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse PrepareSupportDefaultBehavior: code was %d < 1", code));
    }
    if (code > 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse PrepareSupportDefaultBehavior: code was %d > 1", code));
    }
    *result = static_cast<PrepareSupportDefaultBehavior>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->prepare_support, input, "prepareSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->prepare_support_default_behavior, input, "prepareSupportDefaultBehavior"));
    RETURN_IF_ERROR(OptionallyGet(&result->honors_change_annotations, input, "honorsChangeAnnotations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRangeClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->range_limit, input, "rangeLimit"));
    RETURN_IF_ERROR(OptionallyGet(&result->line_folding_only, input, "lineFoldingOnly"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SelectionRangeClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensRangeClientCapabilities* result) {
    if (!input.isObject()) {
        return absl::InternalError("Expected object, got " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensFullClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->delta, input, "delta"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TokenFormat* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "relative") {
        *result = TokenFormat::Relative;
    } else {
        return absl::InternalError(
            "Invalid string for TokenFormat: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->requests_range, input["requests"], "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->requests_full, input["requests"], "full"));
    RETURN_IF_ERROR(Get(&result->token_types, input, "tokenTypes"));
    RETURN_IF_ERROR(Get(&result->token_modifiers, input, "tokenModifiers"));
    RETURN_IF_ERROR(Get(&result->formats, input, "formats"));
    RETURN_IF_ERROR(OptionallyGet(&result->overlapping_token_support, input, "overlappingTokenSupport"));
    RETURN_IF_ERROR(OptionallyGet(&result->multiline_token_support, input, "multilineTokenSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LinkedEditingRangeClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MonikerClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->synchronization, input, "synchronization"));
    RETURN_IF_ERROR(OptionallyGet(&result->completion, input, "completion"));
    RETURN_IF_ERROR(OptionallyGet(&result->hover, input, "hover"));
    RETURN_IF_ERROR(OptionallyGet(&result->signature_help, input, "signatureHelp"));
    RETURN_IF_ERROR(OptionallyGet(&result->declaration, input, "declaration"));
    RETURN_IF_ERROR(OptionallyGet(&result->definition, input, "definition"));
    RETURN_IF_ERROR(OptionallyGet(&result->type_definition, input, "typeDefinition"));
    RETURN_IF_ERROR(OptionallyGet(&result->implementation, input, "implementation"));
    RETURN_IF_ERROR(OptionallyGet(&result->references, input, "references"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_highlight, input, "documentHighlight"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_symbol, input, "documentSymbol"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_action, input, "codeAction"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_lens, input, "codeLens"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_link, input, "documentLink"));
    RETURN_IF_ERROR(OptionallyGet(&result->color_provider, input, "colorProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->formatting, input, "formatting"));
    RETURN_IF_ERROR(OptionallyGet(&result->range_formatting, input, "rangeFormatting"));
    RETURN_IF_ERROR(OptionallyGet(&result->on_type_formatting, input, "onTypeFormatting"));
    RETURN_IF_ERROR(OptionallyGet(&result->rename, input, "rename"));
    RETURN_IF_ERROR(OptionallyGet(&result->publish_diagnostics, input, "publishDiagnostics"));
    RETURN_IF_ERROR(OptionallyGet(&result->folding_range, input, "foldingRange"));
    RETURN_IF_ERROR(OptionallyGet(&result->selection_range, input, "selectionRange"));
    RETURN_IF_ERROR(OptionallyGet(&result->linked_editing_range, input, "linkedEditingRange"));
    RETURN_IF_ERROR(OptionallyGet(&result->call_hierarchy, input, "callHierarchy"));
    RETURN_IF_ERROR(OptionallyGet(&result->semantic_tokens, input, "semanticTokens"));
    RETURN_IF_ERROR(OptionallyGet(&result->moniker, input, "moniker"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationsWorkspaceClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_create, input, "didCreate"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_create, input, "willCreate"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_rename, input, "didRename"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_rename, input, "willRename"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_delete, input, "didDelete"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_delete, input, "willDelete"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeConfigurationClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSymbolClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    RETURN_IF_ERROR(OptionallyGet(&result->symbol_kind, input, "symbolKind"));
    RETURN_IF_ERROR(OptionallyGet(&result->tag_support, input, "tagSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ExecuteCommandClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->dynamic_registration, input, "dynamicRegistration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLensWorkspaceClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->refresh_support, input, "refreshSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensWorkspaceClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->refresh_support, input, "refreshSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSpecificClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->apply_edit, input, "applyEdit"));
    RETURN_IF_ERROR(OptionallyGet(&result->workspace_edit, input, "workspaceEdit"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_change_configuration, input, "didChangeConfiguration"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_change_watched_files, input, "didChangeWatchedFiles"));
    RETURN_IF_ERROR(OptionallyGet(&result->symbol, input, "symbol"));
    RETURN_IF_ERROR(OptionallyGet(&result->execute_command, input, "executeCommand"));
    RETURN_IF_ERROR(OptionallyGet(&result->workspace_folders, input, "workspaceFolders"));
    RETURN_IF_ERROR(OptionallyGet(&result->configuration, input, "configuration"));
    RETURN_IF_ERROR(OptionallyGet(&result->semantic_tokens, input, "semanticTokens"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_lens, input, "codeLens"));
    RETURN_IF_ERROR(OptionallyGet(&result->file_operations, input, "fileOperations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MessageActionItemClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->additional_properties_support, input, "additionalPropertiesSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowMessageRequestClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->message_action_item, input, "messageActionItem"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowDocumentClientCapabilities* result) {
    RETURN_IF_ERROR(Get(&result->support, input, "support"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WindowSpecificClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->work_done_progress, input, "workDoneProgress"));
    RETURN_IF_ERROR(OptionallyGet(&result->show_message, input, "showMessage"));
    RETURN_IF_ERROR(OptionallyGet(&result->show_document, input, "showDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, GeneralClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->regular_expressions, input, "regularExpressions"));
    RETURN_IF_ERROR(OptionallyGet(&result->markdown, input, "markdown"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ClientCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->workspace, input, "workspace"));
    RETURN_IF_ERROR(OptionallyGet(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(OptionallyGet(&result->window, input, "window"));
    RETURN_IF_ERROR(OptionallyGet(&result->general, input, "general"));
    RETURN_IF_ERROR(OptionallyGet(&result->experimental, input, "experimental"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationPatternKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "file") {
        *result = FileOperationPatternKind::File;
    } else if (input == "folder") {
        *result = FileOperationPatternKind::Folder;
    } else {
        return absl::InternalError(
            "Invalid string for FileOperationPatternKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationPatternOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->ignore_case, input, "ignoreCase"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationPattern* result) {
    RETURN_IF_ERROR(Get(&result->glob, input, "glob"));
    RETURN_IF_ERROR(OptionallyGet(&result->matches, input, "matches"));
    RETURN_IF_ERROR(OptionallyGet(&result->options, input, "options"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationFilter* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->scheme, input, "scheme"));
    RETURN_IF_ERROR(Get(&result->pattern, input, "pattern"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationRegistrationOptions* result) {
    RETURN_IF_ERROR(Get(&result->filters, input, "filters"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileOperationsWorkspaceServerCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->did_create, input, "didCreate"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_create, input, "willCreate"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_rename, input, "didRename"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_rename, input, "willRename"));
    RETURN_IF_ERROR(OptionallyGet(&result->did_delete, input, "didDelete"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_delete, input, "willDelete"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceFoldersServerCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->supported, input, "supported"));
    RETURN_IF_ERROR(OptionallyGet(&result->change_notifications, input, "changeNotifications"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSpecificServerCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->workspace_folders, input, "workspaceFolders"));
    RETURN_IF_ERROR(OptionallyGet(&result->file_operations, input, "fileOperations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WatchKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    bool valid = false;
    valid |= (code == 1);
    valid |= (code == 2);
    valid |= (code == 4);
    if (!valid) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse WatchKind: code was %d", code));
    }
    *result = static_cast<WatchKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileSystemWatcher* result) {
    RETURN_IF_ERROR(Get(&result->glob_pattern, input, "globPattern"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesRegistrationOptions* result) {
    RETURN_IF_ERROR(Get(&result->watchers, input, "watchers"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->work_done_progress, input, "workDoneProgress"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSymbolOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_work_done_progress));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSymbolRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_workspace_symbol_options));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ExecuteCommandOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_work_done_progress));
    RETURN_IF_ERROR(Get(&result->commands, input, "commands"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ExecuteCommandRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_execute_command_options));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentSyncKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 0) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse TextDocumentSyncKind: code was %d < 0", code));
    }
    if (code > 2) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse TextDocumentSyncKind: code was %d > 2", code));
    }
    *result = static_cast<TextDocumentSyncKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SaveOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->include_text, input, "includeText"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentSyncOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->open_close, input, "openClose"));
    RETURN_IF_ERROR(OptionallyGet(&result->change, input, "change"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_save, input, "willSave"));
    RETURN_IF_ERROR(OptionallyGet(&result->will_save_wait_until, input, "willSaveWaitUntil"));
    RETURN_IF_ERROR(OptionallyGet(&result->save, input, "save"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->label_details_support, input, "labelDetailsSupport"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->trigger_characters, input, "triggerCharacters"));
    RETURN_IF_ERROR(OptionallyGet(&result->all_commit_characters, input, "allCommitCharacters"));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_provider, input, "resolveProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->completion_item, input, "completionItem"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_co));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, HoverOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, HoverRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_ho));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->trigger_characters, input, "triggerCharacters"));
    RETURN_IF_ERROR(OptionallyGet(&result->retrigger_characters, input, "retriggerCharacters"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sho));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeclarationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeclarationRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_do));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DefinitionOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DefinitionRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_do));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TypeDefinitionOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TypeDefinitionRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ImplementationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ImplementationRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_io));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_ro));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlightOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlightRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dho));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentSymbolOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->label, input, "label"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentSymbolRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dso));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->code_action_kinds, input, "codeActionKinds"));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_provider, input, "resolveProvider"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_cao));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLensOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_provider, input, "resolveProvider"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLensRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_clo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentLinkOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->resolve_provider, input, "resolveProvider"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentLinkRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dlo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentColorOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentColorRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dco));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentFormattingOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentFormattingRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dfo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentRangeFormattingOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentRangeFormattingRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_drfo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingOptions* result) {
    RETURN_IF_ERROR(Get(&result->first_trigger_character, input, "firstTriggerCharacter"));
    RETURN_IF_ERROR(OptionallyGet(&result->more_trigger_character, input, "moreTriggerCharacter"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_dotfo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(OptionallyGet(&result->prepare_provider, input, "prepareProvider"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_ro));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRangeOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRangeRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_fro));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SelectionRangeOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SelectionRangeRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_selro));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_cho));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensLegend* result) {
    RETURN_IF_ERROR(Get(&result->token_types, input, "tokenTypes"));
    RETURN_IF_ERROR(Get(&result->token_modifiers, input, "tokenModifiers"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensRangeOptions* result) {
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensFullOptions* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->delta, input, "delta"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(Get(&result->legend, input, "legend"));
    RETURN_IF_ERROR(OptionallyGet(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->full, input, "full"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sto));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LinkedEditingRangeOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LinkedEditingRangeRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_sro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_lero));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MonikerOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MonikerRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_mo));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ServerCapabilities* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->text_document_sync, input, "textDocumentSync"));
    RETURN_IF_ERROR(OptionallyGet(&result->completion_provider, input, "completionProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->hover_provider, input, "hoverProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->signature_help_provider, input, "signatureHelpProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->declaration_provider, input, "declarationProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->definition_provider, input, "definitionProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->type_definition_provider, input, "typeDefinitionProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->implementation_provider, input, "implementationProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->references_provider, input, "referencesProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_highlight_provider, input, "documentHighlightProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_symbol_provider, input, "documentSymbolProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_action_provider, input, "codeActionProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->code_lens_provider, input, "codeLensProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_link_provider, input, "documentLinkProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->color_provider, input, "colorProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_formatting_provider, input, "documentFormattingProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_range_formatting_provider, input, "documentRangeFormattingProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->document_on_type_formatting_provider, input, "documentOnTypeFormattingProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->rename_provider, input, "renameProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->folding_range_provider, input, "foldingRangeProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->execute_command_provider, input, "executeCommandProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->selection_range_provider, input, "selectionRangeProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->linked_editing_range_provider, input, "linkedEditingRangeProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->call_hierarchy_provider, input, "callHierarchyProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->semantic_tokens_provider, input, "semanticTokensProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->moniker_provider, input, "monikerProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->workspace_symbol_provider, input, "workspaceSymbolProvider"));
    RETURN_IF_ERROR(OptionallyGet(&result->workspace, input, "workspace"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ServerInfo* result) {
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceFolder* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InitializeParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpo));
    RETURN_IF_ERROR(Get(&result->process_id, input, "processId"));
    RETURN_IF_ERROR(OptionallyGet(&result->client_info, input, "clientInfo"));
    RETURN_IF_ERROR(OptionallyGet(&result->locale, input, "locale"));
    RETURN_IF_ERROR(OptionallyGet(&result->root_path, input, "rootPath"));
    RETURN_IF_ERROR(OptionallyGet(&result->root_uri, input, "rootUri"));
    RETURN_IF_ERROR(OptionallyGet(&result->initialization_options, input, "initializationOptions"));
    RETURN_IF_ERROR(Get(&result->capabilities, input, "capabilities"));
    RETURN_IF_ERROR(OptionallyGet(&result->trace, input, "trace"));
    RETURN_IF_ERROR(OptionallyGet(&result->workspace_folders, input, "workspaceFolders"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InitializeResult* result) {
    RETURN_IF_ERROR(Get(&result->capabilities, input, "capabilities"));
    RETURN_IF_ERROR(OptionallyGet(&result->server_info, input, "serverInfo"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InitializeErrorCode* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InitializeErrorCode: code was %d < 1", code));
    }
    if (code > 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InitializeErrorCode: code was %d > 1", code));
    }
    *result = static_cast<InitializeErrorCode>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InitializeErrorData* result) {
    RETURN_IF_ERROR(Get(&result->retry, input, "retry"));
    return absl::OkStatus();
}

template<typename T>
absl::Status FromJSON(const JSON& input, ProgressParams<T>* result) {
    RETURN_IF_ERROR(Get(&result->token, input, "token"));
    RETURN_IF_ERROR(Get(&result->value, input, "value"));
    return absl::OkStatus();
}

template absl::Status FromJSON(const JSON& input, ProgressParams<JSON>* result);
template absl::Status FromJSON(const JSON& input, ProgressParams<WorkDoneProgressBegin>* result);
template absl::Status FromJSON(const JSON& input, ProgressParams<WorkDoneProgressReport>* result);
template absl::Status FromJSON(const JSON& input, ProgressParams<WorkDoneProgressEnd>* result);

absl::Status FromJSON(const JSON& input, InitializedParams* result) {
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LogTraceParams* result) {
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    RETURN_IF_ERROR(OptionallyGet(&result->verbose, input, "verbose"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SetTraceParams* result) {
    RETURN_IF_ERROR(Get(&result->value, input, "value"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MessageType* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse MessageType: code was %d < 1", code));
    }
    if (code > 4) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse MessageType: code was %d > 4", code));
    }
    *result = static_cast<MessageType>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowMessageParams* result) {
    RETURN_IF_ERROR(Get(&result->type, input, "type"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MessageActionItem* result) {
    RETURN_IF_ERROR(Get(&result->title, input, "title"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowMessageRequestParams* result) {
    RETURN_IF_ERROR(Get(&result->type, input, "type"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    RETURN_IF_ERROR(OptionallyGet(&result->actions, input, "actions"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowMessageRequestResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->action));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(OptionallyGet(&result->external, input, "external"));
    RETURN_IF_ERROR(OptionallyGet(&result->take_focus, input, "takeFocus"));
    RETURN_IF_ERROR(OptionallyGet(&result->selection, input, "selection"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ShowDocumentResult* result) {
    RETURN_IF_ERROR(Get(&result->success, input, "success"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LogMessageParams* result) {
    RETURN_IF_ERROR(Get(&result->type, input, "type"));
    RETURN_IF_ERROR(Get(&result->message, input, "message"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressCreateParams* result) {
    RETURN_IF_ERROR(Get(&result->token, input, "token"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkDoneProgressCancelParams* result) {
    RETURN_IF_ERROR(Get(&result->token, input, "token"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Registration* result) {
    RETURN_IF_ERROR(Get(&result->id, input, "id"));
    RETURN_IF_ERROR(Get(&result->method, input, "method"));
    RETURN_IF_ERROR(OptionallyGet(&result->register_options, input, "registerOptions"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RegistrationParams* result) {
    RETURN_IF_ERROR(Get(&result->registrations, input, "registrations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Unregistration* result) {
    RETURN_IF_ERROR(Get(&result->id, input, "id"));
    RETURN_IF_ERROR(Get(&result->method, input, "method"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, UnregistrationParams* result) {
    RETURN_IF_ERROR(Get(&result->unregisterations, input, "unregisterations"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceFoldersChangeEvent* result) {
    RETURN_IF_ERROR(Get(&result->added, input, "added"));
    RETURN_IF_ERROR(Get(&result->removed, input, "removed"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeWorkspaceFoldersParams* result) {
    RETURN_IF_ERROR(Get(&result->event, input, "event"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeConfigurationParams* result) {
    RETURN_IF_ERROR(Get(&result->settings, input, "settings"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ConfigurationItem* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->scope_uri, input, "scopeUri"));
    RETURN_IF_ERROR(OptionallyGet(&result->section, input, "section"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ConfigurationParams* result) {
    RETURN_IF_ERROR(Get(&result->items, input, "items"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileChangeType* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse FileChangeType: code was %d < 1", code));
    }
    if (code > 3) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse FileChangeType: code was %d > 3", code));
    }
    *result = static_cast<FileChangeType>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileEvent* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(Get(&result->type, input, "type"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeWatchedFilesParams* result) {
    RETURN_IF_ERROR(Get(&result->changes, input, "changes"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSymbolParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_work_done_progress));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_partial_result));
    RETURN_IF_ERROR(Get(&result->query, input, "query"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SymbolInformation* result) {
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    RETURN_IF_ERROR(Get(&result->kind, input, "kind"));
    RETURN_IF_ERROR(OptionallyGet(&result->tags, input, "tags"));
    RETURN_IF_ERROR(OptionallyGet(&result->deprecated, input, "deprecated"));
    RETURN_IF_ERROR(Get(&result->location, input, "location"));
    RETURN_IF_ERROR(OptionallyGet(&result->container_name, input, "containerName"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WorkspaceSymbolResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->symbols));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ExecuteCommandParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_work_done_progress));
    RETURN_IF_ERROR(Get(&result->command, input, "command"));
    RETURN_IF_ERROR(OptionallyGet(&result->arguments, input, "arguments"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ApplyWorkspaceEditParams* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->label, input, "label"));
    RETURN_IF_ERROR(Get(&result->edit, input, "edit"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ApplyWorkspaceEditResult* result) {
    RETURN_IF_ERROR(Get(&result->applied, input, "applied"));
    RETURN_IF_ERROR(OptionallyGet(&result->failure_reason, input, "failureReason"));
    RETURN_IF_ERROR(OptionallyGet(&result->failed_change, input, "failedChange"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileCreate* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CreateFilesParams* result) {
    RETURN_IF_ERROR(Get(&result->files, input, "files"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileRename* result) {
    RETURN_IF_ERROR(Get(&result->old_uri, input, "oldUri"));
    RETURN_IF_ERROR(Get(&result->new_uri, input, "newUri"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameFilesParams* result) {
    RETURN_IF_ERROR(Get(&result->files, input, "files"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FileDelete* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeleteFilesParams* result) {
    RETURN_IF_ERROR(Get(&result->files, input, "files"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidOpenTextDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentChangeRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(Get(&result->sync_kind, input, "syncKind"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentContentChangeEvent* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->range_length, input, "rangeLength"));
    RETURN_IF_ERROR(Get(&result->text, input, "text"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidChangeTextDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->content_changes, input, "contentChanges"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentSaveReason* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse TextDocumentSaveReason: code was %d < 1", code));
    }
    if (code > 3) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse TextDocumentSaveReason: code was %d > 3", code));
    }
    *result = static_cast<TextDocumentSaveReason>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, WillSaveTextDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->reason, input, "reason"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TextDocumentSaveRegistrationOptions* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdro));
    RETURN_IF_ERROR(OptionallyGet(&result->include_text, input, "includeText"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidSaveTextDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(OptionallyGet(&result->text, input, "text"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DidCloseTextDocumentParams* result) {
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PublishDiagnosticsParams* result) {
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(OptionallyGet(&result->version, input, "version"));
    RETURN_IF_ERROR(Get(&result->diagnostics, input, "diagnostics"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionTriggerKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionTriggerKind: code was %d < 1", code));
    }
    if (code > 3) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse CompletionTriggerKind: code was %d > 3", code));
    }
    *result = static_cast<CompletionTriggerKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionContext* result) {
    RETURN_IF_ERROR(Get(&result->trigger_kind, input, "triggerKind"));
    RETURN_IF_ERROR(OptionallyGet(&result->trigger_character, input, "triggerCharacter"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(OptionallyGet(&result->context, input, "context"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InsertTextFormat* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InsertTextFormat: code was %d < 1", code));
    }
    if (code > 2) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse InsertTextFormat: code was %d > 2", code));
    }
    *result = static_cast<InsertTextFormat>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, InsertReplaceEdit* result) {
    RETURN_IF_ERROR(Get(&result->new_text, input, "newText"));
    RETURN_IF_ERROR(Get(&result->insert, input, "insert"));
    RETURN_IF_ERROR(Get(&result->replace, input, "replace"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItemLabelDetails* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->parameters, input, "parameters"));
    RETURN_IF_ERROR(OptionallyGet(&result->qualifier, input, "qualifier"));
    RETURN_IF_ERROR(OptionallyGet(&result->type, input, "type"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionItem* result) {
    RETURN_IF_ERROR(Get(&result->label, input, "label"));
    RETURN_IF_ERROR(OptionallyGet(&result->label_details, input, "labelDetails"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    RETURN_IF_ERROR(OptionallyGet(&result->tags, input, "tags"));
    RETURN_IF_ERROR(OptionallyGet(&result->detail, input, "detail"));
    RETURN_IF_ERROR(OptionallyGet(&result->documentation, input, "documentation"));
    RETURN_IF_ERROR(OptionallyGet(&result->deprecated, input, "deprecated"));
    RETURN_IF_ERROR(OptionallyGet(&result->preselect, input, "preselect"));
    RETURN_IF_ERROR(OptionallyGet(&result->sort_text, input, "sortText"));
    RETURN_IF_ERROR(OptionallyGet(&result->filter_text, input, "filterText"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_text, input, "insertText"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_text_format, input, "insertTextFormat"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_text_mode, input, "insertTextMode"));
    RETURN_IF_ERROR(OptionallyGet(&result->text_edit, input, "textEdit"));
    RETURN_IF_ERROR(OptionallyGet(&result->additional_text_edits, input, "additionalTextEdits"));
    RETURN_IF_ERROR(OptionallyGet(&result->commit_characters, input, "commitCharacters"));
    RETURN_IF_ERROR(OptionallyGet(&result->command, input, "command"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionList* result) {
    RETURN_IF_ERROR(Get(&result->is_incomplete, input, "isIncomplete"));
    RETURN_IF_ERROR(Get(&result->items, input, "items"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CompletionResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, HoverParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Hover* result) {
    RETURN_IF_ERROR(Get(&result->contents, input, "contents"));
    RETURN_IF_ERROR(OptionallyGet(&result->range, input, "range"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpTriggerKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SignatureHelpTriggerKind: code was %d < 1", code));
    }
    if (code > 3) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse SignatureHelpTriggerKind: code was %d > 3", code));
    }
    *result = static_cast<SignatureHelpTriggerKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ParameterInformation* result) {
    RETURN_IF_ERROR(Get(&result->label, input, "label"));
    RETURN_IF_ERROR(OptionallyGet(&result->documentation, input, "documentation"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureInformation* result) {
    RETURN_IF_ERROR(Get(&result->label, input, "label"));
    RETURN_IF_ERROR(OptionallyGet(&result->documentation, input, "documentation"));
    RETURN_IF_ERROR(OptionallyGet(&result->parameters, input, "parameters"));
    RETURN_IF_ERROR(OptionallyGet(&result->active_parameter, input, "activeParameter"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelp* result) {
    RETURN_IF_ERROR(Get(&result->signatures, input, "signatures"));
    RETURN_IF_ERROR(OptionallyGet(&result->active_signature, input, "activeSignature"));
    RETURN_IF_ERROR(OptionallyGet(&result->active_parameter, input, "activeParameter"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpContext* result) {
    RETURN_IF_ERROR(Get(&result->trigger_kind, input, "triggerKind"));
    RETURN_IF_ERROR(OptionallyGet(&result->trigger_character, input, "triggerCharacter"));
    RETURN_IF_ERROR(Get(&result->is_retrigger, input, "isRetrigger"));
    RETURN_IF_ERROR(OptionallyGet(&result->active_signature_help, input, "activeSignatureHelp"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SignatureHelpParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(OptionallyGet(&result->context, input, "context"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeclarationParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DeclarationResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DefinitionParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DefinitionResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TypeDefinitionParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, TypeDefinitionResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ImplementationParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ImplementationResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceContext* result) {
    RETURN_IF_ERROR(Get(&result->include_declaration, input, "includeDeclaration"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->context, input, "context"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ReferenceResult* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->result));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlightParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlightKind* result) {
    ASSIGN_OR_RETURN(int32_t code, JsonToInt32(input));
    if (code < 1) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DocumentHighlightKind: code was %d < 1", code));
    }
    if (code > 3) {
        return absl::InternalError(
            absl::StrFormat("Failed to parse DocumentHighlightKind: code was %d > 3", code));
    }
    *result = static_cast<DocumentHighlightKind>(code);
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentHighlight* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentSymbolParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentSymbol* result) {
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    RETURN_IF_ERROR(OptionallyGet(&result->detail, input, "detail"));
    RETURN_IF_ERROR(Get(&result->kind, input, "kind"));
    RETURN_IF_ERROR(OptionallyGet(&result->tags, input, "tags"));
    RETURN_IF_ERROR(OptionallyGet(&result->deprecated, input, "deprecated"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->selection_range, input, "selectionRange"));
    RETURN_IF_ERROR(OptionallyGet(&result->children, input, "children"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionContext* result) {
    RETURN_IF_ERROR(Get(&result->diagnostics, input, "diagnostics"));
    RETURN_IF_ERROR(OptionallyGet(&result->only, input, "only"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeActionParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->context, input, "context"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeAction* result) {
    RETURN_IF_ERROR(Get(&result->title, input, "title"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    RETURN_IF_ERROR(OptionallyGet(&result->diagnostics, input, "diagnostics"));
    RETURN_IF_ERROR(OptionallyGet(&result->is_preferred, input, "isPreferred"));
    JSON disabled = input.get("disabled", -1);
    if (disabled.isObject()) {
        RETURN_IF_ERROR(OptionallyGet(&result->disabled, disabled, "reason"));
    }
    RETURN_IF_ERROR(OptionallyGet(&result->edit, input, "edit"));
    RETURN_IF_ERROR(OptionallyGet(&result->command, input, "command"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLensParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CodeLens* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->command, input, "command"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentLinkParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentLink* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(OptionallyGet(&result->target, input, "target"));
    RETURN_IF_ERROR(OptionallyGet(&result->tooltip, input, "tooltip"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentColorParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Color* result) {
    RETURN_IF_ERROR(Get(&result->red, input, "red"));
    RETURN_IF_ERROR(Get(&result->green, input, "green"));
    RETURN_IF_ERROR(Get(&result->blue, input, "blue"));
    RETURN_IF_ERROR(Get(&result->alpha, input, "alpha"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ColorInformation* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->color, input, "color"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ColorPresentationParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->color, input, "color"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, ColorPresentation* result) {
    RETURN_IF_ERROR(Get(&result->label, input, "label"));
    RETURN_IF_ERROR(OptionallyGet(&result->text_edit, input, "textEdit"));
    RETURN_IF_ERROR(OptionallyGet(&result->additional_text_edits, input, "additionalTextEdits"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FormattingOptions* result) {
    RETURN_IF_ERROR(Get(&result->tab_size, input, "tabSize"));
    RETURN_IF_ERROR(Get(&result->insert_spaces, input, "insertSpaces"));
    RETURN_IF_ERROR(OptionallyGet(&result->trim_trailing_whitespace, input, "trimTrailingWhitespace"));
    RETURN_IF_ERROR(OptionallyGet(&result->insert_final_newline, input, "insertFinalNewline"));
    RETURN_IF_ERROR(OptionallyGet(&result->trim_final_newlines, input, "trimFinalNewlines"));
    RETURN_IF_ERROR(FromJSON(input, &result->extra_properties));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentFormattingParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->options, input, "options"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentRangeFormattingParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->options, input, "options"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, DocumentOnTypeFormattingParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(Get(&result->ch, input, "ch"));
    RETURN_IF_ERROR(Get(&result->options, input, "options"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, RenameParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(Get(&result->new_name, input, "newName"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PrepareRenameParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, PrepareRenameResult* result) {
    result->range = absl::nullopt;
    result->placeholder = absl::nullopt;
    result->default_behavior = absl::nullopt;
    if (input == JSON()) {
        return absl::OkStatus();
    } else if (input.isObject()) {
        JSON range = input.get("range", -1);
        JSON placeholder = input.get("placeholder", -1);
        JSON default_behavior = input.get("defaultBehavior", -1);
        if (range.isObject() && placeholder.isString()) {
            RDSS_CHECK(default_behavior.isIntegral());
            Range range_val;
            std::string placeholder_val;
            RETURN_IF_ERROR(FromJSON(range, &range_val));
            RETURN_IF_ERROR(FromJSON(placeholder, &placeholder_val));
            result->range = range_val;
            result->placeholder = placeholder_val;
        } else if (default_behavior.isBool()) {
            RDSS_CHECK(range.isIntegral());
            RDSS_CHECK(placeholder.isIntegral());
            bool default_behavior_val;
            RETURN_IF_ERROR(FromJSON(default_behavior, &default_behavior_val));
            result->default_behavior = default_behavior_val;
        } else {
            RDSS_CHECK(range.isIntegral());
            RDSS_CHECK(placeholder.isIntegral());
            RDSS_CHECK(default_behavior.isIntegral());
            Range range_val;
            RETURN_IF_ERROR(FromJSON(input, &range_val));
            result->range = range_val;
        }
    } else {
        return absl::InternalError("Failed to parse PrepareRenameResult");
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRangeParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRangeKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "comment") {
        *result = FoldingRangeKind::Comment;
    } else if (input == "imports") {
        *result = FoldingRangeKind::Imports;
    } else if (input == "region") {
        *result = FoldingRangeKind::Region;
    } else {
        return absl::InternalError(
            "Invalid string for FoldingRangeKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, FoldingRange* result) {
    RETURN_IF_ERROR(Get(&result->start_line, input, "startLine"));
    RETURN_IF_ERROR(OptionallyGet(&result->start_character, input, "startCharacter"));
    RETURN_IF_ERROR(Get(&result->end_line, input, "endLine"));
    RETURN_IF_ERROR(OptionallyGet(&result->end_character, input, "endCharacter"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SelectionRangeParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->positions, input, "positions"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SelectionRange* result) {
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    result->parent = absl::nullopt;
    auto parent_json = input.get("parent", -1);
    if (!parent_json.isIntegral()) {
        SelectionRange* parent = new SelectionRange();
        RETURN_IF_ERROR(FromJSON(parent_json, parent));
        result->parent = parent;
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyPrepareParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyItem* result) {
    RETURN_IF_ERROR(Get(&result->name, input, "name"));
    RETURN_IF_ERROR(Get(&result->kind, input, "kind"));
    RETURN_IF_ERROR(OptionallyGet(&result->tags, input, "tags"));
    RETURN_IF_ERROR(OptionallyGet(&result->detail, input, "detail"));
    RETURN_IF_ERROR(Get(&result->uri, input, "uri"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    RETURN_IF_ERROR(Get(&result->selection_range, input, "selectionRange"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyIncomingCallsParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->item, input, "item"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyIncomingCall* result) {
    RETURN_IF_ERROR(Get(&result->from, input, "from"));
    RETURN_IF_ERROR(Get(&result->from_ranges, input, "fromRanges"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyOutgoingCallsParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->item, input, "item"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, CallHierarchyOutgoingCall* result) {
    RETURN_IF_ERROR(Get(&result->to, input, "to"));
    RETURN_IF_ERROR(Get(&result->from_ranges, input, "fromRanges"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokens* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->result_id, input, "resultId"));
    RETURN_IF_ERROR(Get(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensDeltaParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->previous_result_id, input, "previousResultId"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensEdit* result) {
    RETURN_IF_ERROR(Get(&result->start, input, "start"));
    RETURN_IF_ERROR(Get(&result->delete_count, input, "deleteCount"));
    RETURN_IF_ERROR(OptionallyGet(&result->data, input, "data"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensDelta* result) {
    RETURN_IF_ERROR(OptionallyGet(&result->result_id, input, "resultId"));
    RETURN_IF_ERROR(Get(&result->edits, input, "edits"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, SemanticTokensRangeParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    RETURN_IF_ERROR(Get(&result->text_document, input, "textDocument"));
    RETURN_IF_ERROR(Get(&result->range, input, "range"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LinkedEditingRangeParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, LinkedEditingRanges* result) {
    RETURN_IF_ERROR(Get(&result->ranges, input, "ranges"));
    RETURN_IF_ERROR(OptionallyGet(&result->word_pattern, input, "wordPattern"));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MonikerParams* result) {
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_tdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_wdpp));
    RETURN_IF_ERROR(FromJSON(input, &result->underlying_prp));
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, UniquenessLevel* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "document") {
        *result = UniquenessLevel::Document;
    } else if (input == "project") {
        *result = UniquenessLevel::Project;
    } else if (input == "group") {
        *result = UniquenessLevel::Group;
    } else if (input == "scheme") {
        *result = UniquenessLevel::Scheme;
    } else if (input == "global") {
        *result = UniquenessLevel::Global;
    } else {
        return absl::InternalError(
            "Invalid string for UniquenessLevel: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, MonikerKind* result) {
    if (!input.isString()) {
        return absl::InternalError("Expected string, got " + input.asString());
    }
    if (input == "import") {
        *result = MonikerKind::Import;
    } else if (input == "export") {
        *result = MonikerKind::Export;
    } else if (input == "local") {
        *result = MonikerKind::Local;
    } else {
        return absl::InternalError(
            "Invalid string for MonikerKind: " + input.asString());
    }
    return absl::OkStatus();
}

absl::Status FromJSON(const JSON& input, Moniker* result) {
    RETURN_IF_ERROR(Get(&result->scheme, input, "scheme"));
    RETURN_IF_ERROR(Get(&result->identifier, input, "identifier"));
    RETURN_IF_ERROR(Get(&result->unique, input, "unique"));
    RETURN_IF_ERROR(OptionallyGet(&result->kind, input, "kind"));
    return absl::OkStatus();
}

}  // namespace lsp

}  // namespace rdss
