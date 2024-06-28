#ifndef JOBREPORT_HPP
#define JOBREPORT_HPP

#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <filesystem>
#include <optional>

#include "dcgm_agent.h"
#include "dcgm_fields.h"
#include "dcgm_structs.h"
#include "slurm_job.hpp"
#include "utils.hpp"
#include "dataframe.hpp"
#include "dataframe_io.hpp"

class JobReport
{
public:
    JobReport(
        const std::string &path = "",
        int sampling_time = 10,
        const std::string &time_string = "12:00:00")
        : sampling_time(sampling_time*1000000), split_output(split_output)
    {
        LOG("Initializing JobReport object." << std::endl
            << "Output path: " << path << std::endl
            << "Sampling time: " << sampling_time << std::endl
            << "Time string: " << time_string << std::endl
            << "Split output: " << split_output << std::endl
        );

        initialize(path, time_string);
    }

    ~JobReport()
    {
        cleanup();
    }

    void start();
    void stop();
    void run(const std::string& cmd);

private:
    // Input arguments
    int sampling_time; // in microseconds
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

    // Methods
    void initialize(const std::string &path, const std::string &time_string);
    void initialize_dcgm_handle();
    void cleanup();
    void check_error(dcgmReturn_t result, const std::string &errorMsg);
    void get_job_name();
    void set_output_path(const std::string &path);
    void start_job_stats();
    void stop_job_stats();
    void write_job_stats();
};

void JobReport::initialize(const std::string &path, const std::string &time_string)
{
    job.read_slurm_env();
    get_job_name();
    jobInfo.version = dcgmJobInfo_version;
    set_output_path(path);
    max_runtime = parse_time(time_string);
}

void JobReport::set_output_path(const std::string &path)
{
    if (path.empty())
    {
        output_path = std::filesystem::current_path() / ("report_" + job.job_id);
    }
    else
    {
        output_path = path;
    }

    // Create the output directory
    // This should act as mkdir -p command and not throw an error if the directory already exists,
    // if the directory is not empty or if the parent directory does not exist.
    if(std::filesystem::exists(output_path) && !std::filesystem::is_directory(output_path)){
        raise_error("Output path exists and is not a directory.");
    }

    std::filesystem::create_directories(output_path);

    output_path = output_path / ("report_" + job.job_id + "_" + job.proc_id + ".csv");
    output_path = std::filesystem::absolute(output_path);
    LOG(output_path);
}

void JobReport::get_job_name()
{
    std::string tmp = job.job_id + "_" + job.proc_id;
    if (tmp.size() > 63)
    {
        std::cerr << "WARNING: string ${SLURM_JOB_ID}_${SLURM_PROC_ID} exceeds the safe \
                      limit of 64 characters allowed for job name.\n";
        tmp.resize(63);
    }
    std::strncpy(job_name, tmp.c_str(), tmp.size());
    job_name[tmp.size()] = '\0';

    LOG("Job name: " + std::string(job_name));
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

void JobReport::initialize_dcgm_handle()
{
    LOG("Initializing DCGM handle.");
    check_error(dcgmInit(), "Error initializing DCGM engine.");
    check_error(dcgmConnect("127.0.0.1", &dcgmHandle), "Error connecting to remote DCGM engine.");
}

void JobReport::write_job_stats(){
    std::ofstream ofs(output_path);
    DataFrame df(jobInfo, job);
    df.dump(ofs);
    ofs.close();
}

void JobReport::start_job_stats()
{
    //check_error(dcgmWatchPidFields(dcgmHandle, group, sampling_time, max_runtime, 0), "Error setting PID watches.");
    LOG(
        "Starting job stats with the following parameters:" << std::endl
        << "Sampling time: " << sampling_time << std::endl
        << "Max runtime: " << max_runtime << std::endl
        << "Job name: " << job_name << std::endl
    );
    check_error(dcgmWatchJobFields(dcgmHandle, group, sampling_time, max_runtime, 0), "Error setting job watches.");
    check_error(dcgmJobStartStats(dcgmHandle, group, job_name), "Error starting job stats.");
}

void JobReport::stop_job_stats()
{
    LOG("Stopping job stats...");
    check_error(dcgmJobGetStats(dcgmHandle, job_name, &jobInfo), "Error getting job stats.");
    check_error(dcgmJobStopStats(dcgmHandle, job_name), "Error stopping job stats.");
    check_error(dcgmJobRemove(dcgmHandle, job_name), "Error removing job stats.");
}

void JobReport::start()
{
    if (!job.root)
    {
        return;
    }

    if(std::stoul(job.proc_id) == 0){
        std::cout << "Starting job statistics.\n";
    }

    initialize_dcgm_handle();
    start_job_stats();
}

void JobReport::stop()
{
    if (!job.root)
    {
        return;
    }

    if(std::stoul(job.proc_id) == 0){
        std::cout << "Stopping job statistics.\n";
    }

    initialize_dcgm_handle();
    stop_job_stats();
}

void JobReport::run(const std::string& cmd)
{   
    if(std::stoul(job.proc_id) == 0){
        std::cout << "Recording job statistics..." << std::endl;
    }

    if (job.root)
    {
        initialize_dcgm_handle();
        start_job_stats();
    }

    system(cmd.c_str());

    if (job.root)
    {
        stop_job_stats();
        write_job_stats();
    }
}

#endif // JOBREPORT_HPP