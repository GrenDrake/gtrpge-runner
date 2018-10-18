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
        Store        = 7,
        Say          = 10,
        StackPop     = 13, // remove the top item from the stack
        StackDup     = 14, // duplicate the top item on the stack
        StackPeek    = 15, // peek at the stack item X items from the top
        StackSize    = 16, // get the current size of the stack
        Call         = 17, // call a value as a function
        CallMethod   = 18, // call an object property as a function
        Self         = 19, // get object the current function is a property of
        GetProp      = 20,
        HasProp      = 21, // check if property is set on object
        SetProp      = 22, // set object property to value
        GetItem      = 23, // get item from list (index) or map (key)
        HasItem      = 24, // check if index (for list) or key (for map) exists
        GetSize      = 25, // get size of list or map
        SetItem      = 26, // set item in list (by index) of map (by key)
        TypeOf       = 27, // get value type
        Jump         = 30,
        JumpEq       = 31,
        JumpNeq      = 32,
        JumpLt       = 33,
        JumpLte      = 34,
        JumpGt       = 35,
        JumpGte      = 36,
        JumpTrue     = 37, // jump if value is non-zero (true)
        JumpFalse    = 38, // jump if value is zero (false)
        Add          = 40,
        Sub          = 41,
        Mult         = 42,
        Div          = 43,
    };
};

class Runner {
public:
    bool load(const std::string &filename) {
        data.load(filename);
        return data.gameLoaded;
    }

    void callMain();
    Value callFunction(int ident, const std::vector<Value> &arguments = {});

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

Value readLocal(const Value &value, const std::vector<Value> locals) {
    if (value.type == Value::LocalVar) {
        if (value.value < 0 || value.value >= static_cast<int>(locals.size())) {
            std::stringstream ss;
            ss << "Tried to access non-existant local ";
            ss << value.value << '.';
            throw RuntimeError(ss.str());
        }
        return locals[value.value];
    }
    return value;
}

Value popStack(std::vector<Value> &stack) {
    if (stack.empty()) {
        throw RuntimeError("Stack underflow.");
    }
    Value v = stack.back();
    stack.pop_back();
    return v;
}

void dumpStack(const std::vector<Value> &stack) {
    if (stack.empty()) {
        std::cout << "(stack empty)\n";
        return;
    }
    std::cout << '\n';
    for (unsigned i = 0; i < stack.size(); ++i) {
        std::cout << i << ": " << stack[i] << '\n';
    }
}

Value Runner::callFunction(int ident, const std::vector<Value> &arguments) {
    const FunctionDef &function = data.getFunction(ident);
    const ByteStream &code = data.bytecode;

    if (arguments.size() > static_cast<unsigned>(function.arg_count)) {
        throw RuntimeError("Too many arguments to function.");
    }

    std::vector<Value> locals(function.arg_count + function.local_count);
    std::vector<Value> stack;

    for (unsigned i = 0; i < arguments.size() && i < locals.size(); ++i) {
        locals[i] = arguments[i];
    }

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
            case Opcode::Store: {
                Value localId = popStack(stack);
                Value value = popStack(stack);
                if (localId.type != Value::LocalVar) {
                    throw RuntimeError("Tried to store to non-local.");
                }
                if (localId.value < 0 || localId.value >= static_cast<int>(locals.size())) {
                    throw RuntimeError("Tried to store to non-existant local number.");
                }
                locals[localId.value] = value;
                break;
            }

            case Opcode::Say:
                value = popStack(stack);
                value = readLocal(value, locals);
                say(value);
                break;

            case Opcode::StackPop: {
                popStack(stack);
                break;
            }
            case Opcode::StackDup: {
                if (stack.empty()) throw RuntimeError("Stack underflow.");
                stack.push_back(stack.back());
                break;
            }
            case Opcode::StackPeek: {
                Value depth = popStack(stack);
                if (depth.type != Value::Integer) throw RuntimeError("stack-peek: depth must be integer.");
                std::cout << depth.value << '\n';
                if (depth.value >= stack.size()) throw RuntimeError("stack-peek: tried to peek beyond bottom of stack.");
                stack.push_back(stack[depth.value]);
                break;
            }
            case Opcode::StackSize: {
                int stackSize = stack.size();
                stack.push_back(Value{Value::Integer, stackSize});
                break;
            }

            case Opcode::Call: {
                Value functionId = popStack(stack);
                Value argCount = popStack(stack);
                if (argCount.type != Value::Integer) {
                    throw RuntimeError("Argument count must be integer");
                }
                std::vector<Value> args;
                for (int i = 0; i < argCount.value; ++i) {
                    args.push_back(popStack(stack));
                }
                Value result;
                switch(functionId.type) {
                    case Value::Node:
                        result = callFunction(functionId.value, args);
                        break;
                    default:
                        std::stringstream ss;
                        ss << "Value type " << functionId.type << " not callable.";
                        throw RuntimeError(ss.str());
                }
                stack.push_back(result);
                break;
            }

            case Opcode::GetProp: {
                Value objectId = popStack(stack);
                Value propId = popStack(stack);
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

            case Opcode::JumpEq: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value == v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }
            case Opcode::JumpNeq: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value <= v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }
            case Opcode::JumpLt: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value != v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }
            case Opcode::JumpLte: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value <= v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }
            case Opcode::JumpGt: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value > v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }
            case Opcode::JumpGte: {
                Value target = popStack(stack);
                Value v1 = popStack(stack);
                Value v2 = popStack(stack);
                v1 = readLocal(v1, locals); v2 = readLocal(v2, locals);
                if (target.type != Value::JumpTarget) {
                    throw RuntimeError("Tried to jump to non-jump target");
                }
                if (v1.type == v2.type && v2.value >= v1.value) {
                    ip = function.position + target.value;
                }
                break;
            }

            case Opcode::Add: {
                Value v1 = stack.back(); stack.pop_back();
                Value v2 = stack.back();
                v1 = readLocal(v1, locals);
                v2 = readLocal(v2, locals);
                if (v1.type != Value::Integer || v2.type != Value::Integer) {
                    throw RuntimeError("Can only add integer values.");
                }
                v2.value += v1.value;
                stack[stack.size() - 1] = v2;
                break;
            }
            case Opcode::Sub: {
                Value v1 = stack.back(); stack.pop_back();
                Value v2 = stack.back();
                v1 = readLocal(v1, locals);
                v2 = readLocal(v2, locals);
                if (v1.type != Value::Integer || v2.type != Value::Integer) {
                    throw RuntimeError("Can only subtract integer values.");
                }
                v2.value -= v1.value;
                stack[stack.size() - 1] = v2;
                break;
            }
            case Opcode::Mult: {
                Value v1 = stack.back(); stack.pop_back();
                Value v2 = stack.back();
                v1 = readLocal(v1, locals);
                v2 = readLocal(v2, locals);
                if (v1.type != Value::Integer || v2.type != Value::Integer) {
                    throw RuntimeError("Can only multiply integer values.");
                }
                v2.value *= v1.value;
                stack[stack.size() - 1] = v2;
                break;
            }
            case Opcode::Div: {
                Value v1 = stack.back(); stack.pop_back();
                Value v2 = stack.back();
                v1 = readLocal(v1, locals);
                v2 = readLocal(v2, locals);
                if (v1.type != Value::Integer || v2.type != Value::Integer) {
                    throw RuntimeError("Can only divide integer values.");
                }
                v2.value /= v1.value;
                stack[stack.size() - 1] = v2;
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