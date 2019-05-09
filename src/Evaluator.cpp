#include "Evaluator.hpp"

void app::Evaluator::eval(const std::vector<ByteCodeItem>& byteCode)
{
    size_t step = 0;
    while (m_position != byteCode.size()) {
        const auto& item = byteCode[m_position];

        printf("== step: %zu | position: %zu ==\n", step++, m_position);
        printState();

        std::visit([this](auto && arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (!std::is_same_v<T, OpCode>) {
                if constexpr (std::is_same_v<T, Pointer>) {
                    m_pointerStack.push(arg);
                }
                else if constexpr (std::is_same_v<T, std::string_view>) {
                    m_stack.emplace_back(arg);
                }
                else {
                    m_stack.emplace_back(Symbol{ arg, Symbol::ValueCategory::Rvalue });
                }

                ++m_position;
                return;
            }
            else {
                printf("[OP] %s\n", toString(arg).c_str());

                switch (arg) {
                case OpCode::DECLVAR:
                case OpCode::DECLFUN:
                    handleDecl(arg);
                    break;

                case OpCode::ASSIGN:
                case OpCode::ASSIGNREF:
                    handleAssign(arg);
                    break;

                case OpCode::POP:
                    handlePop();
                    break;

                case OpCode::DEREF:
                    handleDeref();
                    break;

                case OpCode::NOT:
                case OpCode::UNM:
                    handleUnaryOperator(arg);
                    break;

                case OpCode::ADD:
                case OpCode::SUB:
                case OpCode::MUL:
                case OpCode::DIV:
                case OpCode::AND:
                case OpCode::OR:
                case OpCode::EQ:
                case OpCode::NEQ:
                case OpCode::LT:
                case OpCode::LE:
                case OpCode::GT:
                case OpCode::GE:
                    handleBinaryOperator(arg);
                    break;

                case OpCode::IF:
                case OpCode::JMP:
                case OpCode::CALL:
                case OpCode::RET:
                    handleControl(arg);
                    break;

                case OpCode::PUSHARG:
                case OpCode::POPARG:
                    handleArguments(arg);
                    break;

                case OpCode::DEFBLOCK:
                case OpCode::DELBLOCK:
                    handleBlocks(arg);
                    break;

                default:
                    throw std::runtime_error("Unknown opcode");
                }
            }
        }, item);

        printf("\n");
    }

    printf("== end ==\n");

    printState();
}

app::Symbol& app::Evaluator::findVariable(const std::string_view name)
{
    const auto it = m_variables.find(name);
    if (it == m_variables.end()) {
        throw std::runtime_error{ "Unable to find variable: '" + std::string(name) + "'" };
    }

    return it->second;
}

void app::Evaluator::pushFunctionArgument(const Symbol& symbol)
{
    symbol.visit([this, &symbol](auto && data) {
        using D = std::decay_t<decltype(data)>;

        if constexpr (std::is_same_v<D, Symbol*>) {
            m_argumentsStack.emplace_back(data);
        }
        else {
            m_argumentsStack.emplace_back(symbol, Symbol::ValueCategory::Rvalue);
        }
    });
}

app::Symbol app::Evaluator::popFunctionArgument()
{
    if (m_argumentsStack.empty()) {
        throw std::runtime_error{ "Unable to read function arguments. Arguments stack is empty" };
    }

    auto argument = std::move(m_argumentsStack.front());
    m_argumentsStack.pop_front();

    return argument;
}

void app::Evaluator::handleDecl(const OpCode op)
{
    if (m_stack.empty()) {
        throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Stack is empty" };
    }

    const auto symbolName = std::get_if<std::string_view>(&m_stack.back());
    if (symbolName == nullptr) {
        throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Invalid argument type" };
    }

    if (op == OpCode::DECLVAR) {
        m_variables.try_emplace(*symbolName, Symbol::ValueCategory::Lvalue);
    }
    else if (op == OpCode::DECLFUN) {
        if (m_pointerStack.empty()) {
            throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Pointer stack is empty" };
        }

        const auto pointer = m_pointerStack.top();
        m_pointerStack.pop();

        m_variables.try_emplace(*symbolName, ScriptFunction{ pointer }, Symbol::ValueCategory::Lvalue);
    }

    if (!m_blocks.empty()) {
        m_blocks.back().emplace(*symbolName);
    }

    m_stack.pop_back();

    ++m_position;
}

void app::Evaluator::handleAssign(const OpCode op)
{
    if (m_stack.size() < 2) {
        throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Stack size is less then 2" };
    }

    auto variableValue = m_stack.back();
    m_stack.pop_back();

    auto& variable = m_stack.back();

    visitSymbolsPair([op](auto&& argLeft, auto&& argRight) {
        if (argLeft.getValueCategory() != Symbol::ValueCategory::Lvalue) {
            throw std::runtime_error{ "Unable to assign value to rvalue" };
        }

        if (op == OpCode::ASSIGN) {
            argLeft.assign(argRight);
        }
        else if (op == OpCode::ASSIGNREF) {
            if (argRight.getValueCategory() != Symbol::ValueCategory::Lvalue) {
                throw std::runtime_error{ "Unable to get reference of rvalue" };
            }

            argLeft = Symbol{ &argRight };
        }
    }, variable, variableValue);

    m_stack.pop_back();

    ++m_position;
}

void app::Evaluator::handleDeref()
{
    if (m_stack.empty()) {
        throw std::runtime_error{ "Unable to read DEREF arguments. Stack is empty" };
    }

    auto& value = m_stack.back();

    visitSymbol([&value](const Symbol& symbol) {
        value = symbol.deref();
    }, value);

    ++m_position;
}

void app::Evaluator::handlePop()
{
    if (!m_stack.empty()) {
        m_stack.pop_back();
    }

    ++m_position;
}

void app::Evaluator::handleUnaryOperator(const OpCode op)
{
    if (m_stack.empty()) {
        throw std::runtime_error{ "Unable to read " + toString(op) + " argument. Stack is empty" };
    }

    auto& value = m_stack.back();

    visitSymbol([&value, op](const Symbol& arg) {
        value = arg.operationUnary(op);
    }, value);

    ++m_position;
}

void app::Evaluator::handleBinaryOperator(const OpCode op)
{
    if (m_stack.size() < 2) {
        throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Stack size is less then 2" };
    }

    auto valueRight = m_stack.back();
    m_stack.pop_back();

    auto& valueLeft = m_stack.back();

    visitSymbolsPair([&valueLeft, op](const Symbol& symbolLeft, const Symbol& symbolRight) {
        if (isBinaryMathOp(op)) {
            valueLeft = symbolLeft.operationBinaryMath(symbolRight, op);
        }
        else if (isLogicOp(op)) {
            valueLeft = symbolLeft.operationLogic(symbolRight, op);
        }
        else if (isComparisonOp(op)) {
            valueLeft = symbolLeft.operationCompare(symbolRight, op);
        }
    }, valueLeft, valueRight);

    ++m_position;
}

void app::Evaluator::handleControl(const OpCode op)
{
    const auto opIf = [this]() {
        if (m_stack.empty()) {
            throw std::runtime_error{ "Unable to read IF arguments. Stack is empty" };
        }
        if (m_pointerStack.size() < 2) {
            throw std::runtime_error{ "Unable to read IF arguments. Pointer stack size is less then 2" };
        }

        auto value = false;
        visitSymbol([&value](auto && symbol) {
            symbol.visit([&value](auto && arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, std::nullopt_t>) {
                    value = false;
                    return;
                }
                else if constexpr (details::is_any_of_v<T, bool, double>) {
                    value = static_cast<bool>(arg);
                    return;
                }

                throw std::runtime_error{ "Unable to read IF arguments. Invalid argument type" };
            });
        }, m_stack.back());
        m_stack.pop_back();

        const auto falsePointer = m_pointerStack.top();
        m_pointerStack.pop();

        const auto truePointer = m_pointerStack.top();
        m_pointerStack.pop();

        m_position = value ? truePointer : falsePointer;
    };

    const auto opJmp = [this](const OpCode op) {
        if (m_pointerStack.empty()) {
            throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Pointer stack is empty" };
        }

        if (op == OpCode::RET) {
            m_argumentsStack.clear();
        }

        const auto pointer = m_pointerStack.top();
        m_pointerStack.pop();

        m_position = pointer;
    };

    const auto opCall = [this]() {
        if (m_stack.empty()) {
            throw std::runtime_error{ "Unable to read CALL arguments. Stack is empty" };
        }

        auto value = m_stack.back();
        m_stack.pop_back();

        visitSymbol([this](const Symbol & symbol) {
            symbol.visit([this](auto && arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, ScriptFunction>) {
                    m_pointerStack.push(m_position + 1);

                    m_position = arg.address;
                }
                else {
                    throw std::runtime_error("Wrong CALL argument type");
                }
            });
        }, value);
    };

    switch (op) {
    case OpCode::IF:
        opIf();
        return;
    case OpCode::JMP:
    case OpCode::RET:
        opJmp(op);
        return;
    case OpCode::CALL:
        opCall();
        return;
    default:
        return;
    }
}

void app::Evaluator::handleArguments(const OpCode op)
{
    if (op == OpCode::PUSHARG) {
        if (m_stack.empty()) {
            throw std::runtime_error{ "Unable to read PUSHARG arguments. Stack is empty" };
        }

        const auto& variable = m_stack.back();

        std::visit([this](auto && arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string_view>) {
                m_argumentsStack.emplace_back(&findVariable(arg));
            }
            else {
                pushFunctionArgument(arg);
            }
        }, variable);

        m_stack.pop_back();
    }
    else if (op == OpCode::POPARG) {
        if (m_argumentsStack.empty()) {
            throw std::runtime_error{ "Unable to read POPARG arguments. Arguments stack is empty" };
        }

        m_stack.emplace_back(popFunctionArgument());
    }

    ++m_position;
}

void app::Evaluator::handleBlocks(const OpCode op)
{
    if (op == OpCode::DEFBLOCK) {
        m_blocks.emplace_back();
    }
    else if (op == OpCode::DELBLOCK) {
        if (m_blocks.empty()) {
            throw std::runtime_error{ "Unable to delete scope block" };
        }

        for (const auto& var : m_blocks.back()) {
            m_variables.erase(var);
        }

        m_blocks.pop_back();
    }

    ++m_position;
}

void app::Evaluator::printState()
{
    if (m_stack.empty()) {
        printf("[stack is empty]\n");
    }

    for (size_t i = 0; i < m_stack.size(); ++i) {
        printf("[%zu] ", i);

        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string_view>) {
                const auto it = m_variables.find(arg);
                if (it == m_variables.end()) {
                    printf("name: %s", std::string{ arg }.c_str());
                }
                else {
                    printf("%s", it->second.getValueCategory() == Symbol::ValueCategory::Lvalue ?
                        "lvalue " : "rvalue ");
                    it->second.print();
                }
            }
            else {
                printf("%s", arg.getValueCategory() == Symbol::ValueCategory::Lvalue ? "lvalue " : "rvalue ");
                arg.print();
            }
        }, m_stack[i]);

        printf("\n");
    }

    printf("variables:\n");
    for (auto& [key, value] : m_variables) {
        printf("\t%s: ", std::string{ key }.c_str());
        value.print();
        printf("\n");
    }
}
