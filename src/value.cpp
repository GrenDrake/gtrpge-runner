/* **************************************************************************
 * General Class Definitions
 *
 * This file contains definitions and close utilities for several small utility
 * classes used by the compiler.
 *
 * Part of GTRPE by Gren Drake
 * **************************************************************************/
#include <fstream>
#include <string>

#include "value.h"

std::ostream& operator<<(std::ostream &out, const Value::Type &type) {
    switch(type) {
        case Value::None:
            out << "None";
            break;
        case Value::Integer:
            out << "Integer";
            break;
        case Value::String:
            out << "String";
            break;
        case Value::Symbol:
            out << "Symbol";
            break;
        case Value::Object:
            out << "Object";
            break;
        case Value::List:
            out << "List";
            break;
        case Value::Map:
            out << "Map";
            break;
        case Value::Node:
            out << "Node";
            break;
        case Value::Property:
            out << "Property";
            break;
        default:
            out << "(unhandled type)";
    }
    return out;
}

std::ostream& operator<<(std::ostream &out, const Value &value) {
    out << '<' << value.type;
    if (value.type == Value::Symbol) {
        out << " ~" << value.text << '~';
    } else if (value.type == Value::None) {
        // nothing
    } else {
        out << ' ' << value.value;
    }
    out << '>';
    return out;
}