#pragma once
#include <concepts>
#include <type_traits>

template<typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept ordered = requires(const T& a, const T& b) {
    { a == b } -> std::convertible_to<bool>;
    { a <  b } -> std::convertible_to<bool>;
    { a <= b } -> std::convertible_to<bool>;
    { a >  b } -> std::convertible_to<bool>;
};

template<typename T>
concept has_next = requires(const T& value) {
    { value.next() } -> std::same_as<T>;
};

template<typename T>
concept bin_key = ordered<T> && (numeric<T> || has_next<T>);

template<typename Stat, typename Item>
concept statistic_for = std::default_initializable<Stat> &&
    requires(Stat stat, const Item& item) { 
        stat.add(item); 
    };
