#include "Evaluator.hpp"

#include "CoreObject.hpp"
#include "CoreFunction.hpp"

app::Evaluator::Evaluator(const bool loggingEnabled) :
    m_loggingEnabled(loggingEnabled)
{
    m_blocks.emplace_back();
}

void app::Evaluator::eval(const std::vector<ByteCodeItem>& byteCode)
{
    size_t step = 0;
    while (m_position < byteCode.size()) {
        const auto& item = byteCode[m_position];

        if (m_loggingEnabled) {
            printf("\n== step: %zu | position: %zu ==\n", step++, m_position);
            printState(true);
        }

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
                if (m_loggingEnabled) {
                    printf("[OP] %s\n", toString(arg).c_str());
                }

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

                case OpCode::STRUCTREF:
                    handleStructRef();
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
    }

    if (m_loggingEnabled) {
        printf("\n== end ==\n");
        printState(true);
    }
}

void app::Evaluator::push(const Symbol& symbol)
{
    m_stack.emplace_back(symbol);
}

app::Symbol& app::Evaluator::findVariable(const std::string_view name)
{
    for (auto block = m_blocks.rbegin(); block != m_blocks.rend(); ++block) {
        const auto it = block->find(name);
        if (it != block->end()) {
            return it->second;
        }        
    }

    throw std::runtime_error{ "Unable to find variable: '" + std::string(name) + "'" };
}

bool app::Evaluator::hasVariable(const std::string_view name) const
{
    for (auto block = m_blocks.rbegin(); block != m_blocks.rend(); ++block) {
        const auto it = block->find(name);
        if (it != block->end()) {
            return true;
        }
    }

    return false;
}

app::Symbol app::Evaluator::popFunctionArgument()
{
    if (m_argumentsStack.empty()) {
        throw std::runtime_error{ "Unable to read function arguments. Arguments stack is empty" };
    }

    auto argument = m_argumentsStack.front();
    m_argumentsStack.pop_front();

    return argument;
}

bool app::Evaluator::hasFunctionArguments() const
{
    return !m_argumentsStack.empty();
}

size_t app::Evaluator::getFunctionArgumentCount() const
{
    return m_argumentsStack.size();
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
        const auto[it, success] = m_blocks.back().try_emplace(*symbolName, Symbol::ValueCategory::Lvalue);
        if (!success) {
            throw std::runtime_error{ "Variable with name " + std::string{ *symbolName } + "already exists" };
        }
    }
    else if (op == OpCode::DECLFUN) {
        if (m_pointerStack.empty()) {
            throw std::runtime_error{ "Unable to read " + toString(op) + " arguments. Pointer stack is empty" };
        }

        const auto pointer = m_pointerStack.top();
        m_pointerStack.pop();

        const auto[it, success] = m_blocks.back().try_emplace(*symbolName,
            ScriptFunction{ pointer }, Symbol::ValueCategory::Lvalue);

        if (!success) {
            throw std::runtime_error{ "Function with name " + std::string{ *symbolName } +"already exists" };
        }
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

    auto variable = m_stack.back();
    m_stack.pop_back();

    visitSymbolsPair([op](Symbol& argLeft, Symbol& argRight) {
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

    ++m_position;
}

void app::Evaluator::handleDeref()
{
    if (m_stack.empty()) {
        throw std::runtime_error{ "Unable to read DEREF arguments. Stack is empty" };
    }

    auto value = m_stack.back();
    m_stack.pop_back();

    visitSymbol([this](const Symbol& symbol) {
        m_stack.emplace_back(Symbol{ symbol.unref(), Symbol::ValueCategory::Rvalue });
    }, value);

    ++m_position;
}

void app::Evaluator::handleStructRef()
{
    if (m_stack.size() < 2) {
        throw std::runtime_error{ "Unable to read STRUCTREF arguments. Stack size is less then 2" };
    }

    const auto memberName = std::move(m_stack.back());
    m_stack.pop_back();

    if (!std::holds_alternative<std::string_view>(memberName)) {
        throw std::runtime_error{ "Unable to read STRUCTREF member name argument" };
    }

    auto object = m_stack.back();
    m_stack.pop_back();

    visitSymbol([this, &memberName](const Symbol & symbol) {
        symbol.unref().visit([this, &memberName](auto && arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, CoreObjectPtr>) {
                if (arg == nullptr) {
                    throw std::runtime_error{ "CoreObject is null" };
                }

                //TODO: check original core object lifetime after assignment
                const auto name = std::string{ std::get<std::string_view>(memberName) };
                auto member = arg->getMember(name);
                m_stack.emplace_back(member);
            }
            else {
                throw std::runtime_error{ "Unable to access member of non core object" };
            }
        });
    }, object);

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

    auto value = m_stack.back();
    m_stack.pop_back();

    visitSymbol([this, op](const Symbol& arg) {
        m_stack.emplace_back(arg.unref().operationUnary(op));
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

    auto valueLeft = m_stack.back();
    m_stack.pop_back();

    visitSymbolsPair([this, op](const Symbol& symbolLeft, const Symbol& symbolRight) {
        if (isBinaryMathOp(op)) {
            m_stack.emplace_back(symbolLeft.unref().operationBinaryMath(symbolRight.unref(), op));
        }
        else if (isLogicOp(op)) {
            m_stack.emplace_back(symbolLeft.unref().operationLogic(symbolRight.unref(), op));
        }
        else if (isComparisonOp(op)) {
            m_stack.emplace_back(symbolLeft.unref().operationCompare(symbolRight.unref(), op));
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
            symbol.unref().visit([&value](auto && arg) {
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
            symbol.unref().visit([this](auto && arg) {
                using T = std::decay_t<decltype(arg)>;

                if constexpr (std::is_same_v<T, ScriptFunction>) {
                    m_pointerStack.push(m_position + 1);

                    m_position = arg.address;
                }
                else if constexpr (std::is_same_v<T, CoreFunctionPtr>) {
                    arg->call(*this);
                    m_argumentsStack.clear();

                    ++m_position;
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

        auto variable = m_stack.back();
        m_stack.pop_back();

        std::visit([this](auto && arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string_view>) {
                m_argumentsStack.emplace_back(&findVariable(arg));
            }
            else {
                m_argumentsStack.emplace_back(arg);
            }
        }, variable);
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
        if (m_blocks.size() <= 1) {
            throw std::runtime_error{ "Unable to delete scope block" };
        }

        m_blocks.pop_back();
    }

    ++m_position;
}

void app::Evaluator::printState(const bool showVariables)
{
    if (m_stack.empty()) {
        printf("[stack is empty]\n");
    }

    for (size_t i = 0; i < m_stack.size(); ++i) {
        printf("[%zu] ", i);

        std::visit([](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, std::string_view>) {
                printf("%s", std::string{ arg }.c_str());
            }
            else {
                printf("%s", arg.getValueCategory() == Symbol::ValueCategory::Lvalue ? "lvalue " : "rvalue ");
                arg.print();
            }
        }, m_stack[i]);

        printf("\n");
    }

    if (showVariables) {
        printf("variables:\n");
        for (const auto& block : m_blocks) {
            for (const auto& [key, value] : block) {
                printf("\t%s: ", std::string{ key }.c_str());
                value.print();
                printf("\n");
            }
            printf("\n");
        }

        if (m_argumentsStack.empty()) {
            printf("[arguments stack is empty]\n");
        }
        else {
            printf("[arguments stack:]\n");
            for (size_t i = 0; i < m_argumentsStack.size(); ++i) {
                printf("[%zu] ", i);
                m_argumentsStack[i].print();
                printf("\n");
            }
        }
    }
}
