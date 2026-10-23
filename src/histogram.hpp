#pragma once
#include <cmath>
#include <concepts>
#include <type_traits>
#include <cstddef>
#include "AVL_tree.hpp"
#include "range_static.hpp"
#include "../structures/ArraySequence.hpp"
#include "section.hpp"
#include "concepts.hpp"


template<typename T, typename Stat = range_statistic<T>>
class histogram{
private:
    avl_tree<std::size_t, Stat> m_bins;
    selection_histogram<T> m_sections;

    histogram(const selection_histogram<T>& sections): m_sections(sections) {}

    static void check_finite(double value, const char* message){
        if(!std::isfinite(value)){
            throw invalid_range(message);
        }
    }

public:
    static constexpr std::size_t max_bin_count = 100000;

    template<typename Y>
    static histogram make_histogram(T min, T max, Y step){
        check_finite(static_cast<double>(min), "Минимум должен быть конечным числом");
        check_finite(static_cast<double>(max), "Максимум должен быть конечным числом");
        check_finite(static_cast<double>(step), "Шаг должен быть конечным числом");

        const T converted_step = static_cast<T>(step);
        if(!(converted_step > 0)){
            throw invalid_range("Шаг слишком мал для типа границ (после приведения он равен нулю)");
        }

        const double count = std::ceil((static_cast<double>(max) - static_cast<double>(min)) /
                                        static_cast<double>(converted_step));

        if(count > static_cast<double>(max_bin_count)){
            throw length_arr("Слишком много интервалов");
        }

        return histogram(selection_histogram<T>::uniform(min, max, converted_step));
    }
    
    static histogram make_histogram(const ArraySequence<T>& boundaries)
        requires ordered<T>
    {
        if(boundaries.GetLength() < 2){
            throw length_arr("Введите как минимум 2 границы");
        }
        if(boundaries.GetLength() - 1 > max_bin_count){
            throw length_arr("Слишком много интервалов");
        }
        if constexpr(std::floating_point<T>){
            for(std::size_t i = 0; i < boundaries.GetLength(); ++i){
                check_finite(static_cast<double>(boundaries.Get(i)),
                             "Границы должны быть конечными числами");
            }
        }
        return histogram(selection_histogram<T>::uneven(boundaries));
    }

    template<typename Item, typename KeyFunction>
        requires bin_key<T> && std::is_invocable_r_v<T, KeyFunction, const Item&>
    static histogram make_histogram(const ArraySequence<Item>& items, KeyFunction key_of){
        if constexpr(std::is_pointer_v<KeyFunction>){
            if(key_of == nullptr){
                throw null_ptr("Не задана функция, которая выдаёт ключ элемента");
            }
        }
        if(items.GetLength() == 0){
            throw length_arr("Нет данных, по которым можно построить гистограмму");
        }
        avl_tree<T, bool> seen;
        ArraySequence<T> boundaries;
        for(std::size_t i = 0; i < items.GetLength(); ++i){
            T key = key_of(items.GetRef(i));
            if(!seen.contains_key(key)){
                if(boundaries.GetLength() >= max_bin_count){
                    throw length_arr("Слишком много различных значений ключа");
                }
                seen.add(key, true);
                boundaries.Append(key);
            }
        }

        bubble_sort<T>::sort(boundaries);
        boundaries.Append(boundaries.GetLast().next());

        histogram result(selection_histogram<T>::uneven(boundaries));
        for(std::size_t i = 0; i < items.GetLength(); ++i){
            result.add(key_of(items.GetRef(i)));
        }
        return result;
    }

     bool add(const T& value){
        const std::size_t index = m_sections.find_bin(value);
        if(index == selection_histogram<T>::no_position){
            return false;
        }

        if(m_bins.contains_key(index)){
            m_bins.get_ref_by_key(index).add(value);
        }else{
            Stat stats;
            stats.add(value);
            m_bins.add(index, stats);
        }
        return true;
    }

    Stat get_bin(std::size_t index) const{
        if(index >= m_sections.get_bin_count()){
            throw no_find("Номер интервала вне диапазона");
        }
        if(!(has_data(index))){
            return Stat();
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
