#include <fstream>
#include <iostream>
#include <sstream>

#include "gamedata.h"
#include "runtime_error.h"

static uint32_t read_32(std::istream &in);
static uint16_t read_16(std::istream &in);
static uint8_t read_8(std::istream &in);
static std::string read_str(std::istream &in);


void GameData::load(const std::string filename) {
    std::ifstream inf(filename);
    if (!inf) {
        std::cerr << "Could not open ~" << filename << "~.\n";
        return;
    }
    if(read_32(inf) != FILETYPE_ID) {
        std::cerr << '~' << filename << "~ is not a valid gamefile.\n";
        return;
    }
    int version = read_32(inf);
    if(version != 0) {
        std::cerr << '~' << filename << "~ has format version " << version;
        std::cerr << ", but only version 0 is supported.\n";
        return;
    }
    mainFunction = read_32(inf);
    unsigned count = 0;

    // READ STRINGS
    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        StringDef def;
        def.ident = i;
        def.text = read_str(inf);
        strings.insert(std::make_pair(def.ident, def));
    }

    // READ LISTS
    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        ListDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.items.push_back(value);
        }
        lists.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        MapDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            Value v1, v2;
            v1.type = static_cast<Value::Type>(read_8(inf));
            v1.value = read_32(inf);
            v2.type = static_cast<Value::Type>(read_8(inf));
            v2.value = read_32(inf);
            def.rows.push_back(MapDef::Row{v1,v2});
        }
        maps.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        ObjectDef def;
        def.ident = read_32(inf);
        unsigned itemCount = read_16(inf);
        for (unsigned j = 0; j < itemCount; ++j) {
            unsigned propId = read_16(inf);
            Value value;
            value.type = static_cast<Value::Type>(read_8(inf));
            value.value = read_32(inf);
            def.properties.insert(std::make_pair(propId, value));
        }
        objects.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        FunctionDef def;
        def.ident = read_32(inf);
        def.arg_count = read_16(inf);
        def.local_count = read_16(inf);
        def.position = read_32(inf);
        functions.insert(std::make_pair(def.ident, def));
    }

    count = read_32(inf);
    for (unsigned i = 0; i < count; ++i) {
        bytecode.add_8(read_8(inf));
    }
    gameLoaded = true;
}

void GameData::dump() const {
    std::cout << "\n## Strings\n";
    for (const auto &stringDef : strings) {
        std::cout << '[' << stringDef.second.ident << "] ~";
        std::cout << stringDef.second.text << "~\n";
    }

    std::cout << "\n## Lists\n";
    for (const auto &listDef : lists) {
        std::cout << '[' << listDef.second.ident << "] {";
        for (const Value &value : listDef.second.items) {
            std::cout << ' ' << value;
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Maps\n";
    for (const auto &mapDef : maps) {
        std::cout << '[' << mapDef.second.ident << "] {";
        for (const MapDef::Row &row : mapDef.second.rows) {
            std::cout << " (" << row.key << ", " << row.value << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Objects\n";
    for (const auto &objectDef : objects) {
        std::cout << '[' << objectDef.second.ident << "] {";
        for (const auto &property : objectDef.second.properties) {
            std::cout << " (" << property.first << ", " << property.second << ")";
        }
        std::cout << " }\n";
    }

    std::cout << "\n## Function Headers\n";
    for (const auto &functionDef : functions) {
        std::cout << '[' << functionDef.second.ident << "] args: ";
        std::cout << functionDef.second.arg_count << " locals: ";
        std::cout << functionDef.second.local_count << " position: ";
        std::cout << functionDef.second.position << "\n";
    }

    std::cout << "\n## Bytecode";
    bytecode.dump(std::cout, 0);
}

const FunctionDef& GameData::getFunction(int ident) {
    auto functionIter = functions.find(ident);
    if (functionIter == functions.end()) {
        std::stringstream ss;
        ss << "Tried to access non-existant function " << ident << '.';
        throw RuntimeError(ss.str());
    }
    return functionIter->second;
}

const StringDef& GameData::getString(int ident) {
    auto stringIter = strings.find(ident);
    if (stringIter == strings.end()) {
        std::stringstream ss;
        ss << "Tried to run non-existant string " << ident << '.';
        throw RuntimeError(ss.str());
    }
    return stringIter->second;
}




uint32_t read_32(std::istream &in) {
    uint32_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

uint16_t read_16(std::istream &in) {
    uint16_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

uint8_t read_8(std::istream &in) {
    uint8_t value;
    in.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

std::string read_str(std::istream &in) {
    int length = read_16(in);
    std::string text(length, ' ');
    for (int i = 0; i < length; ++i) {
        text[i] = read_8(in);
    }
    return text;
}
