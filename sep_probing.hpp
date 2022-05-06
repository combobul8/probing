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

inline size_t next_power_of_two(size_t i)
{
    --i;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i |= i >> 32;
    ++i;
    return i;
}

inline int8_t log2(size_t value)
{
    static constexpr int8_t table[64] =
    {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    return table[((value - (value >> 1)) * 0x07EDD5E59A4E28C2) >> 58];
}

struct fibonacci_hash_policy
{
    size_t index_for_hash(size_t hash, size_t /*num_slots_minus_one*/) const
    {
        return (11400714819323198485ull * hash) >> shift;
    }
    size_t keep_in_range(size_t index, size_t num_slots_minus_one) const
    {
        return index & num_slots_minus_one;
    }

    int8_t next_size_over(size_t & size) const
    {
        size = std::max(size_t(2), next_power_of_two(size));
        return 64 - log2(size);
    }
    void commit(int8_t shift)
    {
        this->shift = shift;
    }
    void reset()
    {
        shift = 63;
    }

private:
    int8_t shift = 63;
};

constexpr int8_t EMPTY = int8_t(0b11111111);
constexpr int8_t BITSFORDIRECTHIT = int8_t(0b10000000);
constexpr int8_t MAGICFORDIRECTHIT = int8_t(0b00000000);
constexpr int8_t BITSFORDISTANCE = int8_t(0b01111111);
constexpr int num_jump_distances = 126;
// jump distances chosen like this:
// 1. pick the first 16 integers to promote staying in the same block
// 2. add the next 66 triangular numbers to get even jumps when
// the hash table is a power of two
// 3. add 44 more triangular numbers at a much steeper growth rate
// to get a sequence that allows large jumps so that a table
// with 10000 sequential numbers doesn't endlessly re-allocate
constexpr size_t jump_distances[num_jump_distances]
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,

	21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231,
	253, 276, 300, 325, 351, 378, 406, 435, 465, 496, 528, 561, 595, 630,
	666, 703, 741, 780, 820, 861, 903, 946, 990, 1035, 1081, 1128, 1176,
	1225, 1275, 1326, 1378, 1431, 1485, 1540, 1596, 1653, 1711, 1770, 1830,
	1891, 1953, 2016, 2080, 2145, 2211, 2278, 2346, 2415, 2485, 2556,

	3741, 8385, 18915, 42486, 95703, 215496, 485605, 1091503, 2456436,
	5529475, 12437578, 27986421, 62972253, 141700195, 318819126, 717314626,
	1614000520, 3631437253, 8170829695, 18384318876, 41364501751,
	93070021080, 209407709220, 471167588430, 1060127437995, 2385287281530,
	5366895564381, 12075513791265, 27169907873235, 61132301007778,
	137547673121001, 309482258302503, 696335090510256, 1566753939653640,
	3525196427195653, 7931691866727775, 17846306747368716,
	40154190394120111, 90346928493040500, 203280588949935750,
	457381324898247375, 1029107980662394500, 2315492957028380766,
	5209859150892887590,
};

template<typename K, typename V, uint8_t TableSize>
struct Table
{
    using value_type = K;

    Table()
    {
    }
    ~Table()
    {
    }
    int8_t control_bytes[TableSize];
    union
    {
        K data[TableSize];
    };
	fibonacci_hash_policy hash_policy;

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

    template<typename ValueType>
    struct templated_iterator
    {
    private:
        friend class sherwood_v8_table;
        Table* current = Table();
        size_t index = 0;

    public:
        templated_iterator()
        {
        }
        templated_iterator(Table* entries, size_t index)
            : current(entries)
            , index(index)
        {
        }

        using iterator_category = std::forward_iterator_tag;
        using value_type = ValueType;
        using difference_type = ptrdiff_t;
        using pointer = ValueType *;
        using reference = ValueType &;

        friend bool operator==(const templated_iterator & lhs, const templated_iterator & rhs)
        {
            return lhs.index == rhs.index;
        }
        friend bool operator!=(const templated_iterator & lhs, const templated_iterator & rhs)
        {
            return !(lhs == rhs);
        }

        templated_iterator & operator++()
        {
            do
            {
                if (index % TableSize == 0)
                    --current;
                if (index-- == 0)
                    break;
            }
            while(current->control_bytes[index % TableSize] == EMPTY);
            return *this;
        }
        templated_iterator operator++(int)
        {
            templated_iterator copy(*this);
            ++*this;
            return copy;
        }

        ValueType & operator*() const
        {
            return current->data[index % TableSize];
        }
        ValueType * operator->() const
        {
            return current->data + index % TableSize;
        }

        operator templated_iterator<const value_type>() const
        {
            return { current, index };
        }
    };

	using iterator = templated_iterator<value_type>;

    template<typename U>
    size_t hash_object(const U & key)
    {
        return static_cast<Hasher &>(*this)(key);
    }
    template<typename U>
    size_t hash_object(const U & key) const
    {
        return static_cast<const Hasher &>(*this)(key);
    }

    template<typename Key, typename... Args>
    inline std::pair<iterator, bool> emplace(Key && key, Args &&... args)
    {
        size_t hash = hash_object(key);
        size_t num_slots_minus_one = this->num_slots_minus_one;
        Table* entries = this->entries;
        hash = hash_policy.index_for_hash(hash, num_slots_minus_one);
        for (;;)
        {
            int index = hash % TableSize;
            Table* block = entries + index;

            if (compares_equal(key, block->data[index]))
                return { { block, index }, false };

	    std::cout << "TODO: probing" << std::endl;
	    break;
            index = hash_policy.keep_in_range(index + 1, num_slots_minus_one);
        }
    }
};
