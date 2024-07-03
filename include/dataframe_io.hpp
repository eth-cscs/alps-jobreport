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
#include <regex>
#include "third_party/tabulate/tabulate.hpp"
#include "dataframe.hpp"
#include "macros.hpp"

std::string format_percent_alignment(unsigned int p)
{

    std::string p_text = std::to_string(p);
    constexpr int width = 3;

    if (width <= p_text.size())
    {
        return p_text;
    }

    int padding = width - p_text.size();
    return std::string(padding, ' ') + p_text;
}

std::string format_percent(unsigned int avg, unsigned int min, unsigned int max)
{
    return format_percent_alignment(avg) + " / " + format_percent_alignment(min) + " / " + format_percent_alignment(max);
}

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

std::string format_power_unit(double val)
{
    std::ostringstream oss;
    if (val > 1000)
    {
        oss << std::fixed << std::setprecision(1) << (val / 1000) << " kW";
    }
    else
    {
        oss << std::fixed << std::setprecision(1) << val << " W";
    }
    return oss.str();
}

std::string format_power(double avg, double min, double max)
{
    if (std::isnan(avg) || std::isnan(min) || std::isnan(max))
    {
        return "*Failed to measure*";
    }

    return format_power_unit(avg) + " / " + format_power_unit(min) + " / " + format_power_unit(max);
}

std::string format_energy(double val)
{
    if (std::isnan(val))
    {
        return "*Failed to measure*";
    }

    std::ostringstream oss;
    if (val >= 1e9) // GWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1e9) << " GWh";
    }
    else if (val >= 1e6) // MWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1e6) << " MWh";
    }
    else if (val >= 1000) // kWh
    {
        oss << std::fixed << std::setprecision(1) << (val / 1000) << " kWh";
    }
    else
    {
        oss << std::fixed << std::setprecision(1) << val << " Wh";
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

    table.add_row(tabulate::Table::Row_t{"Job Id", std::to_string(df.jobId)});

    table.add_row(tabulate::Table::Row_t{"Step Id", std::to_string(df.stepId)});

    table.add_row(tabulate::Table::Row_t{"User", df.user});

    table.add_row(tabulate::Table::Row_t{"SLURM Account", df.account});

    table.add_row(tabulate::Table::Row_t{"Start Time", format_date(df.startTime)});

    table.add_row(tabulate::Table::Row_t{"End Time", format_date(df.endTime)});

    table.add_row(tabulate::Table::Row_t{"Elapsed Time", format_elapsed((df.endTime - df.startTime) / 1000000)}); // Convert us to s

    table.add_row(tabulate::Table::Row_t{"Number of Nodes", std::to_string(df.nNodes)});
    
    table.add_row(tabulate::Table::Row_t{"Number of GPUs", std::to_string(df.nGpus)});

    table.add_row(tabulate::Table::Row_t{"Total Energy Consumed", format_energy(df.energyConsumed)});

    table.add_row(tabulate::Table::Row_t{"Total Power Usage",
                                         format_power_unit(df.powerUsageAvg)
                                         });

    table.add_row(tabulate::Table::Row_t{"Average SM Utilization",
                                         std::to_string(df.smUtilizationAvg) + "%"
                                         });

    table.add_row(tabulate::Table::Row_t{"Average Memory Utilization",
                                         std::to_string(df.memoryUtilizationAvg) + "%"
                                         });

    table.format()
        .border_top("-")
        .border_bottom("-")
        .border_left("|")
        .border_right("|")
        .corner("+");

    // Set a fixed width for each header column and enable text wrapping
    table[0].format().width(41);

    os << table << std::endl;

    return os;
}

// Output stream operator for DataFrame
std::ostream &operator<<(std::ostream &os, const DataFrame &df)
{
    tabulate::Table table;

    // Add header row
    table.add_row({"Host",
                   "GPU",
                   "Elapsed",
                   "SM Utilization %\n(avg/min/max)",
                   "Memory Utilization %\n(avg/min/max)"});

    size_t num_rows = df.gpuId.size();
    for (size_t i = 0; i < num_rows; ++i)
    {
        table.add_row(tabulate::Table::Row_t{
            df.host[i],
            std::to_string(df.gpuId[i]),
            format_elapsed((df.endTime[i] - df.startTime[i]) / 1000000),
            format_percent(df.smUtilizationAvg[i], df.smUtilizationMin[i], df.smUtilizationMax[i]),
            format_percent(df.memoryUtilizationAvg[i], df.memoryUtilizationMin[i], df.memoryUtilizationMax[i])});
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
    table[0][0].format().width(15); // Host
    table[0][1].format().width(6);  // GPU ID
    table[0][2].format().width(18); // Elapsed time
    table[0][3].format().width(18); // Utilization
    table[0][4].format().width(22); // Memory utilization

    // Print the table
    os << table << std::endl;

    return os;
}

DataFrame load_dataframe(const std::filesystem::path &target)
{
    DataFrame df;

    // Check if the target exists
    if (!std::filesystem::exists(target))
    {
        raise_error("File not found: \"" + target.string() + "\"");
    }

    // Target is not a directory
    if (!std::filesystem::is_directory(target))
    {
        // raise_error("Input must be a directory: \"" + input + "\"");
        std::cout << "WARNING: Input is not a directory. Skipping: \"" << target << "\"" << std::endl;
        return df;
    }

    // Target is a directory
    // Iterate over all files in the directory
    bool found_valid_file = false;
    for (const auto &entry : std::filesystem::directory_iterator(target))
    {
        // Check if the entry is a regular file
        if (entry.is_regular_file())
        {
            // Read file into DataFrame
            std::ifstream ifs(entry.path());

            // Check if file was opened successfully
            if (!ifs.is_open())
            {
                std::cerr << "WARNING: Could not open file. Skipping: " << entry.path() << std::endl;
                continue;
            }

            // Load the data from the file into the DataFrame
            // this operation will append the data to the existing DataFrame
            try
            {
                df.load(ifs);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Warning: error reading file. Is the file corrupted?" << std::endl
                          << "Skipping file: " + entry.path().string() << std::endl;
                continue;
            }

            ifs.close();

            found_valid_file = true;
        }
    }

    if (!found_valid_file)
    {
        raise_error("No valid CSV files found in directory: \"" + target.string() + "\"");
    }

    // Sort DataFrame by GPU ID
    df.sort_by_gpu_id();

    return df;
}

void print_job_stats(const std::filesystem::path &input, const std::string &output)
{
    // Load the DataFrame from the input directory
    DataFrame df = load_dataframe(input);

    // Compute averages
    DataFrameAvg avg = df.average();

    // Print summary
    if(output.empty())
    {
        std::cout << "Summary of Job Statistics" << std::endl
              << avg << std::endl
              << "GPU Specific Values" << std::endl
              << df << std::endl;    
    } else {
        // Check if the output file already exists
        if (std::filesystem::exists(output))
        {
            raise_error("Error: Output file already exists: \"" + output + "\"");
        }

        std::ofstream ofs(output);
        if (!ofs.is_open())
        {
            raise_error("Error: Unable to open output file: \"" + output + "\"");
        }
        ofs << "Summary of Job Statistics" << std::endl
            << avg << std::endl
            << "GPU Specific Values" << std::endl
            << df << std::endl;
        ofs.close();

        std::cout << "Report written to: \"" << output  << "\"" << std::endl;
    }
}

bool natural_order_comparator(const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b) {
    std::regex regex("step_(\\d+)");
    std::smatch match_a, match_b;

    std::string filename_a = a.path().filename().string();
    std::string filename_b = b.path().filename().string();

    bool is_a_match = std::regex_search(filename_a, match_a, regex);
    bool is_b_match = std::regex_search(filename_b, match_b, regex);

    if (is_a_match && is_b_match) {
        int num_a = std::stoi(match_a[1].str());
        int num_b = std::stoi(match_b[1].str());
        return num_a < num_b;
    }
    // If only one matches the schema, that one comes first
    if (is_a_match != is_b_match) {
        return is_a_match;
    }
    // If neither matches, sort lexicographically
    return filename_a < filename_b;
}

void process_stats(const std::string &input, const std::string &output)
{
    std::filesystem::path target(input);

    // Check if the target exists
    if (!std::filesystem::exists(target))
    {
        raise_error("File not found: \"" + input + "\"");
    }

    // Target is not a directory
    if (!std::filesystem::is_directory(target))
    {
        raise_error("Input must be a directory: \"" + input + "\"");
    }

    // Target is a directory
    // Check if target contains file
    if (std::filesystem::exists(target / ROOT_METADATA_FILE)) {
        // Collect all directory entries into a vector
        std::vector<std::filesystem::directory_entry> entries;
        for (const auto &entry : std::filesystem::directory_iterator(target)) {
            // Check if the entry is a directory
            if (entry.is_directory()) {
                entries.push_back(entry);
            }
        }

        // Sort the entries by natural numerical order
        std::sort(entries.begin(), entries.end(), natural_order_comparator);

        // Iterate over the sorted entries
        for (const auto &entry : entries) {
            print_job_stats(entry.path(), output);
        }
    } else { // The folder is a step folder already
        print_job_stats(target, output);
    }
}

#endif