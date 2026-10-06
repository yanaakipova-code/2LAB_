#pragma once
#include <cstddef>
#include "../structures/ArraySequence.hpp"

template<typename T>
class bubble_sort {
public:
    static void sort(ArraySequence<T>& seq) {
        std::size_t n = seq.GetLength();
        if (n <= 1) return;

        for (std::size_t i = 0; i < n - 1; ++i) {
            bool swapped = false;

            for (std::size_t j = 0; j < n - i - 1; ++j) {
                if (seq.Get(j) > seq.Get(j + 1)) {
                    swap(seq, j, j + 1);
                    swapped = true;
                }
            }

            if (!swapped) break;
        }
    }

private:
    static void swap(ArraySequence<T>& seq, std::size_t i, std::size_t j){
        T temp = seq.Get(i);
        seq.Set(i, seq.Get(j));
        seq.Set(j, temp);
    }
};