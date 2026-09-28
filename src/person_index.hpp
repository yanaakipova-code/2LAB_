#pragma once
#include "AVL_tree.hpp"
#include "../structures/ArraySequence.hpp"
#include "person.hpp"
#include <string>
#include <cstddef>


class person_index{
private:
    avl_tree<std::string, ArraySequence<const person*>> by_first_name;
    avl_tree<std::string, ArraySequence<const person*>> by_last_name;
    avl_tree<std::string, ArraySequence<const person*>> by_FIO;
    avl_tree<std::size_t, ArraySequence<const person*>> by_age;

    template<typename K>
    void add_to_index(avl_tree<K, ArraySequence<const person*>>& index, const K& key, const person* p){
        if(index.contains_key(key)) {
            index.get_ref_by_key(key).Append(p);
        } else{
            ArraySequence<const person*> seq;
            seq.Append(p);
            index.add(key, seq);
        }
    }
    
    template<typename K>
    ArraySequence<const person*> find_index(const avl_tree<K, ArraySequence<const person*>>& index,
        const K& key) const{
            if(!index.contains_key(key)){
                return ArraySequence<const person*>();
            }
            return index.get_elem_by_key(key);
        }

public:
    person_index() = default;
    ~person_index() = default;

    void build(const ArraySequence<person>& persons){
        for (auto& p : persons){
            add_to_index(by_first_name, p.get_first_name(), &p);
            add_to_index(by_last_name,p.get_last_name(), &p);
            add_to_index(by_FIO, p.get_FIO(), &p);
            add_to_index(by_age, p.get_age(), &p);
        }
    }

    ArraySequence<const person*> find_by_first_name(const std::string& name){
        return find_index(by_first_name, name);
    }
    ArraySequence<const person*> find_by_last_name(const std::string& name){
        return find_index(by_last_name, name);
    }
    ArraySequence<const person*> find_by_FIO(const std::string& name){
        return find_index(by_FIO, name);
    }
    ArraySequence<const person*> find_by_age(const std::size_t& age){
        return find_index(by_age, age);
    }

};
