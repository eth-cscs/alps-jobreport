#include <iostream>
#include <fstream>
#include <string>
#include <memory>

#include "argparse.hpp" // Argument parser library
#include "args.hpp"     // Actual argument definitions
#include "jobreport.hpp"

void jobreport_start(const start_command &args)
{
    JobReport jr;
    jr.start(
        args.output,
        args.sampling_time,
        args.max_time,
        args.split_output,
        args.lock_file
    );

        const std::string &path = "",
        const int sampling_time = 10,
        const std::string &time_string = "12:00:00",
        const bool split_output = false
}

int main(int argc, char **argv)
{
    // Process non-arguments if they exist first
    // This is necessary as we expect the '--' delimiter to be the last argument
    // before non-arguments such as the profiled command
    // Example: ./jobreport -arg1=1 -- ./my_workload -arg1=2 will be processed as:
    // argc = 2, argv = {"./jobreport", "-arg1=1"}, non_arguments = "./my_workload -arg1=2"
    std::string non_arguments = extract_non_arguments(argc, argv);

    // Parse arguments
    std::unique_ptr<main_command> args = nullptr;

    try
    {
        args = std::make_unique<main_command>(argparse::parse<main_command>(argc, argv)); // Throws on error
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        main_command tmp;
        tmp.help(); // Print help message and exit
        return 1;
    }

    if (args->version)
    {
        std::cout << "Jobreport version 1.0" << std::endl;
        std::cout << "Written by Marcel :)" << std::endl;
        return 0;
    }

    args->help();
    return 0;
}
