// Copyright (c) 2018 Parsa Amini
// Copyright (c) 2018 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>
#include <phylanx/execution_tree/compiler/primitive_name.hpp>

#include <hpx/hpx_main.hpp>

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

std::string read_user_code(std::string const& path)
{
    std::ifstream code_stream(path);
    if (!code_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Failed to open the specified file: " + path);
    }

    // Read the file
    std::ostringstream str_stream;
    if (!(str_stream << code_stream.rdbuf()))
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "read_user_code",
            "Failed to read code from the specified file: " + path);
    }

    return str_stream.str();
}

void dump_ast(std::vector<phylanx::ast::expression> const& ast, std::string path)
{
    std::ofstream ast_stream(path, std::ios::binary);
    if (!ast_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "dump_ast",
            "Failed to open the specified file: " + path);
    }

    // Serialize the AST into a byte array
    std::vector<char> bytes = phylanx::util::serialize(ast);

    // char is 1 byte
    ast_stream.write(bytes.data(), bytes.size());
}

std::vector<phylanx::ast::expression> load_ast_dump(std::string const& path)
{
    std::ifstream ast_stream(path, std::ios::binary | std::ios::ate);
    if (!ast_stream.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "dump_ast",
            "Failed to open the specified file: " + path);
    }

    // Size of the data
    auto const data_size = ast_stream.tellg();

    if (data_size == decltype(data_size)(0) || !ast_stream.seekg(0, std::ios::beg))
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "load_ast",
            "Failed to read the specified file: " + path);
    }

    // Allocate all the memory needed to load the AST upfront
    std::vector<char> bytes;
    bytes.reserve(data_size);

    return phylanx::util::unserialize<std::vector<phylanx::ast::expression>>(
        std::move(bytes));
}

void print_physl_code(std::vector<phylanx::ast::expression> const& ast)
{
    std::cout << "PhySL Code:\n";
    for (auto const& i : ast)
    {
        std::cout << phylanx::ast::to_string(i) << '\n';
    }
    std::cout << "\n\n";
}

void dump_physl_code(std::vector<phylanx::ast::expression> const& ast,
    std::string const& path)
{
    std::ofstream os(path);
    if (!os.good())
    {
        HPX_THROW_EXCEPTION(hpx::filesystem_error,
            "dump_physl_code",
            "Failed to open the specified file: " + path);
    }

    for (auto const& i : ast)
    {
        os << phylanx::ast::to_string(i) << '\n';
    }
}

std::vector<phylanx::execution_tree::primitive_argument_type>
read_arguments(std::vector<std::string> const& args,
    phylanx::execution_tree::compiler::function_list& snippets,
    phylanx::execution_tree::compiler::environment& env)
{
    std::vector<phylanx::execution_tree::primitive_argument_type> result(
        args.size());

    std::transform(
        args.begin(), args.end(), result.begin(),
        [&](std::string const& s)
        -> phylanx::execution_tree::primitive_argument_type
        {
            auto code = phylanx::execution_tree::compile(
                "<arguments>", s, snippets, env);
            return code.run();
        });

    return result;
}

///////////////////////////////////////////////////////////////////////////////
int handle_command_line(int argc, char* argv[], po::variables_map& vm)
{
    try
    {
        po::options_description cmdline_options(
            "Usage: physl <physl_script> [options] [arguments...]");
        cmdline_options.add_options()
            ("help,h", "print out program usage")
            ("docs","Print out all primitives/plugins and descriptions")
            ("code,c", po::value<std::string>(),
                "Execute the PhySL code given in argument")
            ("print,p", "Print the result of evaluation of the last "
                "PhySL expression encountered in the input")
            ("performance", "Print the topology of the created execution "
                "tree and the corresponding performance counter results")
            ("dump-dot", po::value<std::string>(), "Write the topology of the "
                "created execution tree as a dot file to a file")
            ("dump-newick-tree", po::value<std::string>(), "Write the topology "
                "of the created execution tree as a Newick tree to a file")
            ("transform,t", po::value<std::string>(),
                "file to read transformation rules from")
            ("dump-ast,d", po::value<std::string>()->implicit_value("<none>"),
                "file to dump AST to")
            ("load-ast,l", po::value<std::string>(),
                "file to dump AST to. If none path is provided, use the base "
                "file name of the input file replacing the extension to .ast")
            ("print-code", "Print the PhySL code that is to be executed")
            ("dump-code", po::value<std::string>(), "Write the PhySL code that "
                "is to be executed to a file")
            ("dump-counters", po::value<std::string>(), "Write the performance "
                "counter CSV data code to a file")
            ("dry-run", "Perform all other options requested but do not "
                "actually run the code")
        ;

        po::positional_options_description pd;
        pd.add("positional", -1);

        po::options_description positional_options;
        positional_options.add_options()
            ("positional", po::value<std::vector<std::string> >(),
             "positional options")
        ;

        po::options_description all_options;
        all_options.add(cmdline_options).add(positional_options);

        po::parsed_options const opts(
            po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(pd)
                .style(po::command_line_style::unix_style)
                .run()
            );

        po::store(opts, vm);

        if (vm.count("help") != 0)
        {
            std::cout << cmdline_options << std::endl;
            return 1;
        }
    }
    catch (std::exception const& e)
    {
        std::cerr << "physl: command line handling: exception caught: "
                  << e.what() << "\n";
        return -1;
    }
    return 0;
}

std::string get_dump_file(po::variables_map const& vm,
    fs::path code_source_path, bool code_is_file)
{
    std::string dump_file = vm["dump-ast"].as<std::string>();
    // If no dump file is specified but PhySL code is read from a
    // file then use the file name with .ast extension
    if (dump_file == "<none>")
    {
        if (!code_is_file)
        {
            HPX_THROW_EXCEPTION(hpx::commandline_option_error, "get_ast()",
                "the required path argument for option '--dump-ast' is "
                "missing");
        }
        return code_source_path.replace_extension("ast").string();
    }
    return dump_file;
}

std::vector<phylanx::ast::expression> ast_from_code_or_dump(
    po::variables_map const& vm, std::vector<std::string>& positional_args,
    std::string& code_source_name)
{
    // Return value
    std::vector<phylanx::ast::expression> ast;
    // Set to true if PhySL code was read from a file, used to generate a file
    // name for the AST dump file, if requested and a name is not provided
    bool code_is_file = false;
    fs::path code_source_path;

    // Determine if an AST dump is to be loaded
    if (vm.count("load-ast") != 0)
    {
        if (vm.count("code"))
        {
            HPX_THROW_EXCEPTION(hpx::commandline_option_error,
                "get_ast()",
                "'--dump-ast' and '--code' options cannot be used "
                "simultaneously");
        }

        std::string ast_dump_file = vm["load-ast"].as<std::string>();
        ast = load_ast_dump(ast_dump_file);
        code_source_name = fs::path(ast_dump_file).filename().string();
    }
    // Read PhySL source code from a file or the provided argument
    else
    {
        // PhySL source code
        std::string user_code;

        if (vm.count("code") != 0)
        {
            // Execute code as given directly on the command line
            user_code = vm["code"].as<std::string>();
            code_source_name = "<command_line>";
        }
        else if (!positional_args.empty() && !positional_args[0].empty())
        {
            // Interpret first argument as the file name for the PhySL code
            user_code = read_user_code(positional_args[0]);
            code_source_path = fs::path(positional_args[0]);
            code_source_name = code_source_path.filename().string();
            // The rest of the positional arguments are arguments for the script
            positional_args.erase(positional_args.begin());
            code_is_file = true;
        }
        else
        {
            HPX_THROW_EXCEPTION(hpx::commandline_option_error, "get_ast()",
                "No code was provided.");
        }

        // Compile the given code into AST
        ast = phylanx::ast::generate_ast(user_code);
    }

    // Apply transformation rules to AST, if requested
    if (vm.count("transform") != 0)
    {
        std::string const transform_rules =
            read_user_code(vm["transform"].as<std::string>());

        ast = phylanx::ast::transform_ast(
            ast, phylanx::ast::generate_transform_rules(transform_rules));
    }

    // Dump the AST to a file, if requested
    if (vm.count("dump-ast") != 0)
    {
        std::string dump_file =
            get_dump_file(vm, std::move(code_source_path), code_is_file);
        dump_ast(ast, std::move(dump_file));
    }

    return ast;
}

phylanx::execution_tree::compiler::result_type compile_and_run(
    std::vector<phylanx::ast::expression> const& ast,
    std::vector<std::string> const& positional_args,
    phylanx::execution_tree::compiler::function_list& snippets,
    std::string const& code_source_name, bool dry_run)
{
    phylanx::execution_tree::compiler::environment env =
        phylanx::execution_tree::compiler::default_environment();

    // Collect the arguments for running the code
    auto args = read_arguments(positional_args, snippets, env);

    // Compile AST into expression tree (into actual executable code);
    phylanx::execution_tree::define_variable(code_source_name,
        phylanx::execution_tree::compiler::primitive_name_parts{
            "sys_argv", -1, 0, 0},
        snippets, env,
        phylanx::execution_tree::primitive_argument_type{args});

    auto const code = phylanx::execution_tree::compile(
        code_source_name, ast, snippets, env);

    // Re-init all performance counters to guarantee correct measurement
    // results if those are requested on the command line.
    hpx::reinit_active_counters();

    // Evaluate user code using the read data
    if (!dry_run)
    {
        auto retval = code.run();
        if (phylanx::execution_tree::is_primitive_operand(retval))
        {
            return retval(std::move(args));
        }
        return retval;
    }
    return phylanx::ast::nil{};
}

///////////////////////////////////////////////////////////////////////////////
void print_performance_counter_data_csv(std::ostream& os)
{
    // CSV Header
    os << "primitive_instance,display_name,count,time,eval_direct\n";

    // List of existing primitive instances
    std::vector<std::string> existing_primitive_instances;

    // Retrieve all primitive instances
    for (auto const& entry :
        hpx::agas::find_symbols(hpx::launch::sync, "/phylanx/*$*"))
    {
        existing_primitive_instances.push_back(entry.first);
    }

    // Print performance data
    std::vector<std::string> const counter_names{
        "count/eval", "time/eval", "eval_direct"
    };

    for (auto const& entry : phylanx::util::retrieve_counter_data(
             existing_primitive_instances, counter_names))
    {
        os << "\"" << entry.first << "\",\""
                  << phylanx::execution_tree::compiler::primitive_display_name(
                         entry.first)
                  << "\"";
        for (auto const& counter_value : entry.second)
        {
            os << "," << counter_value;
        }
        os << "\n";
    }

    os << "\n";
}

void print_dot(std::string const& code_source_name,
    phylanx::execution_tree::topology const& topology, std::ostream& os)
{
    os << phylanx::execution_tree::dot_tree(code_source_name, topology) << "\n";
}

void print_newick_tree(std::string const& code_source_name,
    phylanx::execution_tree::topology const& topology, std::ostream& os)
{
    os << phylanx::execution_tree::newick_tree(code_source_name, topology)
       << "\n";
}

void print_performance_profile(
    phylanx::execution_tree::compiler::function_list& snippets,
    std::string const& code_source_name, std::string const& dot_file,
    std::string const& newick_tree_file, std::string const& counter_file)
{
    std::set<std::string> resolve_children;
    for (auto const& f : snippets.code_.entry_points())
    {
        resolve_children.insert(f.name_);
    }
    for (auto const& f : snippets.scratchpad_.entry_points())
    {
        resolve_children.insert(f.name_);
    }

    auto const topology = snippets.get_expression_topology(
        std::set<std::string>{}, std::move(resolve_children));

    if (dot_file.empty())
    {
        std::cout << "\n";
        print_dot(code_source_name, topology, std::cout);
    }
    else
    {
        std::ofstream os(dot_file);
        if (!os.good())
        {
            HPX_THROW_EXCEPTION(hpx::filesystem_error,
                "print_performance_profile",
                "Failed to open the specified file: " + dot_file);
        }

        print_dot(code_source_name, topology, os);
    }

    if (newick_tree_file.empty())
    {
        std::cout << "\n";
        print_newick_tree(code_source_name, topology, std::cout);
        std::cout << "\n";
    }
    else
    {
        std::ofstream os(newick_tree_file);
        if (!os.good())
        {
            HPX_THROW_EXCEPTION(hpx::filesystem_error,
                "print_performance_profile",
                "Failed to open the specified file: " + newick_tree_file);
        }

        print_newick_tree(code_source_name, topology, os);
    }

    if (counter_file.empty())
    {
        print_performance_counter_data_csv(std::cout);
    }
    else
    {
        std::ofstream os(counter_file);
        if (!os.good())
        {
            HPX_THROW_EXCEPTION(hpx::filesystem_error,
                "print_performance_profile",
                "Failed to open the specified file: " + counter_file);
        }

        print_performance_counter_data_csv(os);
    }
}

///////////////////////////////////////////////////////////////////////////////
void interpreter(po::variables_map const& vm)
{
    // Collect positional arguments
    std::vector<std::string> positional_args;
    if (vm.count("positional") != 0)
    {
        positional_args = vm["positional"].as<std::vector<std::string>>();
    }

    // Origin of PhySL code. It is either file name or <command_line>
    std::string code_source_name;

    // The AST that is either generated from PhySL code or loaded from an AST
    // dump. This also sets the name of the source code 'code_source_name'.
    std::vector<phylanx::ast::expression> ast =
        ast_from_code_or_dump(vm, positional_args, code_source_name);

    // Dump the code that is to be executed to the standard output, if requested
    if (vm.count("print-code") != 0)
    {
        print_physl_code(ast);
    }

    // Dump the code that is to be executed to a file, if requested
    if (vm.count("dump-code") != 0)
    {
        std::string const physl_file = vm["dump-code"].as<std::string>();
        dump_physl_code(ast, physl_file);
    }

    phylanx::execution_tree::compiler::function_list snippets;
    auto const result = compile_and_run(ast, positional_args, snippets,
            code_source_name, vm.count("dry-run") != 0);

    // Print the result of the last PhySL expression, if requested
    if (vm.count("print") != 0)
    {
        std::cout << result << "\n";
    }

    // Print auxiliary information at exit: topology of the execution tree
    // and the associate performance counter data
    if (vm.count("performance") != 0)
    {
        std::string dot_file =
            vm.count("dump-dot") == 0 ? "" : vm["dump-dot"].as<std::string>();
        std::string newick_tree_file = vm.count("dump-newick-tree") == 0 ?
            "" :
            vm["dump-newick-tree"].as<std::string>();
        std::string counter_file = vm.count("dump-counters") == 0 ?
            "" :
            vm["dump-counters"].as<std::string>();

        print_performance_profile(snippets, code_source_name, dot_file,
            newick_tree_file, counter_file);
    }
}

int main(int argc, char* argv[])
{
    po::variables_map vm;
    int const cmdline_result = handle_command_line(argc, argv, vm);
    if (cmdline_result != 0)
    {
        return cmdline_result > 0 ? 0 : cmdline_result;
    }

    if (vm.count("docs") != 0)
    {
        phylanx::execution_tree::show_patterns();
        return 0;
    }

    try
    {
        interpreter(vm);
    }
    catch (std::exception const& e)
    {
        std::cout << "physl: exception caught:\n" << e.what() << "\n";
        return -1;
    }

    return 0;
}
