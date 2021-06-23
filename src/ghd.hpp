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
#include <absl/status/status.h>
#include <absl/status/statusor.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <absl/types/optional.h>

#include "ast.hpp"
#include "union_find_map.hpp"

namespace rdss {

////////////////////////////////////////////////////////////////////////////////

using HyperedgeId = int32_t;

template <typename V> class Hypergraph {
public:
  void AddVertex(const V &vertex) { this->vertex_to_hyperedge[vertex]; }

  HyperedgeId AddEdge() {
    HyperedgeId result = this->hyperedges.size();
    this->hyperedges.push_back(Hyperedge{absl::flat_hash_set<V>()});
    return result;
  }

  bool AddVertexToEdge(const V &vertex, HyperedgeId edge) {
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
  EdgesIncidentOnVertex(const V &vertex) const {
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
    for (const auto &[key, value] : this->vertex_to_hyperedge) {
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

  int32_t NumVertices() const { return this->vertex_to_hyperedge.size(); }

  int32_t NumEdges() const { return this->hyperedges.size(); }

  void DeleteVertex(const V &vertex) {
    if (auto edges = EdgesIncidentOnVertex(vertex)) {
      for (const auto &edge : *edges) {
        this->hyperedges.at(edge).vertices.erase(vertex);
      }
      this->vertex_to_hyperedge.erase(vertex);
    }
  }

  void DeleteEdge(HyperedgeId edge) {
    if (auto vertices = VerticesInEdge(edge)) {
      for (const auto &vertex : *vertices) {
        this->vertex_to_hyperedge.at(vertex).erase(edge);
      }
      this->hyperedges.at(edge) = Hyperedge{{}};
    }
  }

  void Print() const {
    std::cerr << "([";
    for (const Hyperedge &edge : this->hyperedges) {
      std::cerr << "{";
      for (const V &vertex : edge.vertices) {
        std::cerr << vertex << ",";
      }
      std::cerr << "},";
    }
    std::cerr << "],{";
    for (const auto &[vertex, edges] : this->vertex_to_hyperedge) {
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

using NodeId = int32_t;

template <typename Value> struct Digraph {
  std::vector<Value> node_values;
  absl::flat_hash_map<NodeId, absl::flat_hash_set<NodeId>> edges_out_of;

  absl::flat_hash_set<NodeId> roots;

  NodeId AddVertex(const Value &value) {
    int32_t result = node_values.size();
    node_values.push_back(value);
    edges_out_of[result];
    roots.insert(result);
    return result;
  }

  bool AddEdge(NodeId x, NodeId y) {
    if (!edges_out_of.contains(x)) {
      return false;
    }

    if (!edges_out_of.contains(y)) {
      return false;
    }

    edges_out_of.at(x).insert(y);
    roots.erase(y);

    return true;
  }

  const absl::flat_hash_set<NodeId> &Roots() const { return roots; }

  const absl::flat_hash_set<NodeId> &EdgesOutOf(NodeId node) const {
    return edges_out_of.at(node);
  }

  std::vector<NodeId> AllNodes() const {
    std::vector<NodeId> result;
    for (int32_t i = 0; i < node_values.size(); i++) {
      result.push_back(i);
    }
    return result;
  }

  Value &GetValue(NodeId node) { return node_values.at(node); }

  const Value &GetValue(NodeId node) const { return node_values.at(node); }
};

template <typename V, typename E> struct Tree {
  V element;
  std::vector<std::pair<Tree, E>> children;

  std::string Print(std::function<std::string(const V &)> callback) const {
    std::stringstream ss;
    ss << "{ \"element\": " << callback(this->element) << ","
       << "  \"children\": [ ";
    std::vector<std::string> cs;
    for (const auto &child : this->children) {
      cs.push_back(child.Print(callback));
    }
    ss << absl::StrJoin(cs, ", ") << " ] }";

    return ss.str();
  }
};

template <typename V> struct Bag {
  absl::flat_hash_set<V> attributes;
  absl::flat_hash_map<HyperedgeId, double> relations;
  Bag() : attributes(), relations() {}
};

template <typename V>
absl::optional<Tree<V, absl::monostate>> DigraphToTree(Digraph<V> &digraph) {
  if (digraph.Roots().size() != 1) {
    return absl::nullopt;
  }

  NodeId root = *(digraph.Roots().begin());

  std::vector<std::pair<NodeId, Tree<V, absl::monostate> *>> active;
  absl::flat_hash_set<NodeId> seen;

  Tree<V, absl::monostate> tree{digraph.GetValue(root), {}};

  active.push_back(std::pair<NodeId, Tree<V, absl::monostate> *>{root, &tree});
  while (!active.empty()) {
    auto [source, parent] = active.back();
    active.pop_back();
    if (seen.contains(source)) {
      return absl::nullopt;
    }
    seen.insert(source);

    for (const auto &target : digraph.EdgesOutOf(source)) {
      parent->children.push_back(
          {Tree<V, absl::monostate>{digraph.GetValue(target), {}},
           absl::monostate()});
      // FIXME this is a dirty hack
      parent->children.back().first.children.reserve(
          digraph.EdgesOutOf(target).size());
      active.push_back({target, &parent->children.back().first});
    }
  }

  return tree;
}

template <typename V>
bool VerifyRunningIntersectionProperty(const Digraph<Bag<V>> &digraph) {
  absl::flat_hash_set<V> all_attributes;

  for (NodeId node : digraph.AllNodes()) {
    for (auto attribute : digraph.GetValue(node).attributes) {
      all_attributes.insert(attribute);
    }
  }

  for (V attribute : all_attributes) {
    absl::flat_hash_set<NodeId> contains_attribute;
    for (NodeId node : digraph.AllNodes()) {
      if (digraph.GetValue(node).attributes.contains(attribute)) {
        contains_attribute.insert(node);
      }
    }
    UnionFindMap<NodeId, absl::monostate> uf;

    for (NodeId node : contains_attribute) {
      uf.Insert(node, absl::monostate());
    }

    for (NodeId source : contains_attribute) {
      for (NodeId target : digraph.EdgesOutOf(source)) {
        if (contains_attribute.contains(target)) {
          uf.Union(source, target, [](absl::monostate, absl::monostate) {
            return absl::monostate();
          });
        }
      }
    }

    if (uf.GetRepresentatives().size() != 1) {
      return false;
    }
  }

  return true;
}

void LookupInModel(z3::model model, absl::string_view name,
                   std::function<void(const z3::func_decl &)> callback) {
  for (int32_t i = 0; i < model.size(); i++) {
    if (model[i].name().str() == name) {
      callback(model[i]);
    }
  }
}

template <typename V> struct FHD {
  double fhw;
  Tree<Bag<V>, absl::monostate> tree;
};

template <typename V>
absl::StatusOr<FHD<V>> ComputeFHD(const Hypergraph<V> &hypergraph) {
  for (const V &vertex : hypergraph.AllVertices()) {
    if (hypergraph.EdgesIncidentOnVertex(vertex).value().empty()) {
      return absl::FailedPreconditionError(
          "Detected vertex with no covering edges.");
    }
  }

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
    optional_expr(const z3::expr &expr) : value_(expr) {}
    const z3::expr &value() { return value_.value(); }
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
        if ((i == j) || (i == k) || (j == k)) {
          continue;
        }
        s.add(!o_star[i][j].value() || !o_star[j][k].value() ||
              o_star[i][k].value());
      }
    }
  }

  for (auto edge : edges_vec) {
    absl::flat_hash_set<V> vs = hypergraph.VerticesInEdge(edge).value();
    for (const auto &x : vs) {
      for (const auto &y : vs) {
        auto i = vertex_to_int.at(x);
        auto j = vertex_to_int.at(y);

        if (i >= j) {
          continue;
        }
        s.add(o_star[j][i].value() || a[i][j].value());
        s.add(o_star[i][j].value() || a[j][i].value());
      }
    }
  }

  for (int32_t i = 0; i < num_vertices; i++) {
    for (int32_t j = 0; j < num_vertices; j++) {
      for (int32_t k = 0; k < num_vertices; k++) {
        if ((i == j) || (i == k) || (j >= k)) {
          continue;
        }
        s.add(!a[i][j].value() || !a[i][k].value() || o_star[k][j].value() ||
              a[j][k].value());
        s.add(!a[i][j].value() || !a[i][k].value() || o_star[j][k].value() ||
              a[k][j].value());
        s.add(!a[i][j].value() || !a[i][k].value() || a[j][k].value() ||
              a[k][j].value());
      }
    }
  }

  for (int32_t i = 0; i < num_vertices; i++) {
    s.add(!a[i][i].value());
  }

  {
    for (int32_t i = 0; i < num_vertices; i++) {
      for (int32_t j = 0; j < num_vertices; j++) {
        if (i == j) {
          continue;
        }
        absl::optional<z3::expr> sum;
        for (int32_t e = 0; e < num_edges; e++) {
          if (hypergraph.VerticesInEdge(edges_vec[e])
                  .value()
                  .contains(vertices_vec[j])) {
            sum = sum ? *sum + w[i][e].value() : w[i][e].value();
          }
        }
        s.add(!a[i][j].value() || (sum.value() >= 1));
      }
    }

    for (int32_t i = 0; i < num_vertices; i++) {
      absl::optional<z3::expr> sum;
      for (int32_t e = 0; e < num_edges; e++) {
        if (hypergraph.VerticesInEdge(edges_vec[e])
                .value()
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

  auto s_check = s.check();
  if (s_check == z3::unsat) {
    return absl::InternalError("Z3 returned unsat. This should never happen.");
  } else if (s_check == z3::unknown) {
    return absl::DeadlineExceededError("Z3 returned unknown. This usually "
                                       "means it ran out of time or memory.");
  }

  auto model = s.get_model();
  absl::optional<double> fhw;
  LookupInModel(model, "m", [&fhw, &model](const z3::func_decl &decl) {
    double temp;
    if (model.get_const_interp(decl).is_numeral(temp)) {
      fhw = temp;
    }
  });

  std::vector<int32_t> ordering;
  for (int32_t i = 0; i < num_vertices; i++) {
    auto pos = ordering.begin();
    for (int32_t j : ordering) {
      bool ostar_j_i = false;
      LookupInModel(model, absl::StrFormat("ostar_%d_%d", j, i),
                    [&ostar_j_i, &model](const z3::func_decl &decl) {
                      ostar_j_i = model.get_const_interp(decl).bool_value() ==
                                  Z3_L_TRUE;
                    });
      if (!ostar_j_i) {
        break;
      }
      pos++;
    }
    ordering.insert(pos, i);
  }

  Digraph<Bag<V>> tree_graph;

  for (const V &vertex : vertices_vec) {
    (void)tree_graph.AddVertex(Bag<V>());
    // note: we are going to rely on the equivalence between indices in
    // vertices_vec and valid NodeIds from here on.
  }

  for (int32_t i = 0; i < num_vertices; i++) {
    for (int32_t e = 0; e < num_edges; e++) {
      absl::optional<double> n;
      LookupInModel(model, absl::StrFormat("w_%d_%d", i, e),
                    [&n, &model](const z3::func_decl &decl) {
                      double temp;
                      if (model.get_const_interp(decl).is_numeral(temp)) {
                        n = temp;
                      }
                    });
      assert(n.has_value());
      tree_graph.GetValue(i).relations[edges_vec[e]] = n.value();
    }
  }

  auto smallest =
      [&ordering, &vertices_vec](
          const absl::flat_hash_set<V> &x) -> absl::optional<int32_t> {
    for (int32_t v : ordering) {
      if (x.contains(vertices_vec[v])) {
        return v;
      }
    }
    return absl::nullopt;
  };

  absl::flat_hash_map<V, absl::flat_hash_set<V>> chi;

  for (HyperedgeId edge : edges_vec) {
    auto vertices = hypergraph.VerticesInEdge(edge).value();
    for (const V &vertex : vertices) {
      tree_graph.GetValue(smallest(vertices).value()).attributes.insert(vertex);
    }
  }

  for (int32_t v : ordering) {
    absl::flat_hash_set<V> temp = tree_graph.GetValue(v).attributes;
    if (temp.size() > 1) {
      temp.erase(vertices_vec[v]);
      int32_t next = smallest(temp).value();
      tree_graph.GetValue(next).attributes.merge(temp);
      tree_graph.AddEdge(next, v);
    }
  }

  auto tree = DigraphToTree(tree_graph).value();

  /*    std::cerr << tree.Print([](const Bag<V>& bag) -> std::string {
          std::vector<std::string> vec;

          for (const auto& attribute : bag.attributes) {
              vec.push_back(absl::StrFormat("\"%d\"", attribute));
          }

          return absl::StrCat(
              "[",
              absl::StrJoin(vec, ", "),
              "]"
          );
      }) << "\n";

      std::cerr << "Running Intersection Property: "
                << VerifyRunningIntersectionProperty(tree_graph) << "\n";
  */
  if (!fhw.has_value()) {
    return absl::InternalError("Could not find 'm' in the model.");
  }
  if (!VerifyRunningIntersectionProperty(tree_graph)) {
    return absl::InternalError(
        "Result failed to satisfy running intersection property.");
  }

  return FHD<V>{fhw.value(), tree};
}

template <typename S, typename T>
std::pair<T, S> FlipPair(const std::pair<S, T> &pair) {
  return {pair.second, pair.first};
}

JoinOn FlipJoinOn(const JoinOn &join_on) {
  JoinOn result;
  for (const auto &pair : join_on) {
    result.insert(FlipPair(pair));
  }
  return result;
}

Relation *Yannakakis(RelationFactory *factory,
                     Tree<Relation *, JoinOn> join_tree) {
  using TreeNode = Tree<Relation *, JoinOn> *;
  absl::flat_hash_map<TreeNode, std::pair<TreeNode, JoinOn>> parent_of;
  absl::flat_hash_set<TreeNode> leaf_nodes;

  { // Compute `parent_of` and `leaf_nodes`
    std::vector<TreeNode> active;
    active.push_back(&join_tree);
    while (!active.empty()) {
      TreeNode node = active.back();
      active.pop_back();
      bool had_children = false;
      for (auto &[subtree, join_on] : node->children) {
        had_children = true;
        TreeNode subnode = &subtree;
        parent_of[subnode] = {node, join_on};
        active.push_back(subnode);
      }
      if (!had_children) {
        leaf_nodes.insert(node);
      }
    }
  }

  { // First bottom-up pass
    std::deque<TreeNode> active(leaf_nodes.begin(), leaf_nodes.end());
    absl::flat_hash_set<TreeNode> inserted = leaf_nodes;

    while (!active.empty()) {
      TreeNode node = active.back();
      active.pop_back();

      if (parent_of.contains(node)) {
        auto [parent, join_on] = parent_of.at(node);

        parent->element = factory->Make<RelationSemijoin>(
            parent->element, node->element, join_on);

        if (!inserted.contains(parent)) {
          active.push_front(parent);
          inserted.insert(parent);
        }
      }
    }
  }

  { // Top-down pass
    std::deque<TreeNode> active;
    active.push_front(&join_tree);

    while (!active.empty()) {
      TreeNode node = active.back();
      active.pop_back();

      for (auto &[subtree, join_on] : node->children) {
        TreeNode subnode = &subtree;

        subnode->element = factory->Make<RelationSemijoin>(
            subnode->element, node->element, FlipJoinOn(join_on));

        active.push_front(subnode);
      }
    }
  }

  { // Second bottom-up pass
    std::deque<TreeNode> active(leaf_nodes.begin(), leaf_nodes.end());
    absl::flat_hash_set<TreeNode> inserted = leaf_nodes;

    while (!active.empty()) {
      TreeNode node = active.back();
      active.pop_back();

      if (parent_of.contains(node)) {
        auto [parent, join_on] = parent_of.at(node);

        parent->element = factory->Make<RelationJoin>(parent->element,
                                                      node->element, join_on);

        if (!inserted.contains(parent)) {
          active.push_front(parent);
          inserted.insert(parent);
        }
      }
    }
  }

  return join_tree.element;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace rdss

#endif // RDSS_GHD_H_
