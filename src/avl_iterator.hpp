#pragma once
#include "avl_node.hpp" 
#include "../structures/LinkedList.hpp"
#include "error.hpp"
#include <memory>

template<typename K, typename V>
class avl_iterator{
private:
    LinkedList<std::shared_ptr<Node<K,V>>> m_container;
    std::shared_ptr<Node<K,V>> m_current;

    void push_left(std::shared_ptr<Node<K,V>> node){
        while(node != nullptr){
            m_container.Append(node);
            node = node->m_left;
        }
    }

    void advance(){
        if(m_container.GetLength() == 0){
            m_current = nullptr;
            return;
        }
        m_current = m_container.GetLast();
        m_container.RemoveAt(m_container.GetLength()-1);

        if(m_current->m_right != nullptr){
            push_left(m_current->m_right);
        }
    }

public:
    avl_iterator(std::shared_ptr<Node<K,V>> node): m_current{nullptr}{
        push_left(node);
        advance();
    }

    V& operator*() {
        if (m_current == nullptr) {
            throw null_ptr("Вышли за предел");
        }
        return m_current->m_value;
    }

    const V& operator*() const{
        if(m_current == nullptr){
            throw null_ptr("Вышли за предел");
        }
        return m_current->m_value;
    }

    avl_iterator& operator++(){
        advance();
        return *this;
    }

    bool operator==(const avl_iterator& other) const{
        return m_current == other.m_current;
    }

    bool operator!=(const avl_iterator& other){
        return m_current != other.m_current;
    }

    bool has_next() const{
       return m_container.GetLength() > 0 || m_current != nullptr;
    }
};