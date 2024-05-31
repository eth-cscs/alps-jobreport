#ifndef JOBREPORT_ARRAY_HPP
#define JOBREPORT_ARRAY_HPP

#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <fstream>

class Array
{
public:
    Array();
    Array(size_t size);
    double operator[](size_t index) const;
    void append(double value);

    size_t size() const;
    void setZero();

    double mean() const;
    double std() const;
    double min() const;
    double max() const;

    friend std::ofstream &operator<<(std::ofstream &os, const Array &arr);
    friend std::ifstream &operator>>(std::ifstream &is, Array &arr);

    friend std::ostream &operator<<(std::ostream &os, const Array &arr);

private:
    std::vector<double> data;
};

// Constructor definitions
Array::Array() = default;

Array::Array(size_t size) : data(size) {}

// Method definitions
void Array::append(double value)
{
    data.push_back(value);
}

double Array::operator[](size_t index) const
{
    return data[index];
}

size_t Array::size() const
{
    return data.size();
}

void Array::setZero()
{
    std::fill(data.begin(), data.end(), 0.0);
}

double Array::mean() const
{
    if (data.empty())
    {
        return 0.0;
    }
    double sum = std::accumulate(data.begin(), data.end(), 0.0);
    return sum / data.size();
}

double Array::std() const
{
    if (data.empty())
    {
        return 0.0;
    }
    double m = mean();
    double sum = std::accumulate(data.begin(), data.end(), 0.0, [m](double a, double b)
                                 { return a + (b - m) * (b - m); });
    return std::sqrt(sum / data.size());
}

double Array::min() const
{
    if (data.empty())
    {
        return 0.0;
    }
    return *std::min_element(data.begin(), data.end());
}

double Array::max() const
{
    if (data.empty())
    {
        return 0.0;
    }
    return *std::max_element(data.begin(), data.end());
}

// Binary serialization operator for Array
std::ofstream &operator<<(std::ofstream &os, const Array &arr)
{
    // Write the size of the array
    size_t size = arr.data.size();
    os.write(reinterpret_cast<const char *>(&size), sizeof(size));

    // Write the array elements
    os.write(reinterpret_cast<const char *>(arr.data.data()), size * sizeof(double));
    return os;
}

// Binary deserialization operator for Array
std::ifstream &operator>>(std::ifstream &is, Array &arr)
{
    // Read the size of the array
    size_t size;
    is.read(reinterpret_cast<char *>(&size), sizeof(size));

    // Resize the array and read the elements
    arr.data.resize(size);
    is.read(reinterpret_cast<char *>(arr.data.data()), size * sizeof(double));
    return is;
}

// Text output operator for Array
std::ostream &operator<<(std::ostream &os, const Array &arr)
{
    os << "[";
    for (size_t i = 0; i < arr.data.size(); ++i)
    {
        os << arr.data[i];
        if (i != arr.data.size() - 1)
        {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

#endif // JOBREPORT_ARRAY_HPP
