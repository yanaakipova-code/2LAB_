#pragma once
#include <cmath>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include "AVL_tree.hpp"
#include "range_static.hpp"
#include "../structures/ArraySequence.hpp"
#include "section.hpp"


template<typename T>
class histogram{
private:
    avl_tree<std::size_t, range_statistic<T>> m_bins;
    selection_histogram<T> m_sections;

public:
     histogram(const selection_histogram<T>& sections): m_sections(sections) {}

    histogram(T min, T max, T step): histogram(selection_histogram<T>::uniform(min, max, step)) {}
    histogram(const ArraySequence<T>& boundaries): histogram(selection_histogram<T>::uneven(boundaries)) {}

    bool add(const T& value){
        const std::size_t index = m_sections.find_bin(value);
        if(index == selection_histogram<T>::no_position){
            return false;
        }

        if(m_bins.contains_key(index)){
            m_bins.get_ref_by_key(index).add(value);
        }else{
            range_statistic<T> stats;
            stats.add(value);
            m_bins.add(index, stats);
        }
        return true;
    }

    range_statistic<T> get_bin(std::size_t index) const{
        if(index >= m_sections.get_bin_count()){
            throw no_find("Номер интервала вне диапазона");
        }
        if(!(has_data(index))){
            return range_statistic<T>();
        }
        return m_bins.get_elem_by_key(index);
    }

    bool has_data(std::size_t index) const{
        return m_bins.contains_key(index);
    }

    const selection_histogram<T>& get_selection() const{
        return m_sections;
    }
    
    
    std::size_t get_bin_count() const{
        return m_sections.get_bin_count();
    }
    T get_min() const {
        return m_sections.get_min();
    }

    T get_max() const{
        return m_sections.get_max();
    }
};
