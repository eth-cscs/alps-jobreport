#ifndef JOBREPORT_ARGS_HPP
#define JOBREPORT_ARGS_HPP

#define JOB_REPORT_VERSION "v1.0"

#include <iostream>
#include <string>
#include "status.hpp"
#include "argh.hpp"

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
    --split-output: Split the output into multiple files
    --lock-file-dir: Lock file to use for single file output
    -- [non-arguments]: Non-arguments to run as a workload command
*/
class MainCmdArgs {
public:
    MainCmdArgs() {  
        // Preregister the optional arguments which accept values
        parser.add_params({
            "-o", "--output",
            "-u", "--sampling_time",
            "-t", "--max_time",
            "--lock-file-dir"
        });        
    }

    Status parse(int argc, char** argv) {
        // Extract non-arguments
        non_arguments = extract_non_arguments(argc, argv);

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
        parser({"--split-output"}, split_output) >> split_output;
        parser({"--lock-file-dir"}, lock_file_dir) >> lock_file_dir;

        // This is required for the main command
        if(non_arguments.empty() && !version) {
            return Status::MissingNonArguments;
        }

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport [subcommand] [options] [-- workload_command]" << std::endl
            << std::endl
            << "Subcommands:" << std::endl
            << "  start                          Start the jobreport tool" << std::endl
            << "  stop                           Stop the jobreport tool" << std::endl
            << "  export                         Generate a jobreport file" << std::endl
            << "  hook                           Generate DCGM hook script for Pyxis" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -v, --version                  Show version information" << std::endl
            << "  -o, --output <path>            Specify output directory (default: ./)" << std::endl
            << "  -u, --sampling_time <seconds>  Set the sampling time in seconds (default: 10)" << std::endl
            << "  -t, --max_time <time>          Set the maximum time (format: DD-HH:MM:SS, default: 12:00:00)" << std::endl
            << "      --split-output             Split output into multiple files (default: false)" << std::endl
            << "      --lock-file-dir <path>     Specify lock file directory (default: /tmp)" << std::endl
            << std::endl
            << "Arguments:" << std::endl
            << "  workload_command               The command to run as the workload" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport -- ./workload -arg1=val1 -arg2=val2" << std::endl;
    }

    void print_params() {
        std::cout << "version: " << version << std::endl;
        std::cout << "output: " << output << std::endl;
        std::cout << "sampling_time: " << sampling_time << std::endl;
        std::cout << "max_time: " << max_time << std::endl;
        std::cout << "split_output: " << split_output << std::endl;
        std::cout << "lock_file_dir: " << lock_file_dir << std::endl;
        std::cout << "non_arguments: " << non_arguments << std::endl;
    }

    // Public variables used to store the parsed arguments with default values
    bool version = false;                 // -v, --version
    std::string output = "./";            // -o, --output
    int sampling_time = 10;               // -u, --sampling_time
    std::string max_time = "12:00:00";    // -t, --max_time
    bool split_output = false;            // --split-output
    std::string lock_file_dir = "/tmp";   // --lock-file-dir
    std::string non_arguments = "";       // Non-arguments to run as a workload command

private:
    argh::parser parser;
};

/*
jobreport start: Start the jobreport tool -> used for compatibility with containers
    -u, --sampling_time: Sampling time in seconds
    -t, --max_time: Maximum expected runtime in seconds
*/
class StartCmdArgs {
public:
    StartCmdArgs() {
        parser.add_params({
            "-u", "--sampling_time",
            "-t", "--max_time"
        });
    }

    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        parser({"-u", "--sampling_time"}, sampling_time) >> sampling_time;
        parser({"-t", "--max_time"}, max_time) >> max_time;

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport start [options]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -u, --sampling_time <seconds>  Set the sampling time in seconds (default: 10)" << std::endl
            << "  -t, --max_time <time>          Set the maximum time (format: DD-HH:MM:SS, default: 12:00:00)" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport start -u 20 -t 1-00:00:00" << std::endl;
    }

    void print_params() {
        std::cout << "sampling_time: " << sampling_time << std::endl;
        std::cout << "max_time: " << max_time << std::endl;
    }

    int sampling_time = 10;               // -u, --sampling_time
    std::string max_time = "12:00:00";    // -t, --max_time

private:
    argh::parser parser;
};

/*
jobreport stop: Stop the jobreport tool -> used for compatibility with containers
    -o, --output: Output file or directory depending on the context
    --split-output: Split the output into multiple files
    --lock-file-dir: Lock file to use for single file output
*/
class StopCmdArgs {
public:
    StopCmdArgs() {
        parser.add_params({
            "-o", "--output",
            "--lock-file-dir"
        });
    }

    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        parser({"-o", "--output"}, output) >> output;
        parser({"--split-output"}, split_output) >> split_output;
        parser({"--lock-file-dir"}, lock_file_dir) >> lock_file_dir;

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport stop [options]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -o, --output <path>            Specify output directory (default: ./)" << std::endl
            << "      --split-output             Split output into multiple files (default: false)" << std::endl
            << "      --lock-file-dir <path>     Specify lock file directory (default: /tmp)" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport stop -o ./output --split-output" << std::endl;
    }

    void print_params() {
        std::cout << "output: " << output << std::endl;
        std::cout << "split_output: " << split_output << std::endl;
        std::cout << "lock_file_dir: " << lock_file_dir << std::endl;
    }

    std::string output = "./";            // -o, --output
    bool split_output = false;            // --split-output
    std::string lock_file_dir = "/tmp";   // --lock-file-dir

private:
    argh::parser parser;
};

/*
jobreport export: Export the stats to a report file
    -i, --input: Input file or directory depending on the context
*/
class ExportCmdArgs {
public:
    ExportCmdArgs() {
        parser.add_params({
            "-i", "--input"
        });
    }

    Status parse(int argc, char** argv) {
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        parser({"-i", "--input"}, input) >> input;

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport export [options]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << "  -i, --input <path>             Specify input file or directory" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport export -i ./input" << std::endl;
    }

    void print_params() {
        std::cout << "input: " << input << std::endl;
    }

    std::string input = ""; // -i, --input

private:
    argh::parser parser;
};

/*
jobreport hook: Generate Pyxis DCGM hook script for the jobreport tool
    - None
*/
class HookCmdArgs {
public:
    HookCmdArgs() = default;

    Status parse(int argc, char** argv) {
        // Parse the arguments
        parser.parse(argc, argv);

        // Check if -h or --help is present
        if(parser[{"-?", "-h", "--help"}]) {
            return Status::Help;
        }

        return Status::Success;
    }

    void help() {
        std::cout 
            << "Usage: jobreport hook" << std::endl
            << std::endl
            << "This command generates a Pyxis DCGM hook script for the jobreport tool." << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -h, --help                     Show this help message" << std::endl
            << std::endl
            << "Example:" << std::endl
            << "  jobreport hook > dcgm_hook.sh" << std::endl;
    }

    private:
    argh::parser parser;
};

#endif // JOBREPORT_ARGS_HPP
