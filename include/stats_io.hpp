#ifndef JOBREPORT_IO_HPP
#define JOBREPORT_IO_HPP

#include <vector>
#include <unordered_map>
#include <string>

#include "dataframe.hpp"
#include "array.hpp"
#include "filelock.hpp"
#include "utils.hpp"

class StatsIO
{
public:
    StatsIO(bool single_file_output = false, const std::string &lock_file = "")
        : single_file_output(single_file_output), lock_file(lock_file), lock(lock_file)
    {
        if (single_file_output && lock_file.empty())
        {
            raise_error("Lock file must be provided when single_file_output is set to true");
        }
    };

    void write(const std::string &filename, const DataFrame &df);
    void read(const std::string &filename, DataFrame &df);

private:
    bool single_file_output;
    const std::string lock_file;
    FileLock lock;
};

void StatsIO::write(const std::string &filename, const DataFrame &df)
{
    if (single_file_output)
    {
        lock.lock();
    }

    std::ofstream out_file;
    out_file.open(filename, std::ios::binary);

    if (!out_file)
    {
        if (single_file_output)
        {
            lock.unlock();
        }
        raise_error("Failed to open file for writing: " + filename);
    }

    // All good, write the DataFrame to the file
    out_file << df;

    out_file.close();

    if (single_file_output)
    {
        lock.unlock();
    }
}

void StatsIO::read(const std::string &filename, DataFrame &df)
{
    std::ifstream in_file;
    in_file.open(filename, std::ios::binary);

    if (!in_file)
    {
        raise_error("Failed to open file for reading: " + filename);
    }

    // All good, read the DataFrame from the file
    in_file >> df;

    in_file.close();
}
#endif