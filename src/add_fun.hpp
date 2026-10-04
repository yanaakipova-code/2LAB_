#pragma once

template<typename T>
T my_max(const T& value_1, const T& value_2){
    if(value_1 > value_2){
        return value_1;
    }
    return value_2;
}

template<typename T>
T my_seq(const T& value){
    return value*value;
}