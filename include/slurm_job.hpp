/*
    This class is responsible for reading all the information
    from the slurm environment and storing it in a class.
*/

#ifndef JOBREPORT_SLURM_HPP
#define JOBREPORT_SLURM_HPP

#include <string>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "utils.hpp"

class SlurmJob
{
public:
    SlurmJob() = default;
    Status read_slurm_env(bool ignore_gpu_binding, bool verbose);

    // Public variables
    std::string user = "unknown_user";
    std::string account = "unknown_account";
    std::string job_id = "";
    std::string proc_id = "";
    std::string step_id = "";
    std::vector<unsigned int> step_gpus;

    unsigned int n_tasks_per_node = 0;
    unsigned int gpus_per_task = 0;
    unsigned int n_nodes = 0;
    unsigned int n_procs = 0;
    unsigned int time_limit = 0;
    bool root = false;
    bool node_root = false;

    // Debugging
    void print_vars();
    void print_root(const std::string& msg){
        if(root) std::cout << msg << std::endl;
    };
    
private:
    template <typename T>
    Status read_env_var(T& rax, const std::string& var);
    
};

template <typename T>
Status SlurmJob::read_env_var(T& rax, const std::string& var)
{
    const char *env = std::getenv(var.c_str());
    
    if (env == nullptr)
    {
        return Status::Error;
    }

    // Convert the environment variable to a string
    std::string value = env;
    
    // Convert the string to the desired type
    T return_value;
    std::istringstream(value) >> rax;
    
    return Status::Success;
}

Status SlurmJob::read_slurm_env(bool ignore_gpu_binding, bool verbose)
{
    // Read all the slurm environment variables
    if(read_env_var(job_id, "SLURM_JOB_ID") != Status::Success)
        raise_error("SLURM_JOB_ID not found");
    
    if(read_env_var(proc_id, "SLURM_PROCID") != Status::Success)
        raise_error("SLURM_PROCID not found");

    if(read_env_var(step_id, "SLURM_STEPID") != Status::Success)
        raise_error("SLURM_STEPID not found");

    root = (std::stoi(proc_id) == 0);

    // Read the number of GPUs per task only if the user wants to use them
    if(!ignore_gpu_binding){
        std::string step_gpus_str = "";
        if(read_env_var(step_gpus_str, "SLURM_STEP_GPUS") != Status::Success) {
            print_root("Warning: unable to read SLURM_GPUS_PER_TASK\n"
                    "Consider passing --gpus-per-task <x> in your job script.");
        } else {
            // split the comma-separated string into a vector of unsigned integers
            std::stringstream ss(step_gpus_str);
            std::string gpuId;
            while (std::getline(ss, gpuId, ','))
            {
                step_gpus.push_back(std::stoi(gpuId));
            }
        }
    }
    
    if(read_env_var(n_tasks_per_node, "SLURM_TASKS_PER_NODE") != Status::Success)
        print_root("Warning: unable to read SLURM_TASKS_PER_NODE\n"
                   "Consider passing --ntasks-per-node <x> in your job script.");

    // Read time limit if possible
    unsigned int start_time = 0;
    read_env_var(start_time, "SLURM_JOB_START_TIME");
    
    unsigned int end_time = 0;
    read_env_var(end_time, "SLURM_JOB_END_TIME");
    
    time_limit = end_time - start_time;

    // Read the user and account
    read_env_var(user, "USER");
    read_env_var(account, "SLURM_JOB_ACCOUNT");
    
    // Backup values used to determine the number of nodes and processors
    // if SLURM_TASKS_PER_NODE is not set
    read_env_var(n_nodes, "SLURM_NNODES");
    read_env_var(n_procs, "SLURM_NPROCS");

    // Determine the number of processes per node
    // Only needed if SLURM_STEP_GPUS is not set
    if(step_gpus.empty() && n_tasks_per_node == 0)
    {
        if(n_procs != 0 && n_nodes != 0) {
            n_tasks_per_node = n_procs / n_nodes;
        } else {
            raise_error("Unable to determine the number of tasks running on each node.");
        }
    } 
        
    // Determine if the current process is the root process on its node
    // A process is considered the root process of a node if
    node_root = !step_gpus.empty() || (std::stoul(proc_id) % n_tasks_per_node == 0);

    return Status::Success;
}

#endif // SLURM_HPP