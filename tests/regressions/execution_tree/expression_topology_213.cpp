//  Copyright (c) 2017-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <hpx/hpx_main.hpp>
#include <hpx/testing.hpp>

#include <string>

void test_expressiontree_topology(char const* name,
    char const* codestr, char const* newick_expected,
    char const* dot_expected)
{
    phylanx::execution_tree::compiler::function_list snippets;

    auto const& code = phylanx::execution_tree::compile(
        phylanx::ast::generate_ast(codestr), snippets);
    auto topology = snippets.program_.get_expression_topology();

    std::string newick_tree =
        phylanx::execution_tree::newick_tree(name, topology);
    HPX_TEST_EQ(newick_tree, std::string(newick_expected));

    std::string dot_tree = phylanx::execution_tree::dot_tree(name, topology);
    HPX_TEST_EQ(dot_tree, std::string(dot_expected));
}

int main(int argc, char* argv[])
{
    test_expressiontree_topology("fact",
        "block("
            "define(fact, arg0, if(arg0 <= 1, 1, arg0 * fact(arg0 - 1))),"
            "fact(10)"
        ")",
        "(((((((/phylanx$0/access-argument$0$arg0/0$1$29) "
        "/phylanx$0/__le$0/0$1$29,(/phylanx$0/access-argument$1$arg0/0$1$43,("
        "/phylanx$0/access-function$0$fact/0$1$50) "
        "/phylanx$0/call-function$0$fact/0$1$50) /phylanx$0/__mul$0/0$1$43) "
        "/phylanx$0/if$0/0$1$26) /phylanx$0/lambda$0/0$1$14) "
        "/phylanx$0/function$0$fact/0$1$14) "
        "/phylanx$0/define-variable$0$fact/0$1$14,(/phylanx$0/"
        "access-function$1$fact/0$1$67) /phylanx$0/call-function$1$fact/0$1$67) "
        "/phylanx$0/block$0/0$1$1) fact;",
        "graph \"fact\" {\n"
        "    \"/phylanx$0/block$0/0$1$1\" -- "
                "\"/phylanx$0/define-variable$0$fact/0$1$14\";\n"
        "    \"/phylanx$0/define-variable$0$fact/0$1$14\" -- "
                "\"/phylanx$0/function$0$fact/0$1$14\";\n"
        "    \"/phylanx$0/function$0$fact/0$1$14\" -- "
                "\"/phylanx$0/lambda$0/0$1$14\";\n"
        "    \"/phylanx$0/lambda$0/0$1$14\" -- \"/phylanx$0/if$0/0$1$26\";\n"
        "    \"/phylanx$0/if$0/0$1$26\" -- \"/phylanx$0/__le$0/0$1$29\";\n"
        "    \"/phylanx$0/__le$0/0$1$29\" -- "
                "\"/phylanx$0/access-argument$0$arg0/0$1$29\";\n"
        "    \"/phylanx$0/access-argument$0$arg0/0$1$29\";\n"
        "    \"/phylanx$0/if$0/0$1$26\" -- \"/phylanx$0/__mul$0/0$1$43\";\n"
        "    \"/phylanx$0/__mul$0/0$1$43\" -- "
                "\"/phylanx$0/access-argument$1$arg0/0$1$43\";\n"
        "    \"/phylanx$0/access-argument$1$arg0/0$1$43\";\n"
        "    \"/phylanx$0/__mul$0/0$1$43\" -- "
                "\"/phylanx$0/call-function$0$fact/0$1$50\";\n"
        "    \"/phylanx$0/call-function$0$fact/0$1$50\" -- "
                "\"/phylanx$0/access-function$0$fact/0$1$50\";\n"
        "    \"/phylanx$0/access-function$0$fact/0$1$50\" -- "
                "\"/phylanx$0/function$0$fact/0$1$14\";\n"
        "    \"/phylanx$0/function$0$fact/0$1$14\";\n"
        "    \"/phylanx$0/block$0/0$1$1\" -- "
                "\"/phylanx$0/call-function$1$fact/0$1$67\";\n"
        "    \"/phylanx$0/call-function$1$fact/0$1$67\" -- "
                "\"/phylanx$0/access-function$1$fact/0$1$67\";\n"
        "    \"/phylanx$0/access-function$1$fact/0$1$67\" -- "
                "\"/phylanx$0/function$0$fact/0$1$14\";\n"
        "    \"/phylanx$0/function$0$fact/0$1$14\";\n"
        "}\n");

    return hpx::util::report_errors();
}


