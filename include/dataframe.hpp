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

struct DataFrameAvg
{

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
    DataFrame(const dcgmJobInfo_t &jobInfo);

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

    // Resize the DataFrame
    void resize(unsigned int n);
};

DataFrame::DataFrame(const dcgmJobInfo_t &jobInfo)
{
    unsigned int n = jobInfo.numGpus;
    resize(n); // Resize all columns to have n elements

    for (unsigned int id = 0; id < n; ++id)
    {
        gpuId.push_back(id);
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

void DataFrame::resize(unsigned int n)
{
    gpuId.resize(n);
    energyConsumed.resize(n);
    powerUsageMin.resize(n);
    powerUsageMax.resize(n);
    powerUsageAvg.resize(n);
    pcieRxBandwidthMin.resize(n);
    pcieRxBandwidthMax.resize(n);
    pcieRxBandwidthAvg.resize(n);
    pcieTxBandwidthMin.resize(n);
    pcieTxBandwidthMax.resize(n);
    pcieTxBandwidthAvg.resize(n);
    pcieReplays.resize(n);
    startTime.resize(n);
    endTime.resize(n);
    smUtilizationMin.resize(n);
    smUtilizationMax.resize(n);
    smUtilizationAvg.resize(n);
    memoryUtilizationMin.resize(n);
    memoryUtilizationMax.resize(n);
    memoryUtilizationAvg.resize(n);
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

// Load the DataFrame from a file
void DataFrame::dump(std::ofstream &os)
{
    size_t numRows = gpuId.size();
    for (size_t i = 0; i < numRows; ++i)
    {
        gpuId.write(os, i);
        energyConsumed.write(os, i);
        powerUsageMin.write(os, i);
        powerUsageMax.write(os, i);
        powerUsageAvg.write(os, i);
        pcieRxBandwidthMin.write(os, i);
        pcieRxBandwidthMax.write(os, i);
        pcieRxBandwidthAvg.write(os, i);
        pcieTxBandwidthMin.write(os, i);
        pcieTxBandwidthMax.write(os, i);
        pcieTxBandwidthAvg.write(os, i);
        pcieReplays.write(os, i);
        startTime.write(os, i);
        endTime.write(os, i);
        smUtilizationMin.write(os, i);
        smUtilizationMax.write(os, i);
        smUtilizationAvg.write(os, i);
        memoryUtilizationMin.write(os, i);
        memoryUtilizationMax.write(os, i);
        memoryUtilizationAvg.write(os, i);
    }
}

void DataFrame::load(std::ifstream &is)
{
    while (!is.eof())
    {
        gpuId.read(is);
        energyConsumed.read(is);
        powerUsageMin.read(is);
        powerUsageMax.read(is);
        powerUsageAvg.read(is);
        pcieRxBandwidthMin.read(is);
        pcieRxBandwidthMax.read(is);
        pcieRxBandwidthAvg.read(is);
        pcieTxBandwidthMin.read(is);
        pcieTxBandwidthMax.read(is);
        pcieTxBandwidthAvg.read(is);
        pcieReplays.read(is);
        startTime.read(is);
        endTime.read(is);
        smUtilizationMin.read(is);
        smUtilizationMax.read(is);
        smUtilizationAvg.read(is);
        memoryUtilizationMin.read(is);
        memoryUtilizationMax.read(is);
        memoryUtilizationAvg.read(is);
    }
}

#endif // JOBREPORT_DATAFRAME_HPP