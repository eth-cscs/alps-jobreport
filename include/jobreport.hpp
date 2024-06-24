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
#include "file_io.hpp"
#include "dataframe.hpp"
#include "dataframe_io.hpp"

class JobReport
{
public:
    JobReport(
        const std::string &path = "",
        int sampling_time = 10,
        const std::string &time_string = "12:00:00",
        bool split_output = false,
        const std::string &lock_file_dir = "/tmp")
        : sampling_time(sampling_time*1000000), split_output(split_output)
    {
        LOG("Initializing JobReport object." << std::endl
            << "Output path: " << path << std::endl
            << "Sampling time: " << sampling_time << std::endl
            << "Time string: " << time_string << std::endl
            << "Split output: " << split_output << std::endl
            << "Lock file directory: " << lock_file_dir << std::endl
        );

        initialize(path, time_string, lock_file_dir);
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
    std::filesystem::path lockfile_path;

    // Methods
    void initialize(const std::string &path, const std::string &time_string, const std::string &lock_file_dir);
    void initialize_dcgm_handle();
    void cleanup();
    void check_error(dcgmReturn_t result, const std::string &errorMsg);
    void get_job_name();
    void set_output_path(const std::string &path);
    void start_job_stats();
    void stop_job_stats();
    void print_job_stats();
    void write_job_stats();
};

void JobReport::initialize(const std::string &path, const std::string &time_string, const std::string &lock_file_dir)
{
    job.read_slurm_env();
    get_job_name();
    jobInfo.version = dcgmJobInfo_version;
    set_output_path(path);
    max_runtime = parse_time(time_string);
    lockfile_path = std::filesystem::path(lock_file_dir) / ("job_report_" + job.job_id + "_" + job.job_id + ".lock");
}

void JobReport::set_output_path(const std::string &path)
{
    auto cwd = std::filesystem::current_path();
    if (path.empty())
    {
        output_path = split_output ? cwd / ("report_" + job.job_id) : cwd / ("report_" + job.job_id + "_" + job.proc_id + ".bin");
    }
    else
    {
        output_path = path;
    }
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
    //FileIO file_io(!split_output, lockfile_path.string());
    DataFrame df(jobInfo);
    //file_io.write(output_path.string(), df);
    std::cout << "Job stats:" << std::endl
              << df << std::endl;
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

    LOG("Writing job stats to file: " + output_path.string());
    write_job_stats();
}

void JobReport::print_job_stats(){
    unsigned int n_gpus = jobInfo.numGpus;
    for(unsigned int i = 0; i < n_gpus; ++i){
        std::cout << "================================================" << "\n";
        std::cout << "GPU " << i << ":\n";
        std::cout << "smUtilization:\n";
        std::cout << "    minValue: " << jobInfo.gpus[i].smUtilization.minValue << "\n";
        std::cout << "    maxValue: " << jobInfo.gpus[i].smUtilization.maxValue << "\n";
        std::cout << "    average: " << jobInfo.gpus[i].smUtilization.average << "\n";
        std::cout << "memoryUtilization:\n";
        std::cout << "    minValue: " << jobInfo.gpus[i].memoryUtilization.minValue << "\n";
        std::cout << "    maxValue: " << jobInfo.gpus[i].memoryUtilization.maxValue << "\n";
        std::cout << "    average: " << jobInfo.gpus[i].memoryUtilization.average << "\n";
        std::cout << "================================================" << "\n";
    }
}

void JobReport::start()
{
    if (!job.root)
    {
        return;
    }

    std::cout << "Starting job statistics.\n";
    initialize_dcgm_handle();
    start_job_stats();
}

void JobReport::stop()
{
    if (!job.root)
    {
        return;
    }

    std::cout << "Stopping job statistics.\n";
    initialize_dcgm_handle();
    stop_job_stats();
}

void JobReport::run(const std::string& cmd)
{
    if (job.root)
    {
        initialize_dcgm_handle();
        start_job_stats();
    }

    system(cmd.c_str());

    if (job.root)
    {
        stop_job_stats();
        //print_job_stats();
    }
}

#endif // JOBREPORT_HPP