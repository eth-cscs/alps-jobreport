#ifndef JOBREPORT_COLUMN_HPP
#define JOBREPORT_COLUMN_HPP

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
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
        return std::accumulate(this->begin(), this->end(), T(0)) / this->size();
    }
};

#endif // JOBREPORT_COLUMN_HPP
