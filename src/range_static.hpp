#pragma once
#include "../structures/ArraySequence.hpp"
#include "add_fun.hpp"
#include "bubble_sort.hpp"

template<typename T>
struct range_static{
    std::size_t m_count = 0;
    double m_summ{};
    double m_seq_summ{};
    T m_max_elem{};
    T m_min_elem{};

    ArraySequence<T> m_values;

    void add(const T& elem){
        if(m_count == 0){
            m_max_elem = elem;
            m_min_elem = elem;
        }else{
            if (elem > m_max_elem) {
                m_max_elem = elem;
            }
            if( elem < m_min_elem){
                m_min_elem = elem;
            }
        }
        m_count++;
        m_summ += elem;
        m_seq_summ += my_seq(elem);
        m_values.Append(elem);
    }

    double mean() const{
        if (m_count != 0){
            return static_cast<double>(m_summ) / m_count;
        }
        return 0.0;
    }

    double variance() const{
        if(m_count == 0){
            return 0.0;
        }
        double m = mean();
        return (m_seq_summ / m_count) - my_seq(m);
    }

    double median() const {
        if (m_count == 0) {
            return 0.0;
        }

        ArraySequence<T> sorted = m_values;
        bubble_sort<T>::sort(sorted);  

        std::size_t n = sorted.GetLength();
        if (n % 2 == 1) {
            return double(sorted.Get(n / 2));
        } else {
            return (double(sorted.Get(n / 2 - 1)) +(sorted.Get(n / 2))) / 2.0;
        }
    }


};