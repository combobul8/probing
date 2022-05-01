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
    using value_type = T;

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

    template<typename Key, typename... Args>
    inline std::pair<iterator, bool> emplace(Key && key, Args &&... args)
    {
        size_t index = hash_object(key);
        size_t num_slots_minus_one = this->num_slots_minus_one;
        Table* entries = this->entries;
        index = hash_policy.index_for_hash(index, num_slots_minus_one);
        bool first = true;
        for (;;)
        {
            size_t block_index = index / BlockSize;
            int index_in_block = index % BlockSize;
            BlockPointer block = entries + block_index;
            int8_t metadata = block->control_bytes[index_in_block];
            if (first)
            {
                if ((metadata & Constants::bits_for_direct_hit) != Constants::magic_for_direct_hit)
                    return emplace_direct_hit({ index, block }, std::forward<Key>(key), std::forward<Args>(args)...);
                first = false;
            }
            if (compares_equal(key, block->data[index_in_block]))
                return { { block, index }, false };
            int8_t to_next_index = metadata & Constants::bits_for_distance;
            if (to_next_index == 0)
                return emplace_new_key({ index, block }, std::forward<Key>(key), std::forward<Args>(args)...);
            index += Constants::jump_distances[to_next_index];
            index = hash_policy.keep_in_range(index, num_slots_minus_one);
        }
    }
};
