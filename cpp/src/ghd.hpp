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
        return this->hyperedges.at(edge).vertices;
    }

    absl::flat_hash_set<V> AllVertices() const {
        absl::flat_hash_set<V> result;
        for (const auto& [key, value] : this->vertex_to_hyperedge) {
            result.insert(key);
        }
        return result;
    }

    std::vector<HyperedgeId> AllEdges() const {
        std::vector<HyperedgeId> result;
        for (int32_t i = 0; i < this->hyperedges.size(); i++) {
            if (!this->hyperedges.at(i).vertices.empty()) {
                result.push_back(i);
            }
        }
        return result;
    }

    int32_t NumVertices() const {
        return this->vertex_to_hyperedge.size();
    }

    int32_t NumEdges() const {
        return this->hyperedges.size();
    }

    void DeleteVertex(const V& vertex) {
        if (auto edges = EdgesIncidentOnVertex(vertex)) {
            for (const auto& edge : *edges) {
                this->hyperedges.at(edge).vertices.erase(vertex);
            }
            this->vertex_to_hyperedge.erase(vertex);
        }
    }

    void DeleteEdge(HyperedgeId edge) {
        if (auto vertices = VerticesInEdge(edge)) {
            for (const auto& vertex : *vertices) {
                this->vertex_to_hyperedge.at(vertex).erase(edge);
            }
            this->hyperedges.at(edge) = Hyperedge { {} };
        }
    }

    void Print() const {
        std::cerr << "([";
        for (const Hyperedge& edge : this->hyperedges) {
            std::cerr << "{";
            for (const V& vertex : edge.vertices) {
                std::cerr << vertex << ",";
            }
            std::cerr << "},";
        }
        std::cerr << "],{";
        for (const auto& [vertex, edges] : this->vertex_to_hyperedge) {
            std::cerr << vertex << " -> {";
            for (HyperedgeId edge : edges) {
                std::cerr << edge << ",";
            }
            std::cerr << "},";
        }
        std::cerr << "})\n";
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

template<typename V>
absl::optional<double> ComputeFHW(const Hypergraph<V>& hypergraph) {
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
    // z3::solver s(c);
    z3::optimize s(c);

    z3::expr m = c.real_const("m");

    s.add(m >= 1);

    struct optional_expr {
        absl::optional<z3::expr> value_;
        optional_expr() : value_() {}
        optional_expr(const z3::expr& expr) : value_(expr) {}
        const z3::expr& value() { return value_.value(); }
    };

    optional_expr o_star[num_vertices][num_vertices];
    optional_expr a[num_vertices][num_vertices];
    optional_expr w[num_vertices][num_edges];

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (i < j) {
                std::stringstream ss;
                ss << "ostar_" << i << "_" << j;
                std::string o_star_str = ss.str();

                o_star[i][j] = c.bool_const(o_star_str.c_str());
            }

            {
                std::stringstream ss;
                ss << "a_" << i << "_" << j;
                std::string a_str = ss.str();
                a[i][j] = c.bool_const(a_str.c_str());
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t e = 0; e < num_edges; e++) {
            std::stringstream ss;
            ss << "w_" << i << "_" << e;
            std::string w_str = ss.str();
            w[i][e] = c.real_const(w_str.c_str());
            s.add(w[i][e].value() <= 1);
            s.add(w[i][e].value() >= 0);
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            if (j < i) {
                o_star[i][j] = !o_star[j][i].value();
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            for (int32_t k = 0; k < num_vertices; k++) {
                if ((i == j) || (i == k) || (j == k)) { continue; }
                s.add(!o_star[i][j].value()
                      || !o_star[j][k].value()
                      || o_star[i][k].value());
            }
        }
    }

    for (auto edge : edges_vec) {
        absl::flat_hash_set<V> vs = hypergraph.VerticesInEdge(edge).value();
        for (const auto& x : vs) {
            for (const auto& y : vs) {
                auto i = vertex_to_int.at(x);
                auto j = vertex_to_int.at(y);

                if (i >= j) { continue; }
                s.add(o_star[j][i].value() || a[i][j].value());
                s.add(o_star[i][j].value() || a[j][i].value());
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        for (int32_t j = 0; j < num_vertices; j++) {
            for (int32_t k = 0; k < num_vertices; k++) {
                if ((i == j) || (i == k) || (j >= k)) { continue; }
                s.add(!a[i][j].value()
                      || !a[i][k].value()
                      || o_star[k][j].value()
                      || a[j][k].value());
                s.add(!a[i][j].value()
                      || !a[i][k].value()
                      || o_star[j][k].value()
                      || a[k][j].value());
                s.add(!a[i][j].value()
                      || !a[i][k].value()
                      || a[j][k].value()
                      || a[k][j].value());
            }
        }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
        s.add(!a[i][i].value());
    }

    {
        for (int32_t i = 0; i < num_vertices; i++) {
            for (int32_t j = 0; j < num_vertices; j++) {
                if (i == j) { continue; }
                absl::optional<z3::expr> sum;
                for (int32_t e = 0; e < num_edges; e++) {
                    if (hypergraph.VerticesInEdge(edges_vec[e]).value()
                        .contains(vertices_vec[j])) {
                        sum = sum
                            ? *sum + w[i][e].value()
                            : w[i][e].value();
                    }
                }
                s.add(!a[i][j].value() || (sum.value() >= 1));
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
            s.add(sum.value() >= 1);
        }

        for (int32_t i = 0; i < num_vertices; i++) {
            absl::optional<z3::expr> sum;
            for (int32_t e = 0; e < num_edges; e++) {
                sum = sum ? *sum + w[i][e].value() : w[i][e].value();
            }
            s.add(sum.value() <= m);
        }
    }

    z3::set_param("pp.decimal", true);

    s.minimize(m);

    // std::cout << s.to_smt2() << "\n";

    if (s.check() == z3::sat) {
        auto model = s.get_model();
        absl::optional<double> fhw;
        for (int32_t i = 0; i < model.size(); i++) {
            if (model[i].name().str() == "m") {
                double temp;
                if (model.get_const_interp(model[i]).is_numeral(temp)) {
                    fhw = temp;
                }
            }
        }
        return fhw;
    }

    return absl::nullopt;
}

////////////////////////////////////////////////////////////////////////////////

}  // namespace rdss

#endif  // RDSS_GHD_H_
