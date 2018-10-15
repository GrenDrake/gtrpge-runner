#ifndef VALUE_H
#define VALUE_H

#include <string>

struct Value {
    enum Type {
        None        = 0,
        Integer     = 1,
        String      = 2,
        List        = 3,
        Map         = 4,
        Node        = 5,
        Object      = 6,
        Property    = 7,
        LocalVar    = 8,
        Symbol      = 99
    };

    Type type;
    int value;
    std::string text;
};

std::ostream& operator<<(std::ostream &out, const Value::Type &type);
std::ostream& operator<<(std::ostream &out, const Value &value);

#endif
