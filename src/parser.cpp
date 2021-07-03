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

#include "parser.hpp"

#include <sstream>

#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

extern "C" const TSLanguage* tree_sitter_rhodium(void);

namespace rdss {

absl::StatusOr<TSTree*> RunParser(const std::string& code) {
    const TSLanguage* language = tree_sitter_rhodium();
    TSParser* parser = ts_parser_new();
    if (!ts_parser_set_language(parser, language)) {
        return absl::InternalError(
            "Version mismatch between tree-sitter version used to generate "
            "the grammar and the version used by rhodium.");
    }
    return ts_parser_parse_string(parser, nullptr, code.c_str(), code.size());
}

std::string PrintNode(const char* file, TSNode node) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    absl::string_view sliced(file + start, end - start);

    if (ts_node_named_child_count(node) == 0) {
        return absl::StrFormat("(%s [%s])", ts_node_type(node), sliced);
    }

    std::vector<std::string> children;

    for (int32_t i = 0; i < ts_node_named_child_count(node); i++) {
        children.push_back(PrintNode(file, ts_node_named_child(node, i)));
    }

    return absl::StrFormat("(%s %s)",
                           ts_node_type(node),
                           absl::StrJoin(children, " "));
}

std::string PrintTree(const char* file, const TSTree* tree) {
    return PrintNode(file, ts_tree_root_node(tree));
}

}  // namespace rdss
