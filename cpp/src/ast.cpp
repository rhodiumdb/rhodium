#include "ast.hpp"

#include <sstream>

#include <absl/strings/str_join.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>

namespace rdss {

static std::string Indent(std::string str, int32_t n = 1) {
    std::vector<absl::string_view> lines = absl::StrSplit(str, '\n');
    std::string indent;
    indent.resize(4 * n, ' ');
    std::stringstream ss;
    for (auto line : lines) {
        if (!line.empty()) {
            ss << indent << line << "\n";
        }
    }
    return ss.str();
}

std::string Member::ToCpp() const {
    return absl::StrCat(this->type->ToCpp(), " ", this->name.ToCpp(), ";\n");
}

std::string Method::ToCpp() const {
    return "";
}

std::string DataStructure::ToCpp() const {
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
        ss << Indent(member.ToCpp());
    }
    for (const auto& method : this->methods) {
        ss << Indent(method.ToCpp());
    }
    ss << "}\n";
    return ss.str();
}

}  // namespace rdss
