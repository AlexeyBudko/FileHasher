#pragma once

#include <exception>
#include <string>


class MyError : public std::exception
{
public:
    MyError(std::string message) :
        m_message(std::move(message))
    {
    }

    char const* what() const override
    {
        return m_message.c_str();
    }

private:
    std::string const m_message;
};
