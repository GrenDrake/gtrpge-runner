#ifndef RUNTIME_ERROR_H
#define RUNTIME_ERROR_H

#include <stdexcept>

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string &message)
    : std::runtime_error(message)
    { }
};

#endif
