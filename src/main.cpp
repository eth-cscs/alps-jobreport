#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <filesystem>

#include "args.hpp"     // Argument parsers
#include "jobreport.hpp"
#include "dataframe.hpp"
#include "dataframe_io.hpp"

// Start subcommand
void start_cmd(const StartCmdArgs &args)
{
    JobReport jr(
        ".",                // Dummy output path
        args.sampling_time,
        args.max_time
    );
    jr.start();
}

// Stop subcommand
void stop_cmd(const StopCmdArgs &args)
{
    JobReport jr(
        args.output,
        0,                  // Dummy sampling time
        "0:00:00"           // Dummy max time
    );
    jr.stop();
}

void main_cmd(const MainCmdArgs &args)
{
    JobReport jr(
        args.output,
        args.sampling_time,
        args.max_time
    );
    jr.run(args.cmd);
}

void export_cmd(const ExportCmdArgs &args)
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
    
    if(cmd == "start") {
        StartCmdArgs start_args;
        if(start_args.parse(argc, argv) != Status::Success) {
            start_args.help();
            return 1;
        }
        start_cmd(start_args);
    } else if (cmd == "stop") {
        StopCmdArgs stop_args;
        if(stop_args.parse(argc, argv) != Status::Success) {
            stop_args.help();
            return 1;
        }
        stop_cmd(stop_args);
    } else if (cmd == "export"){
        ExportCmdArgs export_args;
        if(export_args.parse(argc, argv) != Status::Success) {
            export_args.help();
            return 1;
        }
        export_cmd(export_args);
    } else if (cmd == "hook") {
        HookCmdArgs hook_args;
        if(hook_args.parse(argc, argv) != Status::Success) {
            hook_args.help();
            return 1;
        }
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
