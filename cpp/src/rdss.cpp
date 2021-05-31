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
    assert(AddVertexToEdge(1, e1));
    assert(AddVertexToEdge(2, e1));
    assert(AddVertexToEdge(9, e1));

    auto e2 = graph.AddEdge();
    assert(AddVertexToEdge(2, e2));
    assert(AddVertexToEdge(3, e2));
    assert(AddVertexToEdge(10, e2));

    auto e3 = graph.AddEdge();
    assert(AddVertexToEdge(3, e3));
    assert(AddVertexToEdge(4, e3));

    auto e4 = graph.AddEdge();
    assert(AddVertexToEdge(4, e4));
    assert(AddVertexToEdge(5, e4));
    assert(AddVertexToEdge(9, e4));

    auto e5 = graph.AddEdge();
    assert(AddVertexToEdge(5, e5));
    assert(AddVertexToEdge(6, e5));
    assert(AddVertexToEdge(10, e5));

    auto e6 = graph.AddEdge();
    assert(AddVertexToEdge(6, e6));
    assert(AddVertexToEdge(7, e6));
    assert(AddVertexToEdge(9, e6));

    auto e7 = graph.AddEdge();
    assert(AddVertexToEdge(7, e7));
    assert(AddVertexToEdge(8, e7));
    assert(AddVertexToEdge(10, e7));

    auto e8 = graph.AddEdge();
    assert(AddVertexToEdge(1, e8));
    assert(AddVertexToEdge(8, e8));

    // for (Tree<absl::flat_hash_set<HyperedgeId>> ghd
    //          : EnumerateGHDs(graph, graph.AllVertices())) {
    //     std::cerr << "DEBUG: " << HypertreeWidth(ghd) << "\n";
    // }

    EnumerateGHDs(graph, 0);
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
