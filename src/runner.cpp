#include <array>
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
        GetProp      = 20,
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

    void say(const Value &value) const;
private:
    GameData data;
};

void Runner::callMain() {
    Value v = callFunction(data.mainFunction);
    std::cout << "\nMAIN RETURNED: ";
    say(v);
    std::cout << '\n';
}

Value Runner::callFunction(int ident) {
    const FunctionDef &function = data.getFunction(ident);
    const ByteStream &code = data.bytecode;

    std::vector<Value> locals(function.arg_count + function.local_count);
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
                if (value.type == Value::LocalVar) {
                    if (value.value < 0 || value.value >= static_cast<int>(locals.size())) {
                        std::stringstream ss;
                        ss << "Tried to access non-existant local ";
                        ss << value.value << '.';
                        throw RuntimeError(ss.str());
                    }
                    value = locals[value.value];
                }
                say(value);
                break;
            case Opcode::GetProp: {
                Value objectId = stack.back(); stack.pop_back();
                Value propId = stack.back(); stack.pop_back();
                if (objectId.type != Value::Object)
                    throw RuntimeError("Tried to get property of non-object.");
                if (propId.type != Value::Property)
                    throw RuntimeError("Tried to get non-property of.");
                const ObjectDef &object = data.getObject(objectId.value);
                auto propertyIter = object.properties.find(propId.value);
                if (propertyIter == object.properties.end()) {
                    stack.push_back(Value{Value::Integer, 0});
                } else {
                    stack.push_back(propertyIter->second);
                }
                break;
            }
            default: {
                std::stringstream ss;
                ss << "Unknown opcode " << opcode << " at code position " << ip << '.';
                throw RuntimeError(ss.str());
            }
        }
    }

    return Value{};
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