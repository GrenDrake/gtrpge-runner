#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include "gamedata.h"
#include "runtime_error.h"
#include "runner.h"


void Runner::callMain() {
    Value v = callFunction(data.mainFunction);
    std::cout << "\nMAIN RETURNED: ";
    say(v);
    std::cout << '\n';
}

void Runner::say(unsigned intValue) const {
    std::cout << intValue;
}

void Runner::say(const Value &value) const {
    switch(value.type) {
        case Value::String: {
            const StringDef &stringDef = data.getString(value.value);
            std::cout << stringDef.text;
            break;
        }
        case Value::Integer:
            std::cout << value.value;
            break;
        default:
            std::cout << '<' << value.type;
            if (value.type != Value::None) {
                std::cout << ' ' << value.value;
            }
            std::cout << '>';
    }
}

int main(int argc, char *argv[]) {
    std::string gamefile = "game.bin";
    if (argc == 2) gamefile = argv[1];
    else if (argc > 2) {
        std::cerr << "USAGE: " << argv[0] << " [gamefile]\n";
        return 1;
    }

    Runner runner;
    if (!runner.load(gamefile)) {
        std::cerr << "Failed to load game data.\n";
        return 1;
    }

    try {
        runner.callMain();
    } catch (RuntimeError &e) {
        std::cerr << "RUNTIME ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}