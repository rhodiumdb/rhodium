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

#include "ast.hpp"

#include <sstream>

#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>

namespace rdss {

std::string Member::ToCpp(FreshVariableSource* source) const {
    return absl::StrFormat("%s %s;\n", this->type->ToCpp(), this->name.ToCpp());
}

// <type> <name>(<arg1>, <arg2>, ..., <argN>)
std::string Method::ToCpp(FreshVariableSource* source) const {
    std::vector<std::string> args_vec;
    for (int32_t i = 0; i < this->arguments.size(); i++) {
        args_vec.push_back(absl::StrCat(
                               this->arguments[i].second->ToCpp(),
                               " ",
                               this->arguments[i].first.ToCpp()));
    }

    auto args = absl::StrJoin(args_vec, ", ");

    std::string body;
    for (int32_t i = 0; i < this->body.size(); i++) {
        absl::StrAppend(&body, Indent(this->body[i]->ToCpp(source), 1), "\n");
    }

    return absl::StrFormat("void %s(%s) {\n%s}",
                           this->name.ToCpp(), args, body);
}

std::string DataStructure::ToCpp(FreshVariableSource* source) const {
    std::stringstream ss;
    if (!this->type_parameters.empty()) {
        ss << "template<";
        std::vector<std::string> params;
        for (const auto& ty_param : this->type_parameters) {
            params.push_back("typename " + ty_param.ToCpp());
        }
        ss << absl::StrJoin(params, ", ");
        ss << ">\n";
    }
    ss << "struct " << this->name << " {\n";
    for (const auto& member : this->members) {
        ss << Indent(member.ToCpp(source), 1);
    }
    for (const auto& method : this->methods) {
        ss << Indent(method.ToCpp(source), 1);
    }
    ss << "}\n";
    return ss.str();
}

}  // namespace rdss
