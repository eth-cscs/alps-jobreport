#ifndef JOBREPORT_DATAFRAME_IO_HPP
#define JOBREPORT_DATAFRAME_IO_HPP

#include <iostream>
#include <string>
#include <vector>

#include "third_party/tabulate/tabulate.hpp"
#include "dataframe.hpp"

// Output stream operator for DataFrameAvg
std::ostream &operator<<(std::ostream &os, const DataFrameAvg &df)
{
    tabulate::Table table;
    table.add_row(tabulate::Table::Row_t{"energyConsumed", std::to_string(df.energyConsumed)});
    table.add_row(tabulate::Table::Row_t{"powerUsage (avg/min/max)",
                                         std::to_string(df.powerUsageAvg) + " / " +
                                             std::to_string(df.powerUsageMin) + " / " +
                                             std::to_string(df.powerUsageMax)});
    table.add_row(tabulate::Table::Row_t{"pcieRxBandwidth (avg/min/max)",
                                         std::to_string(df.pcieRxBandwidthAvg) + " / " +
                                             std::to_string(df.pcieRxBandwidthMin) + " / " +
                                             std::to_string(df.pcieRxBandwidthMax)});
    table.add_row(tabulate::Table::Row_t{"pcieTxBandwidth (avg/min/max)",
                                         std::to_string(df.pcieTxBandwidthAvg) + " / " +
                                             std::to_string(df.pcieTxBandwidthMin) + " / " +
                                             std::to_string(df.pcieTxBandwidthMax)});
    table.add_row(tabulate::Table::Row_t{"startTime", std::to_string(df.startTime)});
    table.add_row(tabulate::Table::Row_t{"endTime", std::to_string(df.endTime)});
    table.add_row(tabulate::Table::Row_t{"ElapsedTime(s)", std::to_string((df.endTime - df.startTime)/1000)});
    table.add_row(tabulate::Table::Row_t{"smUtilization (avg/min/max)",
                                         std::to_string(df.smUtilizationAvg) + " / " +
                                             std::to_string(df.smUtilizationMin) + " / " +
                                             std::to_string(df.smUtilizationMax)});
    table.add_row(tabulate::Table::Row_t{"memoryUtilization (avg/min/max)",
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
                   "Elapsed(s)",
                   "Utilization(%)\n(avg/min/max)",
                   "Memory utilization(%)\n(avg/min/max)"});

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
            std::to_string((df.endTime[i] - df.startTime[i]) / 1000),
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
    table[0][0].format().width(9); // GPU ID
    table[0][1].format().width(23); // Elapsed time
    table[0][2].format().width(23); // Utilization
    table[0][3].format().width(23); // Memory utilization

    // Print the table
    os << table << std::endl;

    return os;
}

#endif