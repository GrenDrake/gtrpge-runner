#ifndef RUNNER_H
#define RUNNER_H

#include <string>
#include "gamedata.h"

struct Value;

class Runner {
public:
    bool load(const std::string &filename) {
        data.load(filename);
        return data.gameLoaded;
    }

    void callMain();
    Value callFunction(int ident, const std::vector<Value> &arguments = {});

    void say(unsigned intValue) const;
    void say(const Value &value) const;
private:
    GameData data;
};

#endif
