#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>
#include <absl/strings/ascii.h>

#include "../src/filesystem.hpp"
#include "../src/ghd.hpp"

absl::optional<rdss::Hypergraph<std::string>>
parse_hg(absl::string_view input) {
    rdss::Hypergraph<std::string> graph;

    std::vector<absl::string_view> lines = absl::StrSplit(input, '\n');

    for (const auto& line : lines) {
        if (line.empty()) { continue; }

        std::vector<absl::string_view> broken_on_open_paren =
          absl::StrSplit(absl::StripAsciiWhitespace(line), '(');
        if (broken_on_open_paren.size() != 2) {
            return absl::nullopt;
        }
        std::vector<absl::string_view> broken_on_close_paren =
          absl::StrSplit(broken_on_open_paren[1], ')');
        if (broken_on_close_paren.size() != 2) {
            return absl::nullopt;
        }
        std::vector<absl::string_view> vertices_with_whitespace =
          absl::StrSplit(broken_on_close_paren[0], ',');

        rdss::HyperedgeId e = graph.AddEdge();

        for (const auto& vertex_with_whitespace
             : vertices_with_whitespace) {
            absl::string_view vertex =
              absl::StripAsciiWhitespace(vertex_with_whitespace);
            graph.AddVertex(std::string(vertex));
            if (!graph.AddVertexToEdge(std::string(vertex), e)) {
                return absl::nullopt;
            }
        }
    }

    return graph;
}

TEST(FHD, Parser) {
    absl::StatusOr<std::string> graph_str = rdss::GetFileContents("../test/graphs/c4.hg");
    if (!graph_str.ok()) {
        std::cerr << graph_str.status();
        FAIL();
    }
    auto parsed_graph = parse_hg(*graph_str);
    EXPECT_TRUE(parsed_graph.has_value());

    auto set0 = { "v1", "v2" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(0).value(), set0);

    auto set1 = { "v1", "v3" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(1).value(), set1);

    auto set2 = { "v1", "v4" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(2).value(), set2);

    auto set3 = { "v2", "v3" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(3).value(), set3);

    auto set4 = { "v2", "v4" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(4).value(), set4);

    auto set5 = { "v3", "v4" };
    EXPECT_EQ(parsed_graph->VerticesInEdge(5).value(), set5);

    EXPECT_EQ(parsed_graph->NumEdges(), 6);
}
