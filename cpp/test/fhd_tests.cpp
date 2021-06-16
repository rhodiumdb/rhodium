#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <absl/strings/ascii.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>

#include "../src/filesystem.hpp"
#include "../src/ghd.hpp"

absl::optional<rdss::Hypergraph<std::string>>
ParseHG(absl::string_view input) {
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

void TestGraph(
    const std::filesystem::path& hg_path,
    const std::filesystem::path& opt_path
) {
     absl::StatusOr<std::string> graph_str = rdss::GetFileContents(hg_path);
    if (!graph_str.ok()) {
        std::cerr << graph_str.status();
        FAIL();
    }
    auto parsed_graph = ParseHG(*graph_str);
    EXPECT_TRUE(parsed_graph.has_value());

    absl::StatusOr<std::string> fhw_result_str = rdss::GetFileContents(opt_path);
    if (!fhw_result_str.ok()) {
        std::cerr << fhw_result_str.status();
        FAIL();
    }
    double fhw_opt;
    auto parsed_fhw_result = absl::SimpleAtod(fhw_result_str.value(), &fhw_opt);
    EXPECT_TRUE(parsed_fhw_result);

    auto computed_fhw = rdss::ComputeFHD(parsed_graph.value());
    EXPECT_TRUE(computed_fhw.ok());
    EXPECT_EQ(computed_fhw->fhw, fhw_opt);

}

TEST(FHD, c4) {
    TestGraph("../test/graphs/c4.hg", "../test/graphs/c4.opt");
}

TEST(FHD, triangle) {
    TestGraph("../test/graphs/triangle.hg", "../test/graphs/triangle.opt");
}

TEST(FHD, imdb_q13a) {
    TestGraph("../test/graphs/imdb-q13a.hg", "../test/graphs/imdb-q13a.opt");
}

TEST(FHD, tpch_manual_q10_min) {
    TestGraph("../test/graphs/tpch-manual-q10-min.hg",
              "../test/graphs/tpch-manual-q10-min.opt");
}

TEST(FHD, tpch_manual_q10) {
    TestGraph("../test/graphs/tpch-manual-q10.hg",
              "../test/graphs/tpch-manual-q10.opt");
}

TEST(FHD, tpch_synthetic_q5) {
    TestGraph("../test/graphs/tpch-synthetic-q5.hg",
              "../test/graphs/tpch-synthetic-q5.opt");
}
