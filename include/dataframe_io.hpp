#ifndef JOBREPORT_DATAFRAME_IO_HPP
#define JOBREPORT_DATAFRAME_IO_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath>

#include "third_party/tabulate/tabulate.hpp"
#include "dataframe.hpp"

std::string format_bytes(long long val)
{
    std::vector<std::string> suffixes = {"B", "KiB", "MiB", "GiB", "TiB"};
    int suffix_index = 0;
    double size = static_cast<double>(val); // Use double for precise division

    while (size >= 1024 && suffix_index < suffixes.size() - 1)
    {
        size /= 1024;
        suffix_index++;
    }

    // Format the size to one decimal place
    char formatted_size[20];
    snprintf(formatted_size, sizeof(formatted_size), "%.1f", size);

    return std::string(formatted_size) + " " + suffixes[suffix_index];
}

std::string format_power(double val)
{
    std::ostringstream oss;
    if (val > 1000)
    {
        oss << std::fixed << std::setprecision(1) << (val / 1000) << "kW";
    }
    else
    {
        oss << std::fixed << std::setprecision(1) << val << "W";
    }
    return oss.str();
}

std::string format_energy(double val)
{
    std::ostringstream oss;
    if (val >= 1e9) // GWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1e9) << "GWh";
    }
    else if (val >= 1e6) // MWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1e6) << "MWh";
    }
    else if (val >= 1000) // kWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1000) << "kWh";
    }
    else
    {
        oss << std::fixed << std::setprecision(1) << val << "Wh";
    }
    return oss.str();
}

std::string format_elapsed(long long val)
{
    // Format elapsed time in seconds to DD-HH:MM:SS
    long long days = val / 86400;
    long long hours = (val % 86400) / 3600;
    long long minutes = (val % 3600) / 60;
    long long seconds = val % 60;

    std::ostringstream oss;
    if (days > 0)
    {
        oss << days << "d ";
    }

    if (hours > 0 || days > 0)
    {
        oss << hours << "h ";
    }

    if (minutes > 0 || hours > 0 || days > 0)
    {
        oss << minutes << "m ";
    }

    oss << seconds << "s";

    return oss.str();
}

std::string format_date(long long timestamp)
{
    // Convert the timestamp to a time_point (assuming the input is in microseconds)
    std::chrono::microseconds us{timestamp};
    std::chrono::time_point<std::chrono::system_clock> tp{us};

    // Convert time_point to std::time_t
    std::time_t time = std::chrono::system_clock::to_time_t(tp);

    // Format the time to a human-readable string
    std::tm tm = *std::localtime(&time); // or use std::gmtime(&time) for UTC

    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
    return oss.str();
}

// Output stream operator for DataFrameAvg
std::ostream &operator<<(std::ostream &os, const DataFrameAvg &df)
{
    tabulate::Table table;
    table.add_row(tabulate::Table::Row_t{"Number of GPUs", std::to_string(df.n_gpus)});
    table.add_row(tabulate::Table::Row_t{"Total Energy Consumed", format_energy(df.n_gpus *
                                                                                df.powerUsageAvg / 3600. *
                                                                                (df.endTime - df.startTime) / 1e6)
                                        });

    table.add_row(tabulate::Table::Row_t{"Power Usage (avg/min/max)",
                                         format_power(df.powerUsageAvg) + " / " +
                                             format_power(df.powerUsageMin) + " / " +
                                             format_power(df.powerUsageMax)});

    // table.add_row(tabulate::Table::Row_t{"pcieRxBandwidth (avg/min/max)",
    //                                      format_bytes(df.pcieRxBandwidthAvg) + " / " +
    //                                          format_bytes(df.pcieRxBandwidthMin) + " / " +
    //                                          format_bytes(df.pcieRxBandwidthMax)});

    // table.add_row(tabulate::Table::Row_t{"pcieTxBandwidth (avg/min/max)",
    //                                      format_bytes(df.pcieTxBandwidthAvg) + " / " +
    //                                          format_bytes(df.pcieTxBandwidthMin) + " / " +
    //                                          format_bytes(df.pcieTxBandwidthMax)});

    table.add_row(tabulate::Table::Row_t{"Start Time", format_date(df.startTime)});

    table.add_row(tabulate::Table::Row_t{"End Time", format_date(df.endTime)});

    table.add_row(tabulate::Table::Row_t{"Elapsed Time (s)", format_elapsed((df.endTime - df.startTime) / 1000000)}); // Convert us to s

    table.add_row(tabulate::Table::Row_t{"SM Utilization % (avg/min/max)",
                                         std::to_string(df.smUtilizationAvg) + " / " +
                                             std::to_string(df.smUtilizationMin) + " / " +
                                             std::to_string(df.smUtilizationMax)});

    table.add_row(tabulate::Table::Row_t{"Memory Utilization % (avg/min/max)",
                                         std::to_string(df.memoryUtilizationAvg) + " / " +
                                             std::to_string(df.memoryUtilizationMin) + " / " +
                                             std::to_string(df.memoryUtilizationMax)});

    table.format()
        .border_top("-")
        .border_bottom("-")
        .border_left("|")
        .border_right("|")
        .corner("+");

    // Set a fixed width for each header column and enable text wrapping
    table[0].format().width(40);

    os << table << std::endl;

    return os;
}

// Output stream operator for DataFrame
std::ostream &operator<<(std::ostream &os, const DataFrame &df)
{
    tabulate::Table table;

    // Add header row
    table.add_row({"GPU",
                   //"energyConsumed",
                   //"powerUsage (avg/min/max)",
                   //"pcieRxBandwidth (avg/min/max)",
                   //"pcieTxBandwidth (avg/min/max)",
                   //"startTime",
                   //"endTime",
                   "Elapsed (s)",
                   "Utilization %\n(avg/min/max)",
                   "Memory Utilization %\n(avg/min/max)"});

    size_t num_rows = df.gpuId.size();
    for (size_t i = 0; i < num_rows; ++i)
    {
        table.add_row(tabulate::Table::Row_t{
            std::to_string(df.gpuId[i]),
            /*
            This is commented out to make the table more readable
            but all the data is still available in the DataFrame
            std::to_string(df.energyConsumed[i]),
            std::to_string(df.powerUsageAvg[i]) + " / " +
            std::to_string(df.powerUsageMin[i]) + " / " +
            std::to_string(df.powerUsageMax[i]),
            std::to_string(df.pcieRxBandwidthAvg[i]) + " / " +
            std::to_string(df.pcieRxBandwidthMin[i]) + " / " +
            std::to_string(df.pcieRxBandwidthMax[i]),
            std::to_string(df.pcieTxBandwidthAvg[i]) + " / " +
            std::to_string(df.pcieTxBandwidthMin[i]) + " / " +
            std::to_string(df.pcieTxBandwidthMax[i]),
            std::to_string(df.startTime[i]),
            std::to_string(df.endTime[i]), */
            format_elapsed((df.endTime[i] - df.startTime[i]) / 1000000),
            std::to_string(df.smUtilizationAvg[i]) + " / " +
                std::to_string(df.smUtilizationMin[i]) + " / " +
                std::to_string(df.smUtilizationMax[i]),
            std::to_string(df.memoryUtilizationAvg[i]) + " / " +
                std::to_string(df.memoryUtilizationMin[i]) + " / " +
                std::to_string(df.memoryUtilizationMax[i])});
    }

    // Enable multi-byte character support
    table.format().multi_byte_characters(true);

    // Format all rows
    table.format()
        .border_left("|")
        .border_right("|")
        .border_bottom("")
        .border_top("")
        .corner("");

    // Format header row
    table[0].format().border_top("-").corner("+");

    // Format first row
    table[1].format().border_top("-").corner_top_left("+").corner_top_right("+");

    // Format last row
    table[num_rows].format().border_bottom("-").corner_bottom_left("+").corner_bottom_right("+");

    // Set a fixed width for each column and enable text wrapping
    table[0][0].format().width(9);  // GPU ID
    table[0][1].format().width(23); // Elapsed time
    table[0][2].format().width(23); // Utilization
    table[0][3].format().width(23); // Memory utilization

    // Print the table
    os << table << std::endl;

    return os;
}

DataFrame load_dataframe(const std::string &input)
{
    DataFrame df;
    std::filesystem::path target(input);

    // Check if the target exists
    if (!std::filesystem::exists(target))
    {
        raise_error("File not found: \"" + input + "\"");
    }

    // Check if the target is a regular file
    if (std::filesystem::is_regular_file(target))
    {
        // Read file into DataFrame
        std::ifstream ifs(target);
        df.load(ifs);
        ifs.close();
    }
    // Target is a directory
    else if (std::filesystem::is_directory(target))
    {
        // Iterate over all files in the directory
        for (const auto &entry : std::filesystem::directory_iterator(target))
        {
            // Check if the entry is a regular file
            if (entry.is_regular_file())
            {
                // Read file into DataFrame
                std::ifstream ifs(entry.path());
                // Load the data from the file into the DataFrame
                // this operation will append the data to the existing DataFrame
                df.load(ifs);
                ifs.close();
            }
        }
    }
    else
    {
        raise_error("An unknown error occurred while reading the input profiling data. Are you sure the file or directory exist?");
    }

    // Sort DataFrame by GPU ID
    df.sort_by_gpu_id();

    return df;
}

#endif