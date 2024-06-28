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

struct DataFrameAvg
{
    unsigned int n_gpus;
    long long energyConsumed;
    double powerUsageMin;
    double powerUsageMax;
    double powerUsageAvg;
    long long pcieRxBandwidthMin;
    long long pcieRxBandwidthMax;
    long long pcieRxBandwidthAvg;
    long long pcieTxBandwidthMin;
    long long pcieTxBandwidthMax;
    long long pcieTxBandwidthAvg;
    long long pcieReplays;
    long long startTime;
    long long endTime;
    int smUtilizationMin;
    int smUtilizationMax;
    int smUtilizationAvg;
    int memoryUtilizationMin;
    int memoryUtilizationMax;
    int memoryUtilizationAvg;

    DataFrameAvg() = default;
};


class DataFrame
{
public:
    // Constructors
    DataFrame() = default;
    DataFrame(const dcgmJobInfo_t &jobInfo, const SlurmJob& job);

    // Columns
    DFColumn<unsigned int> gpuId;
    DFColumn<long long> energyConsumed;
    DFColumn<double> powerUsageMin;
    DFColumn<double> powerUsageMax;
    DFColumn<double> powerUsageAvg;
    DFColumn<long long> pcieRxBandwidthMin;
    DFColumn<long long> pcieRxBandwidthMax;
    DFColumn<long long> pcieRxBandwidthAvg;
    DFColumn<long long> pcieTxBandwidthMin;
    DFColumn<long long> pcieTxBandwidthMax;
    DFColumn<long long> pcieTxBandwidthAvg;
    DFColumn<long long> pcieReplays;
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

    // Reserve memory for the DataFrame
    void reserve(unsigned int n);
};

DataFrame::DataFrame(const dcgmJobInfo_t &jobInfo, const SlurmJob& job)
{
    unsigned int n = jobInfo.numGpus;
    reserve(n); // Reserve all columns to have n elements

    unsigned int base_id = job.n_gpus > 0 ? std::stoi(job.proc_id) * job.n_gpus : std::stoi(job.proc_id) * job.n_tasks_per_node;
    for (unsigned int id = 0; id < n; ++id)
    {
        gpuId.push_back(base_id + id);
        energyConsumed.push_back(jobInfo.gpus[id].energyConsumed);
        powerUsageMin.push_back(jobInfo.gpus[id].powerUsage.minValue);
        powerUsageMax.push_back(jobInfo.gpus[id].powerUsage.maxValue);
        powerUsageAvg.push_back(jobInfo.gpus[id].powerUsage.average);
        pcieRxBandwidthMin.push_back(jobInfo.gpus[id].pcieRxBandwidth.minValue);
        pcieRxBandwidthMax.push_back(jobInfo.gpus[id].pcieRxBandwidth.maxValue);
        pcieRxBandwidthAvg.push_back(jobInfo.gpus[id].pcieRxBandwidth.average);
        pcieTxBandwidthMin.push_back(jobInfo.gpus[id].pcieTxBandwidth.minValue);
        pcieTxBandwidthMax.push_back(jobInfo.gpus[id].pcieTxBandwidth.maxValue);
        pcieTxBandwidthAvg.push_back(jobInfo.gpus[id].pcieTxBandwidth.average);
        pcieReplays.push_back(jobInfo.gpus[id].pcieReplays);
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

void DataFrame::reserve(unsigned int n)
{
    gpuId.reserve(n);
    energyConsumed.reserve(n);
    powerUsageMin.reserve(n);
    powerUsageMax.reserve(n);
    powerUsageAvg.reserve(n);
    pcieRxBandwidthMin.reserve(n);
    pcieRxBandwidthMax.reserve(n);
    pcieRxBandwidthAvg.reserve(n);
    pcieTxBandwidthMin.reserve(n);
    pcieTxBandwidthMax.reserve(n);
    pcieTxBandwidthAvg.reserve(n);
    pcieReplays.reserve(n);
    startTime.reserve(n);
    endTime.reserve(n);
    smUtilizationMin.reserve(n);
    smUtilizationMax.reserve(n);
    smUtilizationAvg.reserve(n);
    memoryUtilizationMin.reserve(n);
    memoryUtilizationMax.reserve(n);
    memoryUtilizationAvg.reserve(n);
}

void DataFrame::sort_by_gpu_id()
{
    // Create a vector of indices
    std::vector<size_t> indices(gpuId.size());
    std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, ..., n-1

    // Sort indices based on the values in the gpuId DFColumn
    std::sort(indices.begin(), indices.end(),
              [this](size_t i1, size_t i2)
              { return gpuId[i1] < gpuId[i2]; });

    // Apply the sorted order to each DFColumn
    gpuId.permute(indices);
    energyConsumed.permute(indices);
    powerUsageMin.permute(indices);
    powerUsageMax.permute(indices);
    powerUsageAvg.permute(indices);
    pcieRxBandwidthMin.permute(indices);
    pcieRxBandwidthMax.permute(indices);
    pcieRxBandwidthAvg.permute(indices);
    pcieTxBandwidthMin.permute(indices);
    pcieTxBandwidthMax.permute(indices);
    pcieTxBandwidthAvg.permute(indices);
    pcieReplays.permute(indices);
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
    DataFrameAvg avg;
    avg.n_gpus = gpuId.size();
    avg.energyConsumed = energyConsumed.average();
    avg.powerUsageMin = powerUsageMin.average();
    avg.powerUsageMax = powerUsageMax.average();
    avg.powerUsageAvg = powerUsageAvg.average();
    avg.pcieRxBandwidthMin = pcieRxBandwidthMin.average();
    avg.pcieRxBandwidthMax = pcieRxBandwidthMax.average();
    avg.pcieRxBandwidthAvg = pcieRxBandwidthAvg.average();
    avg.pcieTxBandwidthMin = pcieTxBandwidthMin.average();
    avg.pcieTxBandwidthMax = pcieTxBandwidthMax.average();
    avg.pcieTxBandwidthAvg = pcieTxBandwidthAvg.average();
    avg.pcieReplays = pcieReplays.average();
    avg.startTime = startTime.average();
    avg.endTime = endTime.average();
    avg.smUtilizationMin = smUtilizationMin.average();
    avg.smUtilizationMax = smUtilizationMax.average();
    avg.smUtilizationAvg = smUtilizationAvg.average();
    avg.memoryUtilizationMin = memoryUtilizationMin.average();
    avg.memoryUtilizationMax = memoryUtilizationMax.average();
    avg.memoryUtilizationAvg = memoryUtilizationAvg.average();
    return avg;
}

void DataFrame::dump(std::ofstream &os)
{
    size_t numRows = gpuId.size();

    // Write the header
    os << "gpuId,energyConsumed,powerUsageMin,powerUsageMax,powerUsageAvg,"
       << "pcieRxBandwidthMin,pcieRxBandwidthMax,pcieRxBandwidthAvg,"
       << "pcieTxBandwidthMin,pcieTxBandwidthMax,pcieTxBandwidthAvg,"
       << "pcieReplays,startTime,endTime,"
       << "smUtilizationMin,smUtilizationMax,smUtilizationAvg,"
       << "memoryUtilizationMin,memoryUtilizationMax,memoryUtilizationAvg\n";

    // Write the data
    for (size_t i = 0; i < numRows; ++i)
    {
        os << gpuId[i] << ',' 
           << energyConsumed[i] << ',' 
           << powerUsageMin[i] << ',' 
           << powerUsageMax[i] << ',' 
           << powerUsageAvg[i] << ','
           << pcieRxBandwidthMin[i] << ',' 
           << pcieRxBandwidthMax[i] << ',' 
           << pcieRxBandwidthAvg[i] << ','
           << pcieTxBandwidthMin[i] << ',' 
           << pcieTxBandwidthMax[i] << ',' 
           << pcieTxBandwidthAvg[i] << ','
           << pcieReplays[i] << ','
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
        gpuId.push_back(std::stoi(value));

        std::getline(ss, value, ',');
        energyConsumed.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        powerUsageMin.push_back(std::stod(value));

        std::getline(ss, value, ',');
        powerUsageMax.push_back(std::stod(value));

        std::getline(ss, value, ',');
        powerUsageAvg.push_back(std::stod(value));

        std::getline(ss, value, ',');
        pcieRxBandwidthMin.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieRxBandwidthMax.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieRxBandwidthAvg.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieTxBandwidthMin.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieTxBandwidthMax.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieTxBandwidthAvg.push_back(std::stoll(value));

        std::getline(ss, value, ',');
        pcieReplays.push_back(std::stoll(value));

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