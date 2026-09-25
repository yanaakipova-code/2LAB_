#pragma once

template<typename K, typename V>
class idictionary{
public: 
    virtual ~idictionary() = default;

    virtual std::size_t get_count()const = 0;

    virtual void add(const K& key, const V& value) = 0;
    virtual V get_elem_by_key(const K& key) const = 0;

    virtual bool contains_key(const K& key) const = 0;
    virtual void remove_by_key(const K& key) = 0;

};