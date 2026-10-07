#pragma once
#include <cmath>
#include <cstddef>
#include "AVL_tree.hpp"
#include "range_static.hpp"
#include "../structures/ArraySequence.hpp"

template<typename T>
class histogram{
private:
    avl_tree<T, range_static<T>> m_bins;
    T m_min;
    T m_max;
    T m_step;
    bool m_uniform;
    ArraySequence<T> m_boundaries;

    T get_bin_key(const T& value) const{
        if(m_uniform){
            double idx = std::floor(static_cast<double>(value - m_min) /
                                    static_cast<double>(m_step));
            return m_min + static_cast<T>(idx) * m_step;
        }else{
            for(std::size_t i = 0; i + 1 < m_boundaries.GetLength(); ++i) {
                T lower = m_boundaries.Get(i);
                T upper = m_boundaries.Get(i+1);
                if(lower <= value && upper > value){
                    return lower;
                }
            }
            return m_boundaries.Get(m_boundaries.GetLength() - 1);
        }
    }

public:
    histogram(T min, T max, T step): m_min{min}, m_max{max}, m_step{step}, m_uniform{true} {}
    histogram(const ArraySequence<T>& boundaries):
        m_min{boundaries.Get(0)},
        m_max{boundaries.Get(boundaries.GetLength() - 1)},
        m_step{0}, m_uniform{false}, m_boundaries{boundaries} {}

    void add(const T& value){
        T key = get_bin_key(value);
        if(m_bins.contains_key(key)){
            range_static<T>& stat = m_bins.get_ref_by_key(key);
            stat.add(value);
        } else {
            range_static<T> stats;
            stats.add(value);
            m_bins.add(key, stats);
        }
    }

    ArraySequence<T> get_bin_keys() const{
        ArraySequence<T> keys;

        if(m_uniform){
            std::size_t n = static_cast<std::size_t>(
                std::ceil(static_cast<double>(m_max - m_min) / static_cast<double>(m_step)));
            for(std::size_t i = 0; i < n; ++i){
                T key = m_min + static_cast<T>(i) * m_step;
                if(m_bins.contains_key(key)){
                    keys.Append(key);
                }
            }
        }else{
            for(std::size_t i = 0; i < m_boundaries.GetLength(); ++i){
                T key = m_boundaries.Get(i);
                if(m_bins.contains_key(key)){
                    keys.Append(key);
                }
            }
        }
        return keys;
    }

    range_static<T> get_bin(const T& key) const {
        return m_bins.get_elem_by_key(key);
    }

    std::size_t get_bin_count() const { return m_bins.get_count(); }
    T get_min() const { return m_min; }
    T get_max() const { return m_max; }
    T get_step() const { return m_step; }
    bool is_uniform() const { return m_uniform; }
};
