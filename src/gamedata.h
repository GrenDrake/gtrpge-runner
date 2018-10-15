#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <map>
#include "bytestream.h"
#include "value.h"

const int FILETYPE_ID = 0x47505254;

struct StringDef {
    int ident;
    std::string text;
};
struct ListDef {
    int ident;
    std::vector<Value> items;
};
struct MapDef {
    struct Row {
        Value key, value;
    };
    int ident;
    std::vector<Row> rows;
};
struct ObjectDef {
    int ident;
    std::map<unsigned, Value> properties;
};
struct FunctionDef {
    int ident;
    int arg_count;
    int local_count;
    unsigned position;
};

struct GameData {
    GameData() : gameLoaded(false) { }
    void load(const std::string filename);
    void dump() const;

    const FunctionDef& getFunction(int ident) const;
    const StringDef& getString(int ident) const;

    bool gameLoaded;
    int mainFunction;
    std::map<int, StringDef> strings;
    std::map<int, ListDef> lists;
    std::map<int, MapDef> maps;
    std::map<int, ObjectDef> objects;
    std::map<int, FunctionDef> functions;
    ByteStream bytecode;
};

#endif
