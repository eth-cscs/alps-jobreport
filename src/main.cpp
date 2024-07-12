#include <locale>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>

#include "macros.hpp"
#include "args.hpp" // Argument parsers
#include "jobreport.hpp"
#include "dataframe.hpp"
#include "dataframe_io.hpp"

void main_cmd(const MainCmdArgs &args)
{
    JobReport jr(
        args.output,
        args.sampling_time,
        args.max_time,
        args.ignore_gpu_binding,
        args.verbose,
        args.force
        );
    jr.run(args.cmd);
}

void print_cmd(const PrintCmdArgs &args)
{
    // Load data into DataFrame
    process_stats(args.input, args.output);
}

void hook_cmd(const HookCmdArgs &args)
{
    std::filesystem::path output;

    if (args.output.empty())
    {
        output = get_home_directory() / CONTAINER_HOOK_DEFAULT_IN_HOME;
    }
    else
    {
        output = args.output;
    }

    // Check if the output file already exists
    if (std::filesystem::exists(output))
    {
        raise_error("Error: Output file already exists: \"" + output.string() + "\"");
    }

    // Create the directory structure if it doesn't exist
    std::filesystem::path dir_path = std::filesystem::path(output).parent_path();
    if (!dir_path.empty() && !std::filesystem::exists(dir_path))
    {
        try{
            std::filesystem::create_directories(dir_path);
        } catch (std::exception &e) {
            raise_error("Error: Unable to create directories: \"" + dir_path.string() + "\"");
        }
    }

    // Write the script to the file
    std::ofstream ofs(output);
    if (!ofs.is_open())
    {
        raise_error("Error: Unable to open output file: \"" + output.string() + "\"");
    }

    std::cout << "Writing enroot hook to " << output << std::endl;
    ofs << ENROOT_HOOK;
    ofs.close();

    // Set the file as executable
    try {
        std::filesystem::permissions(output,
                                    std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec | std::filesystem::perms::others_exec,
                                    std::filesystem::perm_options::add);
    } catch (std::exception &e) {
        raise_error("Error: Unable to set executable permissions on file: \"" + output.string() + "\"");
    }

    std::cout
        << "Add the following to your container .toml file:" << std::endl
        << std::endl
        << "[annotations]" << std::endl
        << "com.hooks.dcgm.enabled = \"true\"" << std::endl
        << std::endl;
}

int main(int argc, char **argv)
{
    // Extract subcommand
    std::string cmd = "";
    if (argc >= 2)
    {
        cmd = argv[1];
    }


    //
    // ugly fix for when a user has a bad/strange/unsupported locale.
    // A bad locale causes a segfault in tabulate.
    // This has happened when a user sent their local locale via ssh 
    // to our machines that didn't support it.
    //
    // try 
    // {
    //     std::locale::global(std::locale(""));
    // }
    // catch (...)
    // {
    // 	setenv("LC_CTYPE", "C", 1);
    //     std::locale::global(std::locale(""));
    // }

    if (cmd == "print")
    {
        PrintCmdArgs print_args;
        if (print_args.parse(argc, argv) != Status::Success)
        {
            print_args.help();
            return 1;
        }
        print_cmd(print_args);
    }
    else if (cmd == "container-hook")
    {
        HookCmdArgs container_hook_args;
        if (container_hook_args.parse(argc, argv) != Status::Success)
        {
            container_hook_args.help();
            return 1;
        }
        hook_cmd(container_hook_args);
    }
    else
    {
        MainCmdArgs main_args;

        if (main_args.parse(argc, argv) != Status::Success)
        {
            main_args.help();
            return 1;
        }

        if (main_args.version)
        {
            std::cout << "ALPS Jobreport - v0.1" << std::endl;
        }
        else
        {
            main_cmd(main_args);
        }
    }

    return 0;
}
