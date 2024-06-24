#ifndef JOBREPORT_IO_HPP
#define JOBREPORT_IO_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

#include "dataframe.hpp"
#include "filelock.hpp"
#include "utils.hpp"

class FileIO
{
public:
    FileIO(bool single_file_output = false, const std::string &lock_file = "") 
    : single_file_output(single_file_output), lock(lock_file){
        if (single_file_output && lock_file.empty())
        {
            raise_error("Lock file path cannot be empty when single_file_output is true.");
        }
    };

    void write(const std::string &filename, DataFrame &df);
    void read(const std::string &filename, DataFrame &df);

private:
    const std::string lock_file;
    bool single_file_output;
    FileLock lock;
};

void FileIO::write(const std::string &filename, DataFrame &df)
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
    df.dump(out_file);
    out_file.close();

    if (single_file_output)
    {
        lock.unlock();
    }
}

void FileIO::read(const std::string &filename, DataFrame &df)
{
    std::ifstream in_file;
    in_file.open(filename, std::ios::binary);

    if (!in_file)
    {
        raise_error("Failed to open file for reading: " + filename);
    }

    // All good, read the DataFrame from the file
    df.load(in_file);

    in_file.close();
}
#endif