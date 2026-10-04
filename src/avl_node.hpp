#pragma once
#include <memory>

template<typename K, typename V>
struct Node{
    K m_key;
    V m_value;
    std::shared_ptr<Node> m_left;
    std::shared_ptr<Node> m_right;
    int m_height;

    Node(const K& key, const V& value): m_key{key}, m_value{value},
        m_left{nullptr}, m_right{nullptr}, m_height{1} {}
};