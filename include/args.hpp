#ifndef JOBREPORT_ARGS_HPP
#define JOBREPORT_ARGS_HPP

#include <iostream>
#include <string>
#include "status.hpp"
#include "third_party/argh/argh.hpp"
#include "utils.hpp"

// This function looks for the '--' delimiter and extracts
// the following string as a non-argument
std::string extract_non_arguments(int &argc, char **argv) {
    std::string non_arguments = "";
    int n_non_arguments = 0;
    bool found_delimiter = false;
    for (int i = argc - 1; i > 0; --i) {
        if (std::string(argv[i]) == "--") {
            found_delimiter = true;
            break;
        } else {
            ++n_non_arguments;
        }
    }

    if (!found_delimiter) {
        return non_arguments;
    }

    // Concatenate non-arguments
    for (int i = argc - n_non_arguments; i < argc; ++i) {
        non_arguments += argv[i];
        if (i < argc - 1) {
            non_arguments += " ";
        }
    }

    // Update argc
    argc -= (n_non_arguments + 1); // +1 for the '--' delimiter

    return non_arguments;
}

/*
jobreport: Main command. Usage: jobreport -arg=val -- ./workload -arg=val
    -v, --version: Print version
    -o, --output: Output file or directory depending on the context
    -u, --sampling_time: Sampling time in seconds
    -t, --max_time: Maximum expected runtime in seconds
    -- [non-arguments]: Non-arguments to run as a workload command
*/
class MainCmdArgs {
public:
    MainCmdArgs() {  
        // Preregister the optional arguments which accept values
        parser.add_params({
            "-o", "--output",
            "-u", "--sampling_time",
            "-t", "--max_time"
        });        
    }

    Status parse(int argc, char** argv) {
        // Extract non-arguments
        cmd = extract_non_arguments(argc, argv);

        // Parse the arguments
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        // Check if -v or --version is present
        if(parser[{"-v", "--version"}]){
            version = true;
            return Status::Success;
        }

        parser({"-o", "--output"}, output) >> output;
        parser({"-u", "--sampling_time"}, sampling_time) >> sampling_time;
        parser({"-t", "--max_time"}, max_time) >> max_time;

        // This is required for the main command
        if(cmd.empty()) {
            return Status::MissingNonArguments;
        }
        
        if (sampling_time < 0) {
            std::cout << "Invalid value for -u, --sampling_time" << std::endl
                      << "Expected a positive value, got: \"" << sampling_time << "\"" << std::endl;
            return Status::InvalidValue;
        }

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport [-v -h] [subcommand] -- COMMAND" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                        Show this help message" << std::endl
            << "  -v, --version                     Show version information" << std::endl
            << std::endl
            << "Subcommands:" << std::endl
            << "  monitor                           Monitor the performance metrics for a job. (Default)" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Specify output directory (default: ./jobreport_<SLURM_JOB_ID>)" << std::endl
            << "    -u, --sampling_time <seconds>   Set the time between samples (default: automatically determined)" << std::endl
            << "    -t, --max_time <time>           Set the maximum monitoring time (format: DD-HH:MM:SS, default: 24:00:00)" << std::endl
            << "  print                             Print a job report" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Output path for the report file" << std::endl
            << "  container-hook                    Write enroot hook for jobreport" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Output path for the enroot hook file" << std::endl
	    << "                                    (default: $HOME/" << CONTAINER_HOOK_DEFAULT_IN_HOME << ")" << std::endl
            << std::endl
            << "Arguments:" << std::endl
            << "  COMMAND                           The command to run as the workload" << std::endl
            << std::endl
            << "Examples:" << std::endl
            << "  jobreport -- sleep 5" << std::endl
            << "  jobreport monitor -o report -- sleep 5" << std::endl
            << "  jobreport print ./report" << std::endl
            << "  jobreport container-hook" << std::endl
            << std::endl
            << "Further documentation can be found on the CSCS Knowledge Base: https://docs.cscs.ch" << std::endl
            << "Open bug reports with the CSCS Service Desk:                   https://support.cscs.ch" << std::endl
            << std::endl;
    }

    // Public variables used to store the parsed arguments with default values
    bool version = false;                 // -v, --version
    std::string output = "";              // -o, --output
    int sampling_time = 0;                // -u, --sampling_time
    std::string max_time = "";            // -t, --max_time
    std::string cmd = "";                 // Non-arguments to run as a workload command

private:
    argh::parser parser;
};

/*
jobreport print: Print the stats to a report file
    -i, --input: Input file or directory depending on the context
*/
class PrintCmdArgs {
public:
    PrintCmdArgs() {
        // Preregister the optional arguments which accept values
        parser.add_params({
            "-o", "--output"
        });
    }

    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        parser({"-o", "--output"}, output) >> output;
        parser(2) >> input;

        if (input.empty()) {
            return Status::MissingArgument;
        }

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport print [-h -o <path>] <directory>" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -o, --output <path>            Output path for the report file" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport print jobreport_1234" << std::endl
            << "  jobreport print -o report.txt jobreport_1234" << std::endl;
    }

    std::string input = ""; 
    std::string output = "";

private:
    argh::parser parser;
};

class HookCmdArgs {
public:
    HookCmdArgs() {
        // Preregister the optional arguments which accept values
        parser.add_params({
            "-o", "--output"
        });
    }

    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        parser({"-o", "--output"}, output) >> output;

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport container-hook [-h -o <path>]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -o, --output <path>            Output path for the hook file" << std::endl
            << "                                 (default: $HOME/" << CONTAINER_HOOK_DEFAULT_IN_HOME << ")" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport container-hook" << std::endl
            << "  jobreport container-hook -o hook.sh" << std::endl
            << std::endl
            << "To activate the hook, add the following to your container .toml file:" << std::endl
            << std::endl
            << "[env]" << std::endl
            << "ENROOT_DCGM_HOOK = \"true\"" << std::endl
            << std::endl;
    }

    std::string output = "";

private:
    argh::parser parser;
};
#endif // JOBREPORT_ARGS_HPP
