#ifndef JOBREPORT_ARGS_HPP
#define JOBREPORT_ARGS_HPP

#define JOB_REPORT_VERSION "v1.0"

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

        // Read optional arguments
        parser({"-v", "--version"}, version) >> version;
        parser({"-o", "--output"}, output) >> output;
        parser({"-u", "--sampling_time"}, sampling_time) >> sampling_time;
        parser({"-t", "--max_time"}, max_time) >> max_time;

        // This is required for the main command
        if(cmd.empty() && !version) {
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
            << "Options:" << std::endl
            << "  -h, --help                        Show this help message" << std::endl
            << "  -v, --version                     Show version information" << std::endl
            << "Subcommands:" << std::endl
            << "  monitor                           Monitor the performance metrics for a job" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Specify output directory (default: ./jobreport_<SLURM_JOB_ID>)" << std::endl
            << "    -u, --sampling_time <seconds>   Set the sampling time in seconds (default: automatically determined)" << std::endl
            << "    -t, --max_time <time>           Set the maximum time (format: DD-HH:MM:SS, default: 24:00:00)" << std::endl
            << "  print                             Print a job report" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Output path for the report file" << std::endl
            << "  container-hook                    Write enroot hook for jobreport" << std::endl
            << "    -h, --help                      Shows help message" << std::endl
            << "    -o, --output <path>             Output path for the enroot hook file" << std::endl
            << "Arguments:" << std::endl
            << "  COMMAND                           The command to run as the workload" << std::endl
            << "Examples:" << std::endl
            << "  jobreport -- sleep 5" << std::endl
            << "  jobreport monitor -- sleep 5" << std::endl
            << "  jobreport print ./report" << std::endl
            << "  jobreport container-hook" << std::endl;
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
            << "  -o, --output <path>            Output path for the hook file (default: $HOME/.config/enroot/hooks.d/dcgm_hook.sh)" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport container-hook" << std::endl
            << "  jobreport container-hook -o " << std::endl;
    }

    std::string output = "";

private:
    argh::parser parser;
};
#endif // JOBREPORT_ARGS_HPP
