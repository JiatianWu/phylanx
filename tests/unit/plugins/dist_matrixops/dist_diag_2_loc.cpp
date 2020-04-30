//   Copyright (c) 2020 Hartmut Kaiser
//   Copyright (c) 2020 Nanmiao Wu
//
//   Distributed under the Boost Software License, Version 1.0. (See accompanying
//   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/testing.hpp>

#include <string>
#include <utility>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
phylanx::execution_tree::primitive_argument_type compile_and_run(
    std::string const& name, std::string const& codestr)
{
    phylanx::execution_tree::compiler::function_list snippets;
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    auto const& code =
        phylanx::execution_tree::compile(name, codestr, snippets, env);
    return code.run().arg_;
}

///////////////////////////////////////////////////////////////////////////////
void test_diag_d_operation(std::string const& name, std::string const& code,
    std::string const& expected_str)
{
    phylanx::execution_tree::primitive_argument_type result =
        compile_and_run(name, code);
    phylanx::execution_tree::primitive_argument_type comparison =
        compile_and_run(name, expected_str);

    hpx::cout << result << "\n";

    HPX_TEST_EQ(result, comparison);
}

////////////////////////////////////////////////////////////////////////////////
void test_diag_1d_0()
{
    if (hpx::get_locality_id() == 0)
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([1, 2], "my_diag1d_0",
                    list("tile", list("columns", 0, 2))
                ),
                1, "row", 0, 2
            )
        )", R"(
            annotate_d([[0, 1, 0, 0], [0, 0, 2, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 0, 2),
                    list("tile", list("columns", 0, 4),
                    list("rows", 0, 2))))
        )");
    }
    else
    {
        test_diag_d_operation("test1d_0", R"(
            diag_d(
                annotate_d([3], "my_diag1d_0",
                    list("tile", list("columns", 2, 3))
                ),
                1, "row", 1, 2
            )
        )", R"(
            annotate_d([[0, 0, 0, 3], [0, 0, 0, 0]],
                "my_diag1d_0_diag/1",
                list("args",
                    list("locality", 1, 2),
                    list("tile", list("columns", 0, 4),
                    list("rows", 2, 4))))
        )");
    }
}

//void test_diag_1d_1()
//{
//    if (hpx::get_locality_id() == 0)
//    {
//        test_diag_d_operation("test1d_1", R"(
//            diag_d(
//                annotate_d([1, 2],
//                    list("tile", list("columns", 0, 2))),
//                1, "column", 0, 2
//            )
//        )", R"(
//            annotate_d([[0, 1], [0, 0], [0, 0], [0, 0]], "diag_array_2",
//                list("args",
//                    list("locality", 0, 2),
//                    list("tile", list("columns", 0, 2),
//                    list("rows", 0, 4))))
//        )");
//    }
//    else
//    {
//        test_diag_d_operation("test1d_1", R"(
//            diag_d(
//                annotate_d([3],
//                    list("tile", list("columns", 2, 3))),
//                1, "column", 1, 2
//            )
//        )", R"(
//            annotate_d([[0, 0], [2, 0], [0, 3], [0, 0]], "diag_array_2",
//                list("args",
//                    list("locality", 1, 2),
//                    list("tile", list("columns", 2, 4),
//                    list("rows", 0, 4))))
//        )");
//    }
//}


////////////////////////////////////////////////////////////////////////////////
int hpx_main(int argc, char* argv[])
{
    test_diag_1d_0();
//    test_diag_1d_1();

    hpx::finalize();
    return hpx::util::report_errors();
}

int main(int argc, char* argv[])
{
    std::vector<std::string> cfg = {
        "hpx.run_hpx_main!=1"
    };

    return hpx::init(argc, argv, cfg);
}
