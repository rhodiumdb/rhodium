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
#include "codegen.hpp"
#include "ghd.hpp"
#include "interpreter.hpp"
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

absl::Status TestYannakakis() {
    RelationFactory fac;
    Tree<Relation*, JoinOn> tree {
        fac.Make<RelationReference>("A", 2),
        {
            { { fac.Make<RelationReference>("B", 2), {} }, {{0, 0}} },
            { { fac.Make<RelationReference>("C", 2), {} }, {{1, 0}} }
        }
    };

    Relation* normal = fac.Make<RelationJoin>(
        fac.Make<RelationJoin>(fac.Make<RelationReference>("A", 2),
                               fac.Make<RelationReference>("C", 2),
                               JoinOn {{1, 0}}),
        fac.Make<RelationReference>("B", 2),
        JoinOn {{0, 0}});
    Relation* yannakakis = Yannakakis(&fac, tree);

    std::cerr << normal->ToString() << "\n";
    std::cerr << yannakakis->ToString() << "\n";

    Table a_table(2);
    Table b_table(2);
    Table c_table(2);

    RETURN_IF_ERROR(a_table.InsertTuple({100, 5}));
    RETURN_IF_ERROR(a_table.InsertTuple({101, 6}));
    RETURN_IF_ERROR(a_table.InsertTuple({102, 7}));
    RETURN_IF_ERROR(b_table.InsertTuple({101, 500}));
    RETURN_IF_ERROR(b_table.InsertTuple({102, 501}));
    RETURN_IF_ERROR(b_table.InsertTuple({103, 502}));
    RETURN_IF_ERROR(c_table.InsertTuple({5, 800}));
    RETURN_IF_ERROR(c_table.InsertTuple({5, 801}));
    RETURN_IF_ERROR(c_table.InsertTuple({7, 802}));
    RETURN_IF_ERROR(c_table.InsertTuple({7, 803}));
    RETURN_IF_ERROR(c_table.InsertTuple({8, 804}));

    // 102, 501, 7, 802
    // 102, 501, 7, 803

    Table result_table(0);

    {
        absl::btree_map<std::string, Table> variables;
        variables.insert_or_assign("A", a_table);
        variables.insert_or_assign("B", b_table);
        variables.insert_or_assign("C", c_table);

        Interpreter interpreter(variables);

        RETURN_IF_ERROR(interpreter.Interpret(normal));
        result_table = interpreter.Lookup(normal).value();
    }

    for (int32_t i = 0; i < result_table.NumberOfTuples(); i++) {
        std::cerr << "TestYannakakis: normal: ["
                  << absl::StrJoin(result_table.GetTuple(i), ", ")
                  << "]\n";
    }

    {
        absl::btree_map<std::string, Table> variables;
        variables.insert_or_assign("A", a_table);
        variables.insert_or_assign("B", b_table);
        variables.insert_or_assign("C", c_table);

        Interpreter interpreter(variables);

        RETURN_IF_ERROR(interpreter.Interpret(yannakakis));
        result_table = interpreter.Lookup(yannakakis).value();
    }

    for (int32_t i = 0; i < result_table.NumberOfTuples(); i++) {
        std::cerr << "TestYannakakis: yannakakis: ["
                  << absl::StrJoin(result_table.GetTuple(i), ", ")
                  << "]\n";
    }

    return absl::OkStatus();
}

absl::Status TestInterpreter() {
    RelationFactory fac;
    auto r = fac.Make<RelationReference>("R", 3);
    auto s = fac.Make<RelationReference>("S", 2);

    Table r_table(3);
    Table s_table(2);

    RETURN_IF_ERROR(r_table.InsertTuple({500, 3415, 1000}));
    RETURN_IF_ERROR(r_table.InsertTuple({501, 2241, 1001}));
    RETURN_IF_ERROR(r_table.InsertTuple({502, 3401, 1000}));
    RETURN_IF_ERROR(r_table.InsertTuple({503, 2202, 1002}));
    RETURN_IF_ERROR(s_table.InsertTuple({1001, 501}));
    RETURN_IF_ERROR(s_table.InsertTuple({1002, 503}));

    Table result_table(0);

    {
        absl::btree_map<std::string, Table> variables;
        variables.insert_or_assign("R", r_table);
        variables.insert_or_assign("S", s_table);

        Interpreter interpreter(variables);

        JoinOn join_on;
        join_on.insert({2, 0});
        auto semijoin = fac.Make<RelationSemijoin>(r, s, join_on);

        RETURN_IF_ERROR(interpreter.Interpret(semijoin));
        result_table = interpreter.Lookup(semijoin).value();
    }

    for (int32_t i = 0; i < result_table.NumberOfTuples(); i++) {
        std::cerr << "DEBUG: ["
                  << absl::StrJoin(result_table.GetTuple(i), ", ")
                  << "]\n";
    }

    std::cerr << "---------\n";

    RETURN_IF_ERROR(s_table.InsertTuple({1002, 504}));

    {
        absl::btree_map<std::string, Table> variables;
        variables.insert_or_assign("R", r_table);
        variables.insert_or_assign("S", s_table);

        Interpreter interpreter(variables);

        JoinOn join_on;
        join_on.insert({2, 0});
        auto join = fac.Make<RelationJoin>(r, s, join_on);

        RETURN_IF_ERROR(interpreter.Interpret(join));
        result_table = interpreter.Lookup(join).value();
    }

    for (int32_t i = 0; i < result_table.NumberOfTuples(); i++) {
        std::cerr << "DEBUG: ["
                  << absl::StrJoin(result_table.GetTuple(i), ", ")
                  << "]\n";
    }

    return absl::OkStatus();
}

absl::Status TestCodegen() {
    RelationFactory fac;
    auto r = fac.Make<RelationReference>("R", 2);
    auto s = fac.Make<RelationReference>("S", 1);

    JoinOn join_on;
    join_on.insert({1, 0});
    auto semijoin = fac.Make<RelationSemijoin>(r, s, join_on);

    auto rel_union = fac.Make<RelationUnion>(r, semijoin);

    TypingContext typing_context;

    typing_context[r] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[s] = new TypeRow({ new TypeInt });
    typing_context[semijoin] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[rel_union] = new TypeRow({ new TypeInt, new TypeInt });

    FreshVariableSource source;

    Codegen codegen("example", &source, typing_context);

    RETURN_IF_ERROR(codegen.Run(rel_union));

    std::cerr << "DEBUG: codegen: " << codegen.ds.ToCpp(&source) << "\n";

    return absl::OkStatus();
}

absl::Status RealMain() {
    rdss::DataStructure example("Example");
    // example.members.push_back(
    //     Member {
    //         VarName("foo"),
    //         absl::make_unique<TypeVector>(absl::make_unique<TypeInt>())
    //     });
    rdss::Method method(VarName("example"));

    method.body.push_back(new ActionGetMember {
            VarName("foo"), VarName("bar"), VarName("baz")
        });
    example.methods.push_back(method);
    FreshVariableSource source;
    std::cout << example.ToCpp(&source);

    TestGHD();
    RETURN_IF_ERROR(TestYannakakis());
    RETURN_IF_ERROR(TestInterpreter());
    RETURN_IF_ERROR(TestCodegen());

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
