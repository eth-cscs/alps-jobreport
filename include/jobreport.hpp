#ifndef JOBREPORT_HPP
#define JOBREPORT_HPP

#include <string>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <sys/wait.h>
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
#include "macros.hpp"

class JobReport
{
public:
    JobReport(
        const std::string &path,
        int sampling_time,
        const std::string &time_string,
        const bool ignore_gpu_binding,
        const bool verbose,
        const bool force
        )
        : sampling_time(sampling_time * 1000000),
          ignore_gpu_binding(ignore_gpu_binding),
          verbose(verbose), 
          force(force)
    {
        initialize(path, time_string);
    }

    ~JobReport()
    {
        cleanup();
    }

    void start();
    void stop();
    void run(const std::string &cmd);

private:
    // Input arguments
    int sampling_time; // in microseconds
    int max_runtime;
    bool ignore_gpu_binding;
    bool verbose;
    bool force;

    // SLURM Variables
    SlurmJob job;

    // DCGM Variables
    dcgmHandle_t dcgmHandle = (dcgmHandle_t)NULL;
    dcgmGpuGrp_t group = (dcgmGpuGrp_t)DCGM_GROUP_ALL_GPUS;
    dcgmJobInfo_t jobInfo;
    char job_name[64];

    // Process variables
    std::filesystem::path output_path;
    pid_t child_pid = -1;

    // Methods
    void initialize(const std::string &path, const std::string &time_string);
    void initialize_dcgm_handle();
    void create_dcgm_group();
    void cleanup();
    void check_error(dcgmReturn_t result, const std::string &errorMsg);
    void get_job_name();
    void set_output_path(const std::string &path);
    void initialize_gpu_group();
    void start_job_stats();
    void stop_job_stats();
    void write_job_stats();
    void compute_time_params(const std::string &time_string);
    void print_root(const std::string &msg)
    {
        if (verbose && job.root)
            std::cout << msg << std::endl;
    };
};

void JobReport::initialize(const std::string &path, const std::string &time_string)
{
    // Need to know if the job is root or not before proceeding
    job.read_slurm_env(ignore_gpu_binding, verbose);

    print_root("ALPS Jobreport - v 0.1");
    print_root("Recording job performance statistics...");

    get_job_name();
    jobInfo.version = dcgmJobInfo_version;
    set_output_path(path);
    compute_time_params(time_string);
}

void JobReport::set_output_path(const std::string &path)
{
    if (path.empty())
    {
        output_path = std::filesystem::current_path() / ("jobreport_" + job.job_id);
    }
    else
    {
        output_path = std::filesystem::path(path);
    }

    // Check if a file exists with the same name
    if (std::filesystem::exists(output_path) && !std::filesystem::is_directory(output_path))
    {
        raise_error("Error: output path exists and is not a directory.");
    }

    if (job.root)
    {
        // Root process creates a metadata file in the output directory
        try{
            std::filesystem::create_directories(output_path);
            std::ofstream ofs(output_path / ROOT_METADATA_FILE, std::ios::app);
            ofs.close();
        } catch (std::exception &e) {
            raise_error("Error creating output directory and/or metadata files: " + std::string(e.what()));
        }
    }

    // Create the output directory
    // This should act as mkdir -p command and not throw an error if the directory already exists,
    // if the directory is not empty or if the parent directory does not exist.
    output_path = output_path / ("step_" + job.step_id);
    try{
        std::filesystem::create_directories(output_path);
    } catch (std::exception &e) {
        raise_error("Error creating output directory: " + std::string(e.what()));
    }
    
    output_path = output_path / ("proc_" + job.proc_id + ".csv");
    output_path = std::filesystem::absolute(output_path);

    // Check if the file already exists
    if (std::filesystem::exists(output_path))
    {
        if(force){
            std::filesystem::remove(output_path);
        } else {
            raise_error("Error: output file already exists: \"" + output_path.string() + "\"");
        }
    }

    LOG(output_path);
}

void JobReport::get_job_name()
{
    std::string tmp = job.job_id + "_" + job.proc_id + "_" + job.step_id;
    if (tmp.size() > 63)
    {
        std::cerr << "WARNING: string ${SLURM_JOB_ID}_${SLURM_PROC_ID}_${SLURM_STEP_ID} exceeds the safe \
                      limit of 63 characters allowed for job name.\n";
        tmp.resize(63);
    }
    std::strncpy(job_name, tmp.c_str(), tmp.size());
    job_name[tmp.size()] = '\0';

    LOG("Job name: " + std::string(job_name));
}

void JobReport::compute_time_params(const std::string &time_string)
{
    // If a time limit is set via command line argument, use it
    if (time_string != "")
    {
        max_runtime = parse_time(time_string);
    }
    else if (job.time_limit != 0) // Otherwise, try to use the time limit from the SLURM job
    {
        max_runtime = job.time_limit;
    }
    else
    {
        max_runtime = 24 * 60 * 60; // Default to 24 hours if no time limit is set
    }

    // If the sampling time is not set, set it such that we have at least 100 samples
    if (sampling_time == 0)
    {
        if (max_runtime < 10) 
        {
            sampling_time = max_runtime * 10000; // max_runtime/100 in usec
        }
	else
        {
            sampling_time = 100000;
        }
    }
}

void JobReport::cleanup()
{
    print_root("Cleaning up...");

    if (!job.step_gpus.empty())
    {
        dcgmGroupDestroy(dcgmHandle, group);
    }

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

void JobReport::initialize_gpu_group()
{
    if (job.step_gpus.empty())
    {
        print_root("Unable to determine the number of GPUs per task.\n"
                   "Falling back to all GPUs on node.");

        group = (dcgmGpuGrp_t)DCGM_GROUP_ALL_GPUS;
    } else {
        dcgmReturn_t result = dcgmGroupCreate(dcgmHandle, DCGM_GROUP_EMPTY, job_name, &group);
        check_error(result, "A fatal error occurred while creating the GPU group.");

        // add the GPUs to the group
        for (auto &gpu : job.step_gpus)
        {
            result = dcgmGroupAddDevice(dcgmHandle, group, gpu);
            check_error(result, "A fatal error occurred while adding a GPU to the group.");
        }
    }
}

void JobReport::write_job_stats()
{
    std::ofstream ofs(output_path);
    DataFrame df(jobInfo, job);
    df.dump(ofs);
    ofs.close();
}

void JobReport::start_job_stats()
{
    // check_error(dcgmWatchPidFields(dcgmHandle, group, sampling_time, max_runtime, 0), "Error setting PID watches.");
    LOG(
        "Starting job stats with the following parameters:" << std::endl
                                                            << "Sampling time: " << sampling_time << std::endl
                                                            << "Max runtime: " << max_runtime << std::endl
                                                            << "Job name: " << job_name << std::endl);
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
    if (!job.node_root)
    {
        return;
    }

    print_root("Starting job statistics.");

    initialize_dcgm_handle();
    start_job_stats();
}

void JobReport::stop()
{
    if (!job.node_root)
    {
        return;
    }

    print_root("Stopping job statistics.");

    initialize_dcgm_handle();
    stop_job_stats();
}

void JobReport::run(const std::string &cmd) {
    // Start Job Stats
    if (job.node_root) {
        initialize_dcgm_handle();
        initialize_gpu_group();
        start_job_stats();
    }

    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, nullptr);

    child_pid = fork();
    if (child_pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)nullptr);
        // If execl returns, there was an error
        raise_error("execl failed to execute command. Is /bin/sh available?");
    } else if (child_pid > 0) {
        // Parent process
        int status;
        waitpid(child_pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            std::cerr << "Warning: workload \"" << cmd << "\" returned non-zero exit code: " << WEXITSTATUS(status) << std::endl;
            // raise_error("Workload returned non-zero exit code.");
        } else if (WIFSIGNALED(status)) {
            std::cerr << "Warning: workload \"" << cmd << "\" was terminated by signal: " << WTERMSIG(status) << std::endl;
            // raise_error("Workload was terminated by signal.");
        }
    } else {
        // Fork failed
        std::cerr << "Failed to fork process for command: " << cmd << std::endl;
    }

    // Reset signal handlers to default
    sa.sa_handler = SIG_DFL;
    sigaction(SIGCHLD, &sa, nullptr);

    // Stop Job Stats
    if (job.node_root) {
        stop_job_stats();
        write_job_stats();
    }
}

#endif // JOBREPORT_HPP
