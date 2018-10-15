#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>


#include "gamedata.h"
#include "runtime_error.h"

namespace Opcode {
    enum Opcode {
        Return       = 0,
        Push0        = 1,
        Push1        = 2,
        PushNeg1     = 3,
        Push8        = 4,
        Push16       = 5,
        Push32       = 6,
        Say          = 10,
    };
};

class Runner {
public:
    bool load(const std::string &filename) {
        data.load(filename);
        return data.gameLoaded;
    }

    void callMain();
    Value callFunction(int ident);
private:
    GameData data;
};

void Runner::callMain() {
    Value v = callFunction(data.mainFunction);
    std::cout << "\nMAIN RETURNED: " << v.type << ' ' << v.value << '\n';
}

Value Runner::callFunction(int ident) {
    const FunctionDef &function = data.getFunction(ident);
    const ByteStream &code = data.bytecode;

    std::vector<Value> stack;
    unsigned ip = function.position;
    int opcode, intValue;
    Value::Type type;
    Value value;
    while (1) {
        opcode = code.read_8(ip++);
        switch(opcode) {
            case Opcode::Return:
                if (stack.empty()) {
                    return Value{Value::Integer, 0};
                } else {
                    return stack.back();
                }
                break;
            case Opcode::Push0:
                type = static_cast<Value::Type>(code.read_8(ip++));
                stack.push_back(Value{type, 0});
                break;
            case Opcode::Push1:
                type = static_cast<Value::Type>(code.read_8(ip++));
                stack.push_back(Value{type, 1});
                break;
            case Opcode::PushNeg1:
                type = static_cast<Value::Type>(code.read_8(ip++));
                stack.push_back(Value{type, -1});
                break;
            case Opcode::Push8:
                type = static_cast<Value::Type>(code.read_8(ip++));
                intValue = code.read_8(ip++);
                stack.push_back(Value{type, intValue});
                break;
            case Opcode::Push16:
                type = static_cast<Value::Type>(code.read_8(ip++));
                intValue = code.read_16(ip);
                ip += 2;
                stack.push_back(Value{type, intValue});
                break;
            case Opcode::Push32:
                type = static_cast<Value::Type>(code.read_8(ip++));
                intValue = code.read_32(ip);
                ip += 4;
                stack.push_back(Value{type, intValue});
                break;
            case Opcode::Say:
                if (stack.empty()) throw RuntimeError("Stack underflow.");
                value = stack.back(); stack.pop_back();
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
                        std::cout << '<' << value.type << ' ' << value.value << '>';
                }
                break;
            default: {
                std::stringstream ss;
                ss << "Unknown opcode " << opcode << " at code position " << ip << '.';
                throw RuntimeError(ss.str());
            }
        }
    }

    return Value{};
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