#ifndef JOBREPORT_DATAFRAME_HPP
#define JOBREPORT_DATAFRAME_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>

#include "array.hpp"
#include "utils.hpp"

class DataFrame
{
public:
    DataFrame() = default;

    Array &operator[](const std::string &key);
    const Array &operator[](const std::string &key) const;
    DataFrame DataFrame::operator[](const std::vector<std::string> &selection) const;

    std::vector<std::string> cols() const;

    DataFrame mean(const std::vector<std::string> &selection = {}) const;
    DataFrame std(const std::vector<std::string> &selection = {}) const;
    DataFrame min(const std::vector<std::string> &selection = {}) const;
    DataFrame max(const std::vector<std::string> &selection = {}) const;

    friend std::ofstream &operator<<(std::ofstream &os, const DataFrame &df);
    friend std::ifstream &operator>>(std::ifstream &is, DataFrame &df);

    friend std::ostream &operator<<(std::ostream &os, const DataFrame &df);

private:
    std::unordered_map<std::string, Array> data;
};

// Operator definitions
Array &DataFrame::operator[](const std::string &key)
{
    return data[key];
}

const Array &DataFrame::operator[](const std::string &key) const
{
    auto it = data.find(key);
    if (it != data.end())
    {
        return it->second;
    }
    raise_error("Key not found in DataFrame");
}

DataFrame DataFrame::operator[](const std::vector<std::string> &selection) const
{
    if (selection.empty())
    {
        raise_error("Empty selection");
    }
    
    DataFrame result;
    for (const auto &col : selection)
    {
        result.data[col] = data.at(col);
    }
    return result;
}

std::vector<std::string> DataFrame::cols() const
{
    std::vector<std::string> columns;
    for (const auto &pair : data)
    {
        columns.push_back(pair.first);
    }
    return columns;
}

// Method definitions
DataFrame DataFrame::mean(const std::vector<std::string> &selection) const
{
    DataFrame result;
    const std::vector<std::string> columns = selection.empty() ? cols() : selection;
    for (const auto &col : columns)
    {
        Array meanArray(1);
        meanArray.append(data.at(col).mean());
        result.data[col] = meanArray;
    }
    return result;
}

DataFrame DataFrame::std(const std::vector<std::string> &selection) const
{
    DataFrame result;
    const std::vector<std::string> columns = selection.empty() ? cols() : selection;
    for (const auto &col : columns)
    {
        Array stdArray(1);
        stdArray.append(data.at(col).std());
        result.data[col] = stdArray;
    }
    return result;
}

DataFrame DataFrame::min(const std::vector<std::string> &selection) const
{
    DataFrame result;
    const std::vector<std::string> columns = selection.empty() ? cols() : selection;
    for (const auto &col : columns)
    {
        Array minArray(1);
        minArray.append(data.at(col).min());
        result.data[col] = minArray;
    }
    return result;
}

DataFrame DataFrame::max(const std::vector<std::string> &selection) const
{
    DataFrame result;
    const std::vector<std::string> columns = selection.empty() ? cols() : selection;
    for (const auto &col : columns)
    {
        Array maxArray(1);
        maxArray.append(data.at(col).max());
        result.data[col] = maxArray;
    }
    return result;
}

// Binary serialization operator for DataFrame
std::ofstream &operator<<(std::ofstream &os, const DataFrame &df)
{
    // Write number of columns
    size_t numCols = df.data.size();
    os.write(reinterpret_cast<const char *>(&numCols), sizeof(numCols));

    // Write each column name and corresponding Array
    for (const auto &pair : df.data)
    {
        size_t keySize = pair.first.size();
        os.write(reinterpret_cast<const char *>(&keySize), sizeof(keySize));
        os.write(pair.first.c_str(), keySize);
        os << pair.second; // Using Array's operator<< for std::ofstream
    }
    return os;
}

// Binary deserialization operator for DataFrame
std::ifstream &operator>>(std::ifstream &is, DataFrame &df)
{
    // Read number of columns
    size_t numCols;
    is.read(reinterpret_cast<char *>(&numCols), sizeof(numCols));

    // Read each column name and corresponding Array
    for (size_t i = 0; i < numCols; ++i)
    {
        size_t keySize;
        is.read(reinterpret_cast<char *>(&keySize), sizeof(keySize));

        std::string key(keySize, '\0');
        is.read(&key[0], keySize);

        Array arr;
        is >> arr; // Using Array's operator>> for std::ifstream

        df.data[key] = arr;
    }
    return is;
}

// Text output operator for DataFrame
std::ostream &operator<<(std::ostream &os, const DataFrame &df)
{
    for (const auto &pair : df.data)
    {
        os << pair.first << ": " << pair.second << std::endl; // Using Array's operator<< for std::ostream
    }
    return os;
}

#endif // JOBREPORT_DATAFRAME_HPP
