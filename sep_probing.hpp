//          Copyright Albert Chen 2022.
// Distributed under the Boost Software License, Version 1.0.
//    (See http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <utility>
#include <type_traits>
#include "flat_hash_map.hpp"
#include <vector>
#include <array>
#include <iostream>

const int8_t EMPTY = int8_t(0b11111111);

template<typename T, uint8_t TableSize>
struct Table
{
    Table()
    {
    }
    ~Table()
    {
    }
    int8_t control_bytes[TableSize];
    union
    {
        T data[TableSize];
    };

    static Table * empty_block()
    {
        static std::array<int8_t, TableSize> empty_bytes = []
        {
            std::array<int8_t, TableSize> result;
            result.fill(EMPTY);
            return result;
        }();
        return reinterpret_cast<Table *>(&empty_bytes);
    }

    int first_empty_index() const
    {
        for (int i = 0; i < TableSize; ++i)
        {
            if (control_bytes[i] == EMPTY)
                return i;
        }
        return -1;
    }

    void fill_control_bytes(int8_t value)
    {
        std::fill(std::begin(control_bytes), std::end(control_bytes), value);
    }
};
