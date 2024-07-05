#ifndef JOBREPORT_DATAFRAME_HPP
#define JOBREPORT_DATAFRAME_HPP

#define ALIGN_WIDTH 40
#define ALIGN_VALUE 10

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <iomanip>

#include "dcgm_structs.h"
#include "column.hpp"
#include "slurm_job.hpp"
#include "utils.hpp"

struct DataFrameAvg
{
    std::string user;
    std::string account;
    unsigned int jobId;
    unsigned int stepId;
    unsigned int nNodes;
    unsigned int nGpus;
    double powerUsageAvg;
    double energyConsumed;
    long long startTime;
    long long endTime;
    int smUtilizationAvg;
    int memoryUtilizationAvg;

    DataFrameAvg() = default;
};


class DataFrame
{
public:
    // Constructors
    DataFrame() = default;
    DataFrame(const dcgmJobInfo_t &jobInfo, const SlurmJob &job);

    // Columns
    DFColumn<std::string> user;
    DFColumn<std::string> account;
    DFColumn<unsigned int> jobId;
    DFColumn<unsigned int> stepId;
    DFColumn<unsigned int> nNodes;
    DFColumn<std::string> host;
    DFColumn<unsigned int> gpuId;
    DFColumn<double> powerUsageMin;
    DFColumn<double> powerUsageMax;
    DFColumn<double> powerUsageAvg;
    DFColumn<long long> startTime;
    DFColumn<long long> endTime;
    DFColumn<int> smUtilizationMin;
    DFColumn<int> smUtilizationMax;
    DFColumn<int> smUtilizationAvg;
    DFColumn<int> memoryUtilizationMin;
    DFColumn<int> memoryUtilizationMax;
    DFColumn<int> memoryUtilizationAvg;

    // Input/Output functions
    void dump(std::ofstream &os);
    void load(std::ifstream &is);

    // Data manipulation functions
    void sort_by_gpu_id();
    DataFrameAvg average();
};

DataFrame::DataFrame(const dcgmJobInfo_t &jobInfo, const SlurmJob &job)
{
    unsigned int n = jobInfo.numGpus;

    std::string hostName = get_hostname();
    
    unsigned int _job_id = std::stoul(job.job_id);
    unsigned int _step_id = std::stoul(job.step_id);

    for (unsigned int id = 0; id < n; ++id)
    {
        user.push_back(job.user);
        account.push_back(job.account);
        jobId.push_back(_job_id);
        stepId.push_back(_step_id);
        nNodes.push_back(job.n_nodes);
        host.push_back(hostName);
        gpuId.push_back(job.step_gpus.empty() ? id : job.step_gpus[id]);
        
        // This is a hack to handle the case where the power usage is not available
        constexpr double thrsh = 1.5e3;
        double tmp;
        tmp = jobInfo.gpus[id].powerUsage.minValue;
        powerUsageMin.push_back(tmp < thrsh ? tmp : std::numeric_limits<double>::quiet_NaN());
        tmp = jobInfo.gpus[id].powerUsage.maxValue;
        powerUsageMax.push_back(tmp < thrsh ? tmp : std::numeric_limits<double>::quiet_NaN());
        tmp = jobInfo.gpus[id].powerUsage.average;
        powerUsageAvg.push_back(tmp < thrsh ? tmp : std::numeric_limits<double>::quiet_NaN());

        startTime.push_back(jobInfo.gpus[id].startTime);
        endTime.push_back(jobInfo.gpus[id].endTime);
        smUtilizationMin.push_back(jobInfo.gpus[id].smUtilization.minValue);
        smUtilizationMax.push_back(jobInfo.gpus[id].smUtilization.maxValue);
        smUtilizationAvg.push_back(jobInfo.gpus[id].smUtilization.average);
        memoryUtilizationMin.push_back(jobInfo.gpus[id].memoryUtilization.minValue);
        memoryUtilizationMax.push_back(jobInfo.gpus[id].memoryUtilization.maxValue);
        memoryUtilizationAvg.push_back(jobInfo.gpus[id].memoryUtilization.average);
    }
}

void DataFrame::sort_by_gpu_id()
{
    // Create a vector of indices
    std::vector<size_t> indices(gpuId.size());
    std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, ..., n-1

    // Sort the indices based on the host name and then the GPU ID
    std::sort(indices.begin(), indices.end(),
              [this](size_t i1, size_t i2) {
                  if (host[i1] == host[i2]) {
                      return gpuId[i1] < gpuId[i2];
                  }
                  return host[i1] < host[i2];
              });

    // Apply the sorted order to each DFColumn
    user.permute(indices);
    account.permute(indices);
    jobId.permute(indices);
    stepId.permute(indices);
    nNodes.permute(indices);
    host.permute(indices);
    gpuId.permute(indices);
    powerUsageMin.permute(indices);
    powerUsageMax.permute(indices);
    powerUsageAvg.permute(indices);
    startTime.permute(indices);
    endTime.permute(indices);
    smUtilizationMin.permute(indices);
    smUtilizationMax.permute(indices);
    smUtilizationAvg.permute(indices);
    memoryUtilizationMin.permute(indices);
    memoryUtilizationMax.permute(indices);
    memoryUtilizationAvg.permute(indices);
}

DataFrameAvg DataFrame::average()
{
    // Safety check
    // This should ideally never trigger.
    if (gpuId.empty())  
    {
        raise_error("Attempted to average an empty DataFrame.\n"
                    "This is a bug and should be reported.");
    }

    DataFrameAvg avg;
    avg.user = user[0];
    avg.account = account[0];
    avg.jobId = jobId[0];
    avg.stepId = stepId[0];
    avg.nNodes = nNodes[0];
    avg.nGpus = gpuId.size();
    avg.powerUsageAvg = powerUsageAvg.sum();
    avg.startTime = startTime.average();
    avg.endTime = endTime.average();
    avg.energyConsumed = avg.powerUsageAvg/3600. * (avg.endTime - avg.startTime) / 1e6;
    avg.smUtilizationAvg = smUtilizationAvg.average();
    avg.memoryUtilizationAvg = memoryUtilizationAvg.average();
    return avg;
}

void DataFrame::dump(std::ofstream &os)
{
    size_t numRows = gpuId.size();

    // Write the header
    os << "jobId,stepId,"
       << "username,slurm_account,"
       << "n_nodes,host,gpuId,"
       << "powerUsageMin,powerUsageMax,powerUsageAvg,"
       << "startTime,endTime,"
       << "smUtilizationMin,smUtilizationMax,smUtilizationAvg,"
       << "memoryUtilizationMin,memoryUtilizationMax,memoryUtilizationAvg\n";

    // Disable scientific notation
    os << std::fixed << std::setprecision(6);

    // Write the data
    for (size_t i = 0; i < numRows; ++i)
    {
        os << jobId[i] << ','
           << stepId[i] << ','
           << user[i] << ','
           << account[i] << ','
           << nNodes[i] << ','
           << host[i] << ','
           << gpuId[i] << ',' 
           << powerUsageMin[i] << ',' 
           << powerUsageMax[i] << ',' 
           << powerUsageAvg[i] << ','
           << startTime[i] << ',' 
           << endTime[i] << ','
           << smUtilizationMin[i] << ',' 
           << smUtilizationMax[i] << ',' 
           << smUtilizationAvg[i] << ','
           << memoryUtilizationMin[i] << ',' 
           << memoryUtilizationMax[i] << ',' 
           << memoryUtilizationAvg[i] << '\n';
    }
}

void DataFrame::load(std::ifstream &is)
{
    std::string line;
    std::getline(is, line); // Skip header line

    // Clear existing data
    // gpuId.clear();
    // energyConsumed.clear();
    // powerUsageMin.clear();
    // powerUsageMax.clear();
    // powerUsageAvg.clear();
    // pcieRxBandwidthMin.clear();
    // pcieRxBandwidthMax.clear();
    // pcieRxBandwidthAvg.clear();
    // pcieTxBandwidthMin.clear();
    // pcieTxBandwidthMax.clear();
    // pcieTxBandwidthAvg.clear();
    // pcieReplays.clear();
    // startTime.clear();
    // endTime.clear();
    // smUtilizationMin.clear();
    // smUtilizationMax.clear();
    // smUtilizationAvg.clear();
    // memoryUtilizationMin.clear();
    // memoryUtilizationMax.clear();
    // memoryUtilizationAvg.clear();

    while (std::getline(is, line))
    {
        std::stringstream ss(line);
        std::string value;

        // Read each value into the respective column
        std::getline(ss, value, ',');
        jobId.push_back(std::stoul(value));
        
        std::getline(ss, value, ',');
        stepId.push_back(std::stoul(value));
        
        std::getline(ss, value, ',');
        user.push_back(value);

        std::getline(ss, value, ',');
        account.push_back(value);

        std::getline(ss, value, ',');
        nNodes.push_back(std::stoul(value));

        std::getline(ss, value, ',');
        host.push_back(value);

        std::getline(ss, value, ',');
        gpuId.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        powerUsageMin.push_back(std::stod(value));

        std::getline(ss, value, ',');
        powerUsageMax.push_back(std::stod(value));

        std::getline(ss, value, ',');
        powerUsageAvg.push_back(std::stod(value));

        std::getline(ss, value, ',');
        startTime.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        endTime.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        smUtilizationMin.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        smUtilizationMax.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        smUtilizationAvg.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        memoryUtilizationMin.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        memoryUtilizationMax.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        memoryUtilizationAvg.push_back(std::stoi(value));
    }
}

#endif // JOBREPORT_DATAFRAME_HPP