#pragma once

class exception{
private:
    const char* m_message;
public:
    exception(const char* message):  m_message(message){}\
    virtual ~exception() = default;
    virtual const char* what() const {
        return m_message;
    }
};

class invalid_key: public exception{
public:
    invalid_key(const char* message) : exception(message) {}
};

class null_ptr: public exception{
public:
    null_ptr(const char* message) : exception(message) {}
};

class no_find: public exception{
public:
    no_find(const char* message) : exception(message) {}
};
