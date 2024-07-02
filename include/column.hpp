#ifndef JOBREPORT_COLUMN_HPP
#define JOBREPORT_COLUMN_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cmath>
#include "utils.hpp"

template <typename T>
class DFColumn : public std::vector<T>
{
public:
    using std::vector<T>::vector;

    void permute(const std::vector<size_t>& indices) {
        DFColumn<T> temp(this->size());
        for (size_t i = 0; i < indices.size(); ++i) {
            temp[i] = (*this)[indices[i]];
        }
        *this = std::move(temp);
    }

    void write(std::ofstream& os, size_t index) const {
        os.write(reinterpret_cast<const char*>(&(*this)[index]), sizeof(T));
    }

    void read(std::ifstream& is) {
        T elem;
        if (is.read(reinterpret_cast<char*>(&elem), sizeof(T))) {
            this->push_back(elem);
        } else {
            raise_error("Unable to read from stream. Is the file corrupted?");
        }
    }

    T average() const {
        double sum = 0;
        double count = 0;
        for (const auto& elem : *this) {
            if (!std::isnan(elem)){
                sum += elem;
                count += 1.0;
            }
        }
        return count > 0 ? static_cast<T>(sum / count) : std::numeric_limits<T>::quiet_NaN();
    }

    T sum() const {
        double sum = 0;
        unsigned int count = 0;
        for (const auto& elem : *this) {
            if (!std::isnan(elem)){
                sum += elem;
                count++;
            }
        }
        return count > 0 ? static_cast<T>(sum) : std::numeric_limits<T>::quiet_NaN();
    }

    T min() const {
        return *std::min_element(this->begin(), this->end());
    }

    T max() const {
        return *std::max_element(this->begin(), this->end());
    }
};

#endif // JOBREPORT_COLUMN_HPP
