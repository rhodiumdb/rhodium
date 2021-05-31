#ifndef RDSS_GHD_H_
#define RDSS_GHD_H_

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#include <z3++.h>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <absl/types/optional.h>

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

using HyperedgeId = int32_t;

template<typename V>
class Hypergraph {
public:
    void AddVertex(const V& vertex) {
        this->vertex_to_hyperedge[vertex];
    }

    HyperedgeId AddEdge() {
        HyperedgeId result = this->hyperedges.size();
        this->hyperedges.push_back(Hyperedge { absl::flat_hash_set<V>() });
        return result;
    }

    bool AddVertexToEdge(const V& vertex, HyperedgeId edge) {
        if (!this->vertex_to_hyperedge.contains(vertex)) {
            return false;
        }
        if (edge >= this->hyperedges.size()) {
            return false;
        }
        this->vertex_to_hyperedge[vertex].insert(edge);
        this->hyperedges[edge].vertices.insert(vertex);
        return true;
    }

    absl::optional<absl::flat_hash_set<HyperedgeId>>
    EdgesIncidentOnVertex(const V& vertex) const {
        if (!this->vertex_to_hyperedge.contains(vertex)) {
            return absl::nullopt;
        }
        return this->vertex_to_hyperedge.at(vertex);
    }

    absl::optional<absl::flat_hash_set<V>>
    VerticesInEdge(HyperedgeId edge) const {
        if (edge >= this->hyperedges.size()) {
            return absl::nullopt;
        }
        return hyperedges.at(edge).vertices;
    }

    absl::flat_hash_set<V> AllVertices() const {
        absl::flat_hash_set<V> result;
        for (const auto& [key, value] : vertex_to_hyperedge) {
            result.insert(key);
        }
        return result;
    }

    std::vector<HyperedgeId> AllEdges() const {
        std::vector<HyperedgeId> result;
        for (int32_t i = 0; i < hyperedges.size(); i++) {
            if (!hyperedges.at(i).vertices.empty()) {
                result.push_back(i);
            }
        }
        return result;
    }

    int32_t NumVertices() const {
        return vertex_to_hyperedge.size();
    }

    int32_t NumEdges() const {
        return hyperedges.size();
    }

    void DeleteVertex(const V& vertex) {
        if (auto edges = EdgesIncidentOnVertex(vertex)) {
            for (const auto& edge : *edges) {
                hyperedges.at(edge).vertices.erase(vertex);
            }
            vertex_to_hyperedge.erase(vertex);
        }
    }

    void DeleteEdge(HyperedgeId edge) {
        if (auto vertices = VerticesInEdge(edge)) {
            for (const auto& vertex : *vertices) {
                vertex_to_hyperedge.at(vertex).erase(edge);
            }
            hyperedges.at(edge) = Hyperedge { {} };
        }
    }

    void CollectGarbage() {
        // FIXME
    }

    std::vector<Hypergraph> ConnectedComponents() const {
        return std::vector<Hypergraph>();
    }

private:
    struct Hyperedge {
        absl::flat_hash_set<V> vertices;
    };

    std::vector<Hyperedge> hyperedges;
    absl::flat_hash_map<V, absl::flat_hash_set<HyperedgeId>> vertex_to_hyperedge;
};

template<typename V>
struct Tree {
    V value;
    std::vector<Tree> children;
};

template<typename T>
std::vector<std::vector<T>>
AllSubsetsOfSize(int32_t i, const std::vector<T>& vec) {
    return std::vector<std::vector<T>>(); // FIXME
}

template<typename T>
absl::flat_hash_set<T> SetIntersection(const absl::flat_hash_set<T>& x,
                                       const absl::flat_hash_set<T>& y) {
    return absl::flat_hash_set<T>(); // FIXME
}

int32_t HypertreeWidth(const Tree<absl::flat_hash_set<HyperedgeId>>& tree) {
    int32_t result = static_cast<int32_t>(tree.value.size());
    std::vector<const Tree<absl::flat_hash_set<HyperedgeId>>*> active;
    active.push_back(&tree);
    while (active.empty()) {
        auto t = active.back();
        active.pop_back();
        result = std::min(result, static_cast<int32_t>(t->value.size()));
        for (const auto& child : t->children) {
            active.push_back(&child);
        }
    }
    return result;
}

template<typename V>
void EnumerateGHDs(const Hypergraph<V>& hypergraph, int32_t width) {
    auto all_vertices = hypergraph.AllVertices();
    std::vector<V> vertices_vec(all_vertices.begin(), all_vertices.end());
    std::sort(vertices_vec.begin(), vertices_vec.end());

    auto all_edges = hypergraph.AllEdges();
    std::vector<HyperedgeId> edges_vec(all_edges.begin(), all_edges.end());
    std::sort(edges_vec.begin(), edges_vec.end());

    int32_t num_vertices = vertices_vec.size();
    int32_t num_edges = edges_vec.size();

    absl::flat_hash_map<V, int32_t> vertex_to_int;
    for (int32_t i = 0; i < num_vertices; i++) {
        vertex_to_int[vertices_vec[i]] = i;
    }

    absl::flat_hash_map<HyperedgeId, int32_t> edge_to_int;
    for (int32_t i = 0; i < num_edges; i++) {
        edge_to_int[edges_vec[i]] = i;
    }

    z3::context c;
    z3::solver s(c);

    absl::optional<z3::expr> o_star[num_vertices][num_vertices];
    absl::optional<z3::expr> e_star[num_vertices][num_vertices];
    absl::optional<z3::expr> a[num_vertices][num_vertices];
    absl::optional<z3::expr> b[num_vertices][num_vertices];
    // absl::optional<z3::expr> p[num_vertices][num_vertices];
    absl::optional<z3::expr> w[num_vertices][num_edges];

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (i < j) {
                std::stringstream ss;
                ss << "ostar_" << i << "_" << j;
                std::string o_star_str = ss.str();

                o_star[i][j] = c.bool_const(o_star_str.c_str());

                std::string e_star_str = o_star_str;
                e_star_str[0] = 'e';

                e_star[i][j] = c.bool_const(e_star_str.c_str());
            }

            {
                std::stringstream ss;
                ss << "a_" << i << "_" << j;
                std::string a_str = ss.str();
                a[i][j] = c.bool_const(a_str.c_str());
            }

            {
                std::stringstream ss;
                ss << "b_" << i << "_" << j;
                std::string b_str = ss.str();
                b[i][j] = c.bool_const(b_str.c_str());
            }

            // {
            //     std::stringstream ss;
            //     ss << "p_" << i << "_" << j;
            //     std::string p_str = ss.str();
            //     p[i][j] = c.bool_const(p_str.c_str());
            // }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t e = 0; e < num_edges; e++) {
            std::stringstream ss;
            ss << "w_" << i << "_" << e;
            std::string w_str = ss.str();
            w[i][e] = c.int_const(w_str.c_str());
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (j < i) {
                o_star[i][j] = !o_star[j][i].value();
                e_star[i][j] = e_star[j][i].value();
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            for (int32_t k = 0; k < num_vertices; k++) {
                if ((i != j) && (j != k) && (k != i)) {
                    { // Transitivity for o_star
                        s.add(!o_star[i][j].value()
                              || !o_star[j][k].value()
                              || o_star[i][k].value());
                    }

                    { // Transitivity for e_star
                        s.add(!e_star[i][j].value()
                              || !e_star[j][k].value()
                              || e_star[i][k].value());
                    }
                }
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (i != j) {
                // Ensure that ≺ is a topological ordering of D
                s.add(!o_star[i][j].value() || !a[j][i].value());
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        // Constraints for bᵢⱼ
        s.add(b[i][i].value());
        for (int32_t j = 0; j < num_vertices; j++) {
            if (i != j) {
                s.add((a[i][j].value() || e_star[i][j].value())
                      == b[i][j].value());
            }
        }
    }

    for (HyperedgeId edge : hypergraph.AllEdges()) {
        auto vertices = hypergraph.VerticesInEdge(edge).value();
        for (V x : vertices) {
            for (V y : vertices) {
                // Condition O1
                int32_t i = vertex_to_int.at(x);
                int32_t j = vertex_to_int.at(y);
                if (i != j) {
                    s.add(!o_star[i][j].value() || a[i][j].value());
                }
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (i != j) {
                // Condition O2
                s.add(!o_star[i][j].value()
                      || !e_star[i][j].value()
                      || a[i][j].value());
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            for (int32_t k = 0; k < num_vertices; k++) {
                if ((i != j) && (j != k) && (k != i)) {
                    // Condition O3
                    s.add(!o_star[j][k].value()
                          || !a[i][j].value()
                          || !a[i][k].value()
                          || a[j][k].value());

                    // Condition O4
                    s.add(!o_star[i][j].value()
                          || !o_star[j][k].value()
                          || !e_star[i][j].value()
                          || !a[j][k].value()
                          || a[i][k].value());
                }
            }
        }
    }

    {
        // Condition O5

        for (int32_t i = 0; i < num_vertices; i++) {
            for (int32_t j = 0; j < num_vertices; j++) {
                if (i != j) {
                    absl::optional<z3::expr> sum;
                    for (int32_t e = 0; e < num_edges; e++) {
                        if (hypergraph.VerticesInEdge(edges_vec[e]).value()
                            .contains(vertices_vec[j])) {
                            sum = sum
                                ? *sum + w[i][e].value()
                                : w[i][e].value();
                        }
                    }
                    if (sum) {
                        s.add(!a[i][j].value() || (sum.value() >= 1));
                    }
                    else {
                        s.add(!a[i][j].value());
                    }
                }
            }
        }

        for (int32_t i = 0; i < num_vertices; i++) {
            absl::optional<z3::expr> sum;
            for (int32_t e = 0; e < num_edges; e++) {
                if (hypergraph.VerticesInEdge(edges_vec[e]).value()
                    .contains(vertices_vec[i])) {
                    sum = sum ? *sum + w[i][e].value() : w[i][e].value();
                }
            }
            if (sum) { s.add(sum.value() >= 1); }
        }

        for (int32_t i = 0; i < num_vertices; i++) {
            absl::optional<z3::expr> sum;
            for (int32_t e = 0; e < num_edges; e++) {
                sum = sum ? *sum + w[i][e].value() : w[i][e].value();
            }
            if (sum) { s.add(sum.value() <= width); }
        }
    }

    z3::set_param("pp.decimal", true);

    switch(s.check()) {
    case z3::unsat:
        std::cerr << "Graph had no GHD of width " << width << "\n"; break;
    case z3::sat:
        std::cerr
            << "Graph had GHD of width " << width << "\n"
            << "Model:\n" << s.get_model() << "\n";
        break;
    case z3::unknown:
        std::cerr << "Graph was too confusing for Z3\n"; break;
    }
}

// template<typename V>
// std::vector<Tree<absl::flat_hash_set<HyperedgeId>>>
// EnumerateGHDs(const Hypergraph<V>& hypergraph,
//               const absl::flat_hash_set<V>& attrs) {
//     using Hypertree = Tree<absl::flat_hash_set<HyperedgeId>>;
//     std::vector<Hypertree> result;
//     for (int32_t i = 1; i < hypergraph.NumVertices(); i++) {
//         for (const std::vector<HyperedgeId>& subset_vec
//                  : AllSubsetsOfSize(i, hypergraph.AllEdges())) {
//             absl::flat_hash_set<HyperedgeId>
//                 subset(subset_vec.begin(), subset_vec.end());
//
//             // Define ε \ S
//             absl::flat_hash_set<HyperedgeId> edges_minus_subset(
//                 hypergraph.AllEdges().begin(),
//                 hypergraph.AllEdges().end());
//             for (HyperedgeId edge : subset) {
//                 edges_minus_subset.erase(edge);
//             }
//
//             // Define S.attributes
//             absl::flat_hash_set<V> subset_attrs;
//             for (HyperedgeId edge : subset) {
//                 for (V vertex : hypergraph.VerticesInEdge(edge).value()) {
//                     subset_attrs.insert(vertex);
//                 }
//             }
//
//             // Define H \ S
//             Hypergraph graph_minus_subset = hypergraph;
//             for (HyperedgeId edge : subset) {
//                 graph_minus_subset.DeleteEdge(edge);
//             }
//             for (const V& vertex : subset_attrs) {
//                 graph_minus_subset.DeleteVertex(vertex);
//             }
//
//             // For any R ∈ ε \ S:
//             //     If R.attributes ∩ attributes ⊈ S.attributes:
//             //         return []
//             for (HyperedgeId r : edges_minus_subset) {
//                 auto r_attrs = hypergraph.VerticesInEdge(r).value();
//                 for (V vertex : SetIntersection(r_attrs, attrs)) {
//                     if (!subset_attrs.contains(vertex)) {
//                         return std::vector<Hypertree>();
//                     }
//                 }
//             }
//
//             // Find connected components in ε \ S, ignoring attributes in S
//             // For every connected component Cᵢ:
//             //     subtrees{Cᵢ} = enumerateGHDs(Cᵢ, S.attributes)
//             std::vector<std::vector<Hypertree>> subtrees_c;
//             for (const auto& cc : graph_minus_subset.ConnectedComponents()) {
//                 subtrees_c.push_back(EnumerateGHDs(cc, subset_attrs));
//             }
//
//             // subtrees = {{c₁, c₂, …} | each cᵢ ∈ subtrees{Cᵢ}}
//             std::vector<Hypertree> subtrees;
//             // for (const auto& subtree_c : subtrees_c) {
//             //     for (const auto& subtree : subtree_c) {
//             //         subtrees.push_back(subtree);
//             //     }
//             // }
//
//             for (int32_t j = 2; j < subtrees_c.size(); j++) {
//                 for (const auto& subtree_subset
//                          : AllSubsetsOfSize(j, subtrees_c)) {
//                     subtrees.push_back(subtree_subset);
//                 }
//             }
//
//             // result = []
//             // For each s in subtrees:
//             //     result.append(S with s as children)
//             // return result
//             for (absl::flat_hash_set<std::vector<Hypertree>> s : subtrees) {
//                 result.push_back(Hypertree { subset, s });
//             }
//         }
//     }
//     return result;
// }

// template<typename V>
// struct Tree {
//     V value;
//     std::vector<Tree> children;
// };
//
// template<typename V>
// struct GeneralizedHypertreeDecomposition {
//     using ProjectedAttr = absl::flat_hash_set<V>;
//     using Bag = absl::flat_hash_set<ProjectedAttr>;
//
//     Tree<Bag> tree;
//
//     bool IsValid(const Hypergraph& hypergraph) {
//         for (const auto& edge)
//     }
// };

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_GHD_H_
