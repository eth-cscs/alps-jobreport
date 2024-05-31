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

    template <typename T>
    T get(const std::string &key) const;

    void read_slurm_env();

    // template <typename T>
    // T get(const char* key) const {
    //     return get(std::string(key));
    // };

private:
    // Variables
    std::unordered_map<std::string, std::string> vars;
    std::string read_env_var(const std::string &var);
};

template <typename T>
T SlurmJob::get(const std::string &key) const
{
    // Look for key in vars
    auto it = vars.find(key);
    if (it == vars.end())
    {
        raise_error("Error: environment variable " + key + " not found in SlurmJob object.");
    }

    // Check if the value can be converted to the desired type
    std::istringstream iss(it->second);
    T value;
    char c;
    if (!(iss >> value) || iss.get(c))
    {
        raise_error("Error: environment variable " + key + " cannot be converted to the desired type.");
    }

    return value;
}

// Specialized type for string
template <>
std::string SlurmJob::get<std::string>(const std::string &key) const
{
    // Look for key in vars
    auto it = vars.find(key);
    if (it == vars.end())
    {
        raise_error("Error: environment variable " + key + " not found in SlurmJob object.");
    }

    return it->second;
}

std::string SlurmJob::read_env_var(const std::string &var)
{
    const char *env = std::getenv(var.c_str());
    if (env == nullptr)
    {
        raise_error("Error: unable to read environment variable " + var);
    }
    // Convert the environment variable to a string
    std::string value = env;
    return value;
}

void SlurmJob::read_slurm_env()
{
    // Read all the slurm environment variables
    // and store them in the class
    // SLURM_JOB_ID
    try
    {
        vars["job_id"] = read_env_var("SLURM_JOB_ID");
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
    }

    // SLURM_PROCID
    try
    {
        vars["proc_id"] = read_env_var("SLURM_PROCID");
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
    }

    // SLURM_NTASKS_PER_NODE
    try
    {
        vars["n_tasks_per_node"] = read_env_var("SLURM_NTASKS_PER_NODE");
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
    }

    // SLURM_GPUS_PER_TASK
    try
    {
        vars["gpus_per_task"] = read_env_var("SLURM_GPUS_PER_TASK");
    }
    catch (const std::runtime_error &e)
    {
        // Fall back to -1 if the environment variable is not set
        vars["gpus_per_task"] = "";
    }
}

#endif // SLURM_HPP