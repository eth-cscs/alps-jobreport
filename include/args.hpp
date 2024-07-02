#ifndef JOBREPORT_ARGS_HPP
#define JOBREPORT_ARGS_HPP

#define JOB_REPORT_VERSION "v1.0"

#include <iostream>
#include <string>
#include "status.hpp"
#include "third_party/argh/argh.hpp"

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

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport [subcommand | [options] -- workload_command]" << std::endl
            << std::endl
            << "Subcommands:" << std::endl
            << "  print                          Print a job report" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -v, --version                  Show version information" << std::endl
            << "  -o, --output <path>            Specify output directory (default: ./jobreport_<SLURM_JOB_ID>)" << std::endl
            << "  -u, --sampling_time <seconds>  Set the sampling time in seconds (default: automatically determined)" << std::endl
            << "  -t, --max_time <time>          Set the maximum time (format: DD-HH:MM:SS, default: 24:00:00)" << std::endl
            << std::endl
            << "Arguments:" << std::endl
            << "  workload_command               The command to run as the workload" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport -- sleep 5" << std::endl;
    }

    void print_params() {
        std::cout << "version: " << version << std::endl;
        std::cout << "output: " << output << std::endl;
        std::cout << "sampling_time: " << sampling_time << std::endl;
        std::cout << "max_time: " << max_time << std::endl;
        std::cout << "cmd: " << cmd << std::endl;
    }

    // Public variables used to store the parsed arguments with default values
    bool version = false;                 // -v, --version
    std::string output = "";              // -o, --output
    int sampling_time = 0;               // -u, --sampling_time
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
    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        if(parser.size() != 3) {
            return Status::InvalidValue;
        }

        parser(2) >> input;

        if (input.empty()) {
            return Status::MissingArgument;
        }

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport print <directory>" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport print jobreport_1234" << std::endl;
    }

    void print_params() {
        std::cout << "input: " << input << std::endl;
    }

    std::string input = ""; // -i, --input

private:
    argh::parser parser;
};
#endif // JOBREPORT_ARGS_HPP
