#pragma once
#include <string> 
#include <ctime>
#include <stdexcept>
#include <format>

struct person_id{
    std::size_t series;
    std::size_t number;

};

class person{
private:
    person_id m_passport;
    std::string m_first_name;
    std::string m_last_name;
    std::string m_patronymic;
    std::time_t m_date;

public:
    person():m_first_name{""}, m_last_name{""}, 
            m_patronymic{""}, m_date{0},m_passport{} {}

    person(const person_id& id, const std::string& first_name, const std::string& last_name, 
            const std::string& patronymic, const std::time_t& date):m_passport{id},
            m_first_name{first_name}, m_last_name{last_name}, m_patronymic{patronymic},
            m_date{date} {}
    person(const person& other) = default;

    std::string get_first_name() const{
        return m_first_name;
    }
    std::string get_last_name() const{
        return m_last_name;
    }
    std::string get_patronymic() const{
        return m_patronymic;
    }
    std::time_t get_date() const{
        return m_date;
    } 
    person_id get_passport() const{
        return m_passport;
    }
    

    std::string get_FIO() const{
        return std::format("{} {} {}", m_last_name[0], m_first_name[0], m_patronymic[0]);
    }

    std::string get_full_name() const{
        return std::format("{} {} {}", m_last_name, m_first_name, m_patronymic);
    }

    std::size_t get_age() const {
        std::time_t now = std::time(nullptr);

        const std::tm* pb = std::localtime(&m_date);
        if(pb == nullptr){
            return 0;
        }
        std::tm b = *pb;

        const std::tm* pt = std::localtime(&now);
        if(pt == nullptr){
            return 0;
        }
        std::tm t = *pt;

        int age = t.tm_year - b.tm_year;
        if(t.tm_mon < b.tm_mon || (t.tm_mon == b.tm_mon && t.tm_mday < b.tm_mday)){
            --age;
        }
        if (age < 0) {
            return 0;
        } else {
            return static_cast<std::size_t>(age);
        }
    }
};