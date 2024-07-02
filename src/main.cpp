#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>

#include "args.hpp"     // Argument parsers
#include "jobreport.hpp"
#include "dataframe.hpp"
#include "dataframe_io.hpp"

void main_cmd(const MainCmdArgs &args)
{
    JobReport jr(
        args.output,
        args.sampling_time,
        args.max_time
    );
    jr.run(args.cmd);
}

void print_cmd(const PrintCmdArgs &args)
{
    // Load data into DataFrame
    process_stats(args.input);
}


int main(int argc, char **argv)
{
    // Extract subcommand
    std::string cmd = "";
    if (argc >= 2)
    {
        cmd = argv[1];
    }
    
   if (cmd == "print"){
        PrintCmdArgs print_args;
        if(print_args.parse(argc, argv) != Status::Success) {
            print_args.help();
            return 1;
        }
        print_cmd(print_args);
    } else {
        MainCmdArgs main_args;
        if(main_args.parse(argc, argv) != Status::Success) {
            main_args.help();
            return 1;
        }
        main_cmd(main_args);
    }
    
    return 0;
}
