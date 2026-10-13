#pragma once
#include "error.hpp"
#include "../structures/ArraySequence.hpp"
#include <cmath>
#include <cstddef>

template<typename T>
class selection_histogram{
private:
    ArraySequence<T> m_boundaries;

    selection_histogram(const ArraySequence<T>& boundaries): m_boundaries{boundaries}{
        if(m_boundaries.GetLength() < 2){
            throw length_arr("Введите как минумум 2 границы");
        }
        for(std::size_t i = 1; i < m_boundaries.GetLength(); i ++){
            if (!(m_boundaries.Get(i) > m_boundaries.Get(i - 1))){
                throw length_arr("Границы должны строго возрастать");
            }
        }
    }

    void check_index(std::size_t index) const{
        if(index >= get_bin_count()){
            throw std::out_of_range("Номер интервала вне диапазона");
        }
    }

public:
    static constexpr std::size_t no_position = static_cast<std::size_t>(-1);

    static selection_histogram uniform(T min, T max, T step){
        if(!(min < max)){
            throw invalid_range("Минимум не может быть больше максимума");
        }
        if (!( step > 0)){
            throw invalid_range("шаг не может быть отрицательным");
        }

        const std::size_t count = static_cast<std::size_t>(std::ceil((static_cast<double>(max) - static_cast<double>(min)) /static_cast<double>(step)));
        ArraySequence<T> bounds;
        for (std::size_t i = 0; i < count; i++){
            T current = min + static_cast<T>(i) * step;
            if (!(current < max)){
                break;
            }
            bounds.Append(current);
        }
        bounds.Append(max);
        return selection_histogram(bounds);
    }

    static selection_histogram uneven(const ArraySequence<T>& bouds){
        return selection_histogram(bouds);
    }

    std::size_t find_bin(const T& value) const{
        const std::size_t count = m_boundaries.GetLength();
        if(!(value >= m_boundaries.Get(0) && value <= m_boundaries.Get(count - 1) )){
            return no_position;
        }
        
        if(value == m_boundaries.Get(count - 1)){
            return count - 2;
        }

        std::size_t low = 0, high = count - 1;
        while((high - low) > 1){
            std::size_t mid = low + (high - low) / 2;
            if(m_boundaries.Get(mid) <= value){
                low = mid;
            }else{
                high = mid;
            }
        }
        return low;
    }

    T get_lower(std::size_t index) const{
        check_index(index);
        return m_boundaries.Get(index);
    }

    T get_upper(std::size_t index) const{
        check_index(index);
        return m_boundaries.Get(index + 1);
    }

    std::size_t get_bin_count() const{
        return (m_boundaries.GetLength() - 1);
    }

    T get_min() const{
        return (m_boundaries.GetFirst());
    }

    T get_max() const{
        return (m_boundaries.GetLast());
    }
    bool contains(const T& value) const{
        return find_bin(value) != no_position;
    }

};