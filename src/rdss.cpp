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
#include "subprocess.hpp"
#include "logging/logging.hpp"
#include "filesystem/filesystem.hpp"
#include "filesystem/temp_directory.hpp"

#include <cstdlib>
#include <iostream>
#include <filesystem>

#include <absl/memory/memory.h>
#include <absl/status/status.h>
#include <absl/status/statusor.h>

namespace rdss {

absl::StatusOr<std::string> RunGCC(absl::string_view generated_code,
                                   absl::string_view testing_code) {
    ASSIGN_OR_RETURN(TempDirectory temp_dir, TempDirectory::Create());

    std::filesystem::path main_file = temp_dir.path() / "main.cpp";

    RETURN_IF_ERROR(
        SetFileContents(main_file,
                        absl::StrCat(
                            "#include <iostream>\n",
                            "#include <absl/container/flat_hash_set.h>\n",
                            "#include <absl/container/flat_hash_map.h>\n",
                            "#include <absl/strings/str_format.h>\n",
                            "#include <absl/strings/str_join.h>\n",
                            "\n",
                            generated_code,
                            "\n",
                            "int main() {\n",
                            testing_code,
                            "return 0;\n",
                            "}\n")));

    ASSIGN_OR_RETURN(
        auto gcc_output_pair,
        InvokeSubprocess({"/usr/bin/env", "g++", "-o", "test",
                "-labsl_bad_any_cast_impl",
                "-labsl_bad_optional_access",
                "-labsl_bad_variant_access",
                "-labsl_base",
                "-labsl_city",
                "-labsl_civil_time",
                "-labsl_cord",
                "-labsl_debugging_internal",
                "-labsl_demangle_internal",
                "-labsl_examine_stack",
                "-labsl_exponential_biased",
                "-labsl_failure_signal_handler",
                "-labsl_flags_commandlineflag_internal",
                "-labsl_flags_commandlineflag",
                "-labsl_flags_config",
                "-labsl_flags_internal",
                "-labsl_flags_marshalling",
                "-labsl_flags_parse",
                "-labsl_flags_private_handle_accessor",
                "-labsl_flags_program_name",
                "-labsl_flags_reflection",
                "-labsl_flags",
                "-labsl_flags_usage_internal",
                "-labsl_flags_usage",
                "-labsl_graphcycles_internal",
                "-labsl_hash",
                "-labsl_hashtablez_sampler",
                "-labsl_int128",
                "-labsl_leak_check_disable",
                "-labsl_leak_check",
                "-labsl_log_severity",
                "-labsl_malloc_internal",
                "-labsl_periodic_sampler",
                "-labsl_random_distributions",
                "-labsl_random_internal_distribution_test_util",
                "-labsl_random_internal_platform",
                "-labsl_random_internal_pool_urbg",
                "-labsl_random_internal_randen_hwaes_impl",
                "-labsl_random_internal_randen_hwaes",
                "-labsl_random_internal_randen_slow",
                "-labsl_random_internal_randen",
                "-labsl_random_internal_seed_material",
                "-labsl_random_seed_gen_exception",
                "-labsl_random_seed_sequences",
                "-labsl_raw_hash_set",
                "-labsl_raw_logging_internal",
                "-labsl_scoped_set_env",
                "-labsl_spinlock_wait",
                "-labsl_stacktrace",
                "-labsl_statusor",
                "-labsl_status",
                "-labsl_strerror",
                "-labsl_str_format_internal",
                "-labsl_strings_internal",
                "-labsl_strings",
                "-labsl_symbolize",
                "-labsl_synchronization",
                "-labsl_throw_delegate",
                "-labsl_time",
                "-labsl_time_zone",
                "main.cpp"},
            temp_dir.path()));
    auto [gcc_stdout, gcc_stderr] = gcc_output_pair;
    RDSS_VLOG(1) << "gcc_stdout: \"" << gcc_stdout << "\"\n";
    RDSS_VLOG(1) << "gcc_stderr: \"" << gcc_stderr << "\"\n";

    std::filesystem::path main_exec = temp_dir.path() / "test";

    ASSIGN_OR_RETURN(
        auto main_output_pair,
        InvokeSubprocess({"test"}, temp_dir.path()));
    auto [main_stdout, main_stderr] = main_output_pair;
    RDSS_VLOG(1) << "main_stdout: \"" << main_stdout << "\"\n";
    RDSS_VLOG(1) << "main_stderr: \"" << main_stderr << "\"\n";

    RETURN_IF_ERROR(std::move(temp_dir).Cleanup());

    return main_stdout;
}

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
        absl::btree_map<RelName, Table> variables;
        variables.insert_or_assign(RelName("A"), a_table);
        variables.insert_or_assign(RelName("B"), b_table);
        variables.insert_or_assign(RelName("C"), c_table);

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
        absl::btree_map<RelName, Table> variables;
        variables.insert_or_assign(RelName("A"), a_table);
        variables.insert_or_assign(RelName("B"), b_table);
        variables.insert_or_assign(RelName("C"), c_table);

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
        absl::btree_map<RelName, Table> variables;
        variables.insert_or_assign(RelName("R"), r_table);
        variables.insert_or_assign(RelName("S"), s_table);

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
        absl::btree_map<RelName, Table> variables;
        variables.insert_or_assign(RelName("R"), r_table);
        variables.insert_or_assign(RelName("S"), s_table);

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

    auto difference = fac.Make<RelationDifference>(rel_union, semijoin);

    auto view = fac.Make<RelationView>(Viewed<Relation*>({1, 0}, difference));

    TypingContext typing_context;

    typing_context[r] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[s] = new TypeRow({ new TypeInt });
    typing_context[semijoin] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[rel_union] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[difference] = new TypeRow({ new TypeInt, new TypeInt });
    typing_context[view] = new TypeRow({ new TypeInt, new TypeInt });

    FreshVariableSource source;

    Codegen codegen("Example", &source, typing_context);

    RETURN_IF_ERROR(codegen.ProcessRelation(semijoin));

    std::string generated = codegen.ds.ToCpp(&source);

    std::cerr << "DEBUG: codegen: " << generated << "\n";

    std::string test_code;

    absl::StrAppend(&test_code,
                    "Example test;", "\n",
                    "test.R_insert({0, 0});", "\n",
                    "test.R_insert({0, 1});", "\n",
                    "test.R_insert({5, 1});", "\n",
                    "test.R_insert({0, 2});", "\n",
                    "test.R_insert({0, 3});", "\n",
                    "test.R_insert({6, 3});", "\n",
                    "test.S_insert({1});", "\n",
                    "test.S_insert({3});", "\n",
                    "std::vector<std::tuple<int32_t, int32_t>> sorted(test.fresh0.begin(), test.fresh0.end());", "\n",
                    "std::sort(sorted.begin(), sorted.end());", "\n",
                    "std::vector<std::string> sorted_strings;", "\n"
                    "for (const auto& [x, y] : sorted) {", "\n",
                    "    sorted_strings.push_back(absl::StrFormat(\"{%d, %d}\", x, y));", "\n",
                    "}", "\n",
                    "std::cout << \"{\" << absl::StrJoin(sorted_strings, \", \") << \"}\\n\";", "\n",
                    "");

    ASSIGN_OR_RETURN(std::string output, RunGCC(generated, test_code));

    std::cerr << "DEBUG: execution output: " << output << "\n";

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
