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
    Status read_slurm_env();

    // Public variables
    std::string job_id = "";
    std::string proc_id = "";
    std::string step_id = "";
    std::string step_gpus = "";

    unsigned int n_tasks_per_node = 0;
    unsigned int gpus_per_task = 0;
    unsigned int n_nodes = 0;
    unsigned int n_procs = 0;
    unsigned int n_gpus = 0;
    bool root = false;
    bool node_root = false;

    // Debugging
    void print_vars();
    
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

Status SlurmJob::read_slurm_env()
{
    // Read all the slurm environment variables
    if(read_env_var(job_id, "SLURM_JOB_ID") != Status::Success)
        raise_error("SLURM_JOB_ID not found");
    
    if(read_env_var(proc_id, "SLURM_PROCID") != Status::Success)
        raise_error("SLURM_PROCID not found");

    if(read_env_var(step_id, "SLURM_STEPID") != Status::Success)
        raise_error("SLURM_STEPID not found");

    if(read_env_var(step_gpus, "SLURM_STEP_GPUS") != Status::Success)
        std::cout << "Warning: unable to read SLURM_GPUS_PER_TASK" << std::endl
                  << "Consider passing --gpus-per-task <x> in your job script." << std::endl;
    
    if(read_env_var(n_tasks_per_node, "SLURM_TASKS_PER_NODE") != Status::Success)
        std::cout << "Warning: unable to read SLURM_TASKS_PER_NODE" << std::endl
                  << "Consider passing --ntasks-per-node <x> in your job script." << std::endl;

    // if(read_env_var(gpus_per_task, "SLURM_GPUS_PER_TASK") != Status::Success)
    //     std::cout << "Warning: unable to read SLURM_GPUS_PER_TASK" << std::endl
    //               << "Consider passing --gpus-per-task <x> in your job script." << std::endl;

    // Backup values used to determine the number of nodes and processors
    // if SLURM_TASKS_PER_NODE is not set
    read_env_var(n_nodes, "SLURM_NNODES");
    read_env_var(n_procs, "SLURM_NPROCS");
    read_env_var(n_gpus, "SLURM_NGPUS");

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
    root = (std::stoul(proc_id) == 0);

    return Status::Success;
}

void SlurmJob::print_vars()
{
    std::cout << "SLURM_JOB_ID: " << job_id << std::endl;
    std::cout << "SLURM_PROCID: " << proc_id << std::endl;
    std::cout << "SLURM_NTASKS_PER_NODE: " << n_tasks_per_node << std::endl;
    std::cout << "SLURM_GPUS_PER_TASK: " << gpus_per_task << std::endl;
    std::cout << "SLURM_NNODES: " << n_nodes << std::endl;
    std::cout << "SLURM_NPROCS: " << n_procs << std::endl;
    std::cout << "SLURM_NGPUS: " << n_gpus << std::endl;
    std::cout << "Root process: " << root << std::endl;
}

#endif // SLURM_HPP