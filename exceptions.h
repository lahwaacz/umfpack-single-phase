#pragma once

#include <exception>

class ExceptionMessage : public std::exception
{
private:
    const char* _message;
public:
    ExceptionMessage( const char* message ) : _message( message ) {};
    virtual const char* what() const noexcept { return _message; };
};

class BadIndex : public ExceptionMessage
{
public:
    BadIndex( const char* message ) : ExceptionMessage( message ) {};
};
