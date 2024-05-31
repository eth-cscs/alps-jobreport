#ifndef JOBREPORT_HPP
#define JOBREPORT_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <filesystem>

#include "dcgm_agent.h"
#include "dcgm_fields.h"
#include "dcgm_structs.h"
#include "slurm_job.hpp"
#include "utils.hpp"
#include "stats_io.hpp"

class JobReport
{
public:
    JobReport(
        const std::string &path = "",
        const int sampling_time = 10,
        const std::string &time_string = "12:00:00",
        const bool split_output = false,
        const std::string &lock_file = "/tmp") : sampling_time(sampling_time), split_output(split_output)
    {
        // These steps must always be performed before doing anything else
        // so we place them in the constructor
        // Query the Slurm environment variables
        job.read_slurm_env();

        // Get the job name
        get_job_name(job_name);

        // Determine if the current process is the root process
        isRootProcess = job.get<int>("proc_id") % job.get<int>("n_tasks_per_node") == 0;

        // Set jobInfo version
        jobInfo.version = dcgmJobInfo_version;

        // Set the io path
        DEBUG_LOG("Constructor got path=" << path);
        set_output_path(path);
        DEBUG_LOG("Output path set to=" << output_path);

        // Parse the maximum runtime
        DEBUG_LOG("Parsing time string=" << time_string);
        parse_time(time_string);
        DEBUG_LOG("Parsed time=" << max_runtime);

        // Set the lockfile path
        lockfile_path = std::filesystem::path(lock_file) / "job_report_" +
                        job.get<std::string>("job_id") / job.get<std::string>("job_id") + ".lock";
    };

    // Methods
    void start();
    void stop();

private:
    // Input arguments
    int sampling_time;
    bool split_output = true;
    int max_runtime;

    // SLURM Variables
    SlurmJob job;

    // DCGM Variables
    dcgmHandle_t dcgmHandle = (dcgmHandle_t)NULL;
    dcgmGpuGrp_t group;
    dcgmJobInfo_t jobInfo;
    char job_name[64];

    // Process variables
    std::filesystem::path output_path;
    std::filesystem::path lockfile_path;
    bool isRootProcess;

    // Methods
    void initialize_dcgm_handle();
    void cleanup();
    void check_error(dcgmReturn_t result, const std::string &errorMsg);
    void get_job_name(char job_name[64]);
    void set_output_path(const std::string &path);
    void start_job_stats();
    void stop_job_stats();

    // bool determineRootProcess();
    // void displayStats(const dcgmJobInfo_t& jobInfo);
    // void cleanup();
    // int stringToInt(const std::string& str);
    // bool getHostName(std::string& hostname);
};

void JobReport::parse_time(const std::string &time_str)
{
    int days = 0, hours = 0, minutes = 0, seconds = 0;
    std::vector<int> time_components;

    // Check for the presence of the '-' specifier at the beginning
    std::string remaining_time = time_str;
    if (time_str.find('-') != std::string::npos)
    {
        size_t dash_pos = time_str.find('-');
        days = std::stoi(time_str.substr(0, dash_pos));
        remaining_time = time_str.substr(dash_pos + 1);
    }

    // Create a stringstream from the remaining time string
    std::stringstream ss(remaining_time);
    std::string component;

    // Split the input string based on the ':' delimiter
    while (std::getline(ss, component, ':'))
    {
        time_components.push_back(std::stoi(component));
    }

    // Determine the format based on the number of components
    int n = time_components.size();
    if (n == 1)
    {
        // Only minutes provided
        minutes = time_components[0];
    }
    else if (n == 2)
    {
        // Format is minutes:seconds
        minutes = time_components[0];
        seconds = time_components[1];
    }
    else if (n == 3)
    {
        // Format is hours:minutes:seconds
        hours = time_components[0];
        minutes = time_components[1];
        seconds = time_components[2];
    }

    // Calculate the total time in seconds
    max_runtime = (days * 86400) + (hours * 3600) + (minutes * 60) + seconds;
}

void JobReport::set_output_path(const std::string &path)
{
    // Get the current working directory
    std::filesystem::path cwd = std::filesystem::current_path();

    const std::string job_id = job.get<std::string>("job_id");
    const std::string proc_id = job.get<std::string>("proc_id");

    if (path.empty())
    {
        if (split_output)
        {
            // Output path is a directory
            output_path = cwd / ("report_" + job_id);
        }
        else
        {
            // Output path is a file
            output_path = cwd / ("report_" + job_id + "_" + proc_id + ".bin");
        }
    }
    else
    {
        // Path is already set
        output_path = path;
    }

    // Make output_path an absolute path
    output_path = std::filesystem::absolute(output_path);
}

void JobReport::get_job_name(char job_name[64])
{
    std::string tmp = job.get<std::string>("job_id") + "_" + job.get<std::string>("proc_id");

    if (tmp.size() > 63)
    {
        std::cout << "WARNING: string ${SLURM_JOB_ID}_${SLURM_PROC_ID} exceeds the safe limit of"
                     "of 64 characters allowed for job name. Consider specifying --job-name if problems occur.\n";
    }

    size_t n = std::min((size_t)63, tmp.size());
    tmp.copy(job_name, n);
    job_name[n] = '\0';
}

void JobReport::cleanup()
{
    std::cout << "Cleaning up.\n";
    dcgmDisconnect(dcgmHandle);
    dcgmShutdown();
}

void JobReport::check_error(dcgmReturn_t result, const std::string &errorMsg)
{
    if (result != DCGM_ST_OK)
    {
        cleanup();
        raise_error(errorMsg);
    }
}

// Initialize the DCGM handle
void JobReport::initialize_dcgm_handle()
{
    dcgmReturn_t result = dcgmInit();
    check_error(result, "Error initializing DCGM engine.");

    char hostIpAddress[] = "127.0.0.1";
    result = dcgmConnect(hostIpAddress, &dcgmHandle);
    check_error(result, "Error connecting to remote DCGM engine.");
}

void JobReport::start_job_stats()
{
    dcgmReturn_t result;
    group = DCGM_GROUP_ALL_GPUS;

    result = dcgmWatchJobFields(dcgmHandle, group, 100000, 3600, 0);
    check_error(result, "Error setting job watches.");

    result = dcgmJobStartStats(dcgmHandle, group, job_name);
    check_error(result, "Error starting job stats.");
}

void JobReport::stop_job_stats()
{
    dcgmReturn_t result;

    result = dcgmJobGetStats(dcgmHandle, job_name, &jobInfo);
    check_error(result, "Error getting job stats.");

    result = dcgmJobStopStats(dcgmHandle, job_name);
    check_error(result, "Error stopping job stats.");

    result = dcgmJobRemove(dcgmHandle, job_name);
    check_error(result, "Error removing job stats.");
}

// Starting and stopping a job via start() and stop() functions before and after the workload
// has limited functionality. It is better to wrap the workload with jobreport directly.
void JobReport::start()
{
    if (!isRootProcess)
    {
        return;
    }

    // Using start and stop only supports DCGM_GROUP_ALL_GPUS
    group = DCGM_GROUP_ALL_GPUS;

    std::cout << "Starting job statistics.\n";
    initialize_dcgm_handle();
    start_job_stats();
}

// Stop the job statistics collection process and print the results
void JobReport::stop()
{
    if (!isRootProcess)
    {
        return;
    }

    // Using start and stop only supports DCGM_GROUP_ALL_GPUS
    group = DCGM_GROUP_ALL_GPUS;

    std::cout << "Stopping job statistics.\n";
    initialize_dcgm_handle();
    stop_job_stats();
}

#endif // JOBREPORT_HPP
