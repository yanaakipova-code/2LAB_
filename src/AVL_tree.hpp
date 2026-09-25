#pragma once
#include "idictionary.hpp"
#include <memory>
#include "add_fun.hpp"
#include "error.hpp"


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

template<typename K, typename V>
class avl_tree: public idictionary<K,V>{
private:
    std::shared_ptr<Node<K, V>> m_root;
    std::size_t m_count;


    int get_height(const std::shared_ptr<Node<K, V>> node) const{
        if (node == nullptr){
            return 0;
        }
        return node->m_height;
    }

    void update_height(Node<K, V>* node) {
        if (node == nullptr){
            return;
        }
        node->m_height = 1 + my_max(
            get_height(node->m_left),
            get_height(node->m_right));
    }

    int balance_factor(const std::shared_ptr<Node<K, V>> node) const{
        if (node == nullptr){
            return 0;
        }
        return get_height(node->m_right) 
                - get_height(node->m_left);
    }

    std::shared_ptr<Node<K,V>> right_rotation(std::shared_ptr<Node<K,V>> a) {
        std::shared_ptr<Node<K,V>> b = std::move(a->m_left);
        std::shared_ptr<Node<K,V>> B = std::move(b->m_right);

        b->m_right = std::move(a);
        b->m_right->m_left = std::move(B);

        update_height(b->m_right.get());
        update_height(b.get());

        return b;
    }

    std::shared_ptr<Node<K,V>> left_rotation(std::shared_ptr<Node<K,V>> a) {
        std::shared_ptr<Node<K,V>> b = std::move(a->m_right);
        std::shared_ptr<Node<K,V>> B = std::move(b->m_left);

        b->m_left = std::move(a);
        b->m_left->m_right = std::move(B);   // ← ИСПРАВЛЕНО

        update_height(b->m_left.get());       // ← ИСПРАВЛЕНО
        update_height(b.get());

        return b;
    }
    

    std::shared_ptr<Node<K, V>> balance(std::shared_ptr<Node<K, V>> node){
        if (node == nullptr){
            return nullptr;
        }
        update_height(node.get());

        int b_f = balance_factor(node);
        if (b_f > 1){
            if(balance_factor(node->m_right) < 0){
                node->m_right = right_rotation(std::move(node->m_right));
            }
            return left_rotation(std::move(node));
        }
        if(b_f < -1){ 
            if (balance_factor(node->m_left) > 0){
                node->m_left = left_rotation(std::move(node->m_left));
            }
            return right_rotation(std::move(node));
        }
        return node;
    }

    std::shared_ptr<Node<K,V>> insert(std::shared_ptr<Node<K,V>> node, const K& key, const V& value) {
        if( node == nullptr){
            m_count ++;
            return std::make_shared<Node<K,V>>(key, value);
        }

        if (key < node->m_key){
            node->m_left = insert(std::move(node->m_left), key, value);
        }

        else if(key > node->m_key){
            node->m_right = insert(std::move(node->m_right), key, value);
        }

        else{
            throw invalid_key("Элемент с этим ключем уже есть");
        }

        return balance(std::move(node));
    }

    std::shared_ptr<Node<K, V>> find_node(std::shared_ptr<Node<K, V>> node, const K& key) const{
        if (node == nullptr){
           return nullptr;
        }
        if(node->m_key == key){
            return node;
        }
        if (node->m_key > key){
            return find_node(node->m_left, key);
        }
        return find_node(node->m_right, key);
        
    }

    std::shared_ptr<Node<K, V>> find_min(std::shared_ptr<Node<K, V>> node) const{
        if(node == nullptr){
            throw null_ptr("Ключ не найден ");
        }

        for(;node->m_left != nullptr;){
            node = node->m_left;
        }
        return  node;
    }

    std::shared_ptr<Node<K, V>> remove_node(std::shared_ptr<Node<K, V>> node, const K& key){
        if( node == nullptr){
            throw null_ptr("Ключ не найден");
        }
        if (key < node->m_key){
            node->m_left = remove_node(std::move(node->m_left), key);
        }
        else if(key > node->m_key){
            node->m_right = remove_node(std::move(node->m_right), key);
        }

        else{
            if(node->m_left == nullptr){
                return std::move(node->m_right);
            }

            if(node->m_right  == nullptr){
                return std::move(node->m_left);
            }

            std::shared_ptr<Node<K, V>> min_node = find_min(node->m_right);

            node->m_key = min_node->m_key;
            node->m_value = min_node->m_value;

            node->m_right = remove_node(std::move(node->m_right), min_node->m_key);
        }
        return balance(std::move(node));
    }


public:
    avl_tree(): m_count{0}, m_root{nullptr} {}

    ~avl_tree() override = default;

    std::size_t get_count()const override{
        return m_count;
    }

    void add(const K& key, const V& value) override{
        m_root = insert(std::move(m_root), key, value);
    }

    V get_elem_by_key(const K& key) const override{
        std::shared_ptr<Node<K, V>> node = find_node(m_root, key);

        if (node == nullptr){
            throw null_ptr("Ключ не найден");
        }

        return node->m_value;
    }

    bool contains_key(const K& key)const override{
        if(find_node(m_root, key) != nullptr){
            return true;
        }
        return false;
    }

    void remove_by_key(const K& key) override{
        if(!contains_key(key)){
            throw invalid_key("Ключ не найден");
        }
        m_root = remove_node(std::move(m_root), key);

        --m_count;
    }

};