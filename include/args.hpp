
#ifndef JOBREPORT_ARGS_HPP
#define JOBREPORT_ARGS_HPP

#define JOB_REPORT_VERSION "v1.0"

#include <iostream>
#include <string>
#include "argparse.hpp"

/*
The supported commands are:
- jobreport: Main command. Usage: jobreport -arg=val -- ./workload -arg=val
    -v, --version: Print version
    -o, --output: Output file or directory depending on the context
    -u, --sampling_time: Sampling time in seconds
    -t, --max_time: Maximum expected runtime in seconds
    --split-output: Split the output into multiple files
    --lock-file: Lock file to use for single file output
    -- [non-arguments]: Non-arguments to run as a workload command

- jobreport start: Start the jobreport tool -> used for compatibility with containers
    -u, --sampling_time: Sampling time in seconds
    -t, --max_time: Maximum expected runtime in seconds

- jobreport stop: Stop the jobreport tool -> used for compatibility with containers
    -o, --output: Output file or directory depending on the context
    --split-output: Split the output into multiple files
    --lock-file: Lock file to use for single file output

- jobreport export: Export the stats to a report file
    -i, --input: Input file or directory depending on the context

- jobreport hook: Generate Pyxis DCGM hook script for the jobreport tool
    - None
*/

struct stop_command : public argparse::Args
{
    bool &all = flag("a,all", "Tell the command to automatically stage files that have been modified and deleted, but new files you have not told git about are not affected.");
    std::string &message = kwarg("m,message", "Use the given <msg> as the commit message.");
};

struct start_command : public argparse::Args
{
    std::string &source = arg("Source repository").set_default("origin");
    std::string &destination = arg("Destination repository").set_default("master");

    void welcome() override
    {
        std::cout << "Push code changes to remote" << std::endl;
    }
};

struct main_command : public argparse::Args
{
    bool &version = flag("v,version", "Print version and exit.").set_default(false);
    std::string &output = kwarg("o,output", "Output file or directory depending on the context.").set_default("./");
    int &sampling_time = kwarg("u,sampling_time", "Sampling time in seconds.").set_default(10);
    std::string &max_time = kwarg("t,max_time", "Maximum expected runtime of workload in DD:HH:MM format.").set_default("00:12:00");
    bool &split_output = flag("split-output", "Split the output into multiple files (used for compatibility reasons).").set_default(false);
    std::string &lock_file = kwarg("lock-file", "Specify output directory for lock file used for process synchronization.").set_default("/tmp");

    main_command() : argparse::Args(false) {}; // Signal that this is the main command
    
    // Subcommands
    start_command& start = subcommand("start");
    stop_command& stop = subcommand("stop");

};

// This function looks for the '--' delimter and extracts
// the following string as a non-argument
std::string extract_non_arguments(int &argc, char **argv)
{
    std::string non_argument = "";
    int n_non_arguments = 0;
    bool found_delimiter = false;
    for (int i = argc - 1; i > 0; --i)
    {
        if (std::string(argv[i]) == "--")
        {
            found_delimiter = true;
            break;
        }
        else
        {
            ++n_non_arguments;
        }
    }

    if (!found_delimiter)
    {
        return non_argument;
    }

    // Concatenate non-arguments
    for (int i = argc - n_non_arguments; i < argc; ++i)
    {
        non_argument += argv[i];
        if (i < argc - 1)
        {
            non_argument += " ";
        }
    }

    // Update argc
    argc -= (n_non_arguments + 1); // +1 for the '--' delimiter

    return non_argument;
}

#endif // JOBREPORT_ARGS_HPP