#include "ast.hpp"
#include "ghd.hpp"
#include "macros.hpp"

#include <cstdlib>
#include <iostream>

#include <absl/memory/memory.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace rdss {

void TestGHD() {
    Hypergraph<int> graph;

    for (int i = 1; i <= 10; i++) {
        graph.AddVertex(i);
    }

    auto e1 = graph.AddEdge();
    graph.AddVertexToEdge(1, e1);
    graph.AddVertexToEdge(2, e1);
    graph.AddVertexToEdge(9, e1);

    auto e2 = graph.AddEdge();
    graph.AddVertexToEdge(2, e2);
    graph.AddVertexToEdge(3, e2);
    graph.AddVertexToEdge(10, e2);

    auto e3 = graph.AddEdge();
    graph.AddVertexToEdge(3, e3);
    graph.AddVertexToEdge(4, e3);

    auto e4 = graph.AddEdge();
    graph.AddVertexToEdge(4, e4);
    graph.AddVertexToEdge(5, e4);
    graph.AddVertexToEdge(9, e4);

    auto e5 = graph.AddEdge();
    graph.AddVertexToEdge(5, e5);
    graph.AddVertexToEdge(6, e5);
    graph.AddVertexToEdge(10, e5);

    auto e6 = graph.AddEdge();
    graph.AddVertexToEdge(6, e6);
    graph.AddVertexToEdge(7, e6);
    graph.AddVertexToEdge(9, e6);

    auto e7 = graph.AddEdge();
    graph.AddVertexToEdge(7, e7);
    graph.AddVertexToEdge(8, e7);
    graph.AddVertexToEdge(10, e7);

    auto e8 = graph.AddEdge();
    graph.AddVertexToEdge(1, e8);
    graph.AddVertexToEdge(8, e8);

    // graph.AddVertex(1);
    // graph.AddVertex(2);
    // graph.AddVertex(3);
    //
    // auto e1 = graph.AddEdge();
    // graph.AddVertexToEdge(1, e1);
    // graph.AddVertexToEdge(2, e1);
    //
    // auto e2 = graph.AddEdge();
    // graph.AddVertexToEdge(2, e2);
    // graph.AddVertexToEdge(3, e2);
    //
    // auto e3 = graph.AddEdge();
    // graph.AddVertexToEdge(3, e3);
    // graph.AddVertexToEdge(1, e3);

    absl::StatusOr<FHD<int32_t>> fhd = ComputeFHD(graph);
    std::cerr << "FHW of graph is: ";
    if (fhd.ok()) {
        std::cerr << fhd->fhw << "\n";
    } else {
        std::cerr << "unknown\n";
    }
}

void TestYannakakis() {
    RelationFactory fac;
    Tree<Relation*, JoinOn> tree {
        fac.Make<RelationReference>("A", 2),
        {
            { { fac.Make<RelationReference>("B", 2), {} }, {{0, 0}} },
            { { fac.Make<RelationReference>("C", 2), {} }, {{1, 0}} }
        }
    };
    std::cerr << Yannakakis(&fac, tree)->ToString() << "\n";
}

absl::Status RealMain() {
    rdss::DataStructure example("Example");
    // example.members.push_back(
    //     Member {
    //         VarName("foo"),
    //         absl::make_unique<TypeVector>(absl::make_unique<TypeInt>())
    //     });
    std::cout << example.ToCpp();

    TestGHD();
    TestYannakakis();

    return absl::OkStatus();
}

}  // namespace rdss

int main() {
    absl::Status result = rdss::RealMain();
    if(!result.ok()) {
        std::cerr << result << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
