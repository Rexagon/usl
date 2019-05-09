#include "Evaluator.hpp"

void app::Evaluator::eval(const ByteCode& bytecode)
{
	if (bytecode.empty()) {
		return;
	}

	size_t step = 0;

	size_t position = 0;
	while (position != bytecode.size()) {
		const auto& item = bytecode[position];

		printf("== step: %zu | position: %zu ==\n", step++, position);
        printState();

		std::visit([this, &position, &bytecode](auto && arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (!std::is_same_v<T, opcode::Code>) {
				if constexpr (std::is_same_v<T, Pointer>) {
					m_pointerStack.push(arg);
				}
				else if constexpr (std::is_same_v<T, std::string_view>) {
				    //printf("EMPLACE VAR: %s\n", std::string{arg}.c_str());
					m_stack.emplace_back(arg);
				}
				else {
                    m_stack.emplace_back(Symbol{arg, Symbol::ValueCategory::Rvalue});
				}

                ++position;
				return;
			}
			else {
			    printf("[OP] %s\n", opcode::toString(arg));

				switch (arg) {
				case opcode::DECLVAR:
				case opcode::DECLFUN:
					handleDecl(bytecode, position, arg);
					break;

				case opcode::ASSIGN:
				case opcode::ASSIGNREF:
					handleAssign(bytecode, position, arg);
					break;

				case opcode::POP:
					handlePop(bytecode, position);
					break;

                case opcode::DEREF:
                    handleDeref(bytecode, position);
                    break;

				case opcode::NOT:
				case opcode::UNM:
					handleUnaryOperator(bytecode, position, arg);
					break;

				case opcode::ADD:
				case opcode::SUB:
				case opcode::MUL:
				case opcode::DIV:
                case opcode::AND:
                case opcode::OR:
                case opcode::EQ:
                case opcode::NEQ:
                case opcode::LT:
                case opcode::LE:
                case opcode::GT:
                case opcode::GE:
					handleBinaryOperator(bytecode, position, arg);
					break;

				case opcode::IF:
				case opcode::JMP:
				case opcode::CALL:
				case opcode::RET:
					handleControl(bytecode, position, arg);
					break;

				case opcode::PUSHARG:
				case opcode::POPARG:
                    handleArguments(bytecode, position, arg);
                    break;

				case opcode::DEFBLOCK:
				case opcode::DELBLOCK:
					handleBlocks(bytecode, position, arg);
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

app::Symbol &app::Evaluator::findVariable(std::string_view name)
{
    const auto it = m_variables.find(name);
    if (it == m_variables.end()) {
        throw std::runtime_error("Unable to find variable: '" + std::string(name) + "'");
    }

    return it->second;
}

void app::Evaluator::pushFunctionArgument(app::Symbol symbol)
{

}

app::Symbol app::Evaluator::popFunctionArgument()
{
	return Symbol{ Symbol::ValueCategory::Rvalue };
}

void app::Evaluator::handleDecl(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	if (m_stack.empty()) {
	    throw std::runtime_error("Unable to read " + std::string(toString(op)) + " arguments. Stack is empty");
	}

	const auto symbolName = std::get_if<std::string_view>(&m_stack.back());
	if (symbolName == nullptr) {
        throw std::runtime_error("Unable to read " + std::string(toString(op)) + " arguments. Invalid argument type");
	}

	if (op == opcode::DECLVAR) {
        m_variables.try_emplace(*symbolName, Symbol::ValueCategory::Lvalue);
	}
	else if (op == opcode::DECLFUN) {
	    if (m_pointerStack.empty()) {
            throw std::runtime_error("Unable to read " + std::string(toString(op)) +
                    " arguments. Pointer stack is empty");
	    }

        const auto pointer = m_pointerStack.top();
        m_pointerStack.pop();

        m_variables.try_emplace(*symbolName, ScriptFunction{pointer}, Symbol::ValueCategory::Lvalue);
	}

	if (!m_blocks.empty()) {
	    m_blocks.back().emplace(*symbolName);
	}

	m_stack.pop_back();

	++position;
}

void app::Evaluator::handleAssign(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	if (m_stack.size() < 2) {
	    throw std::runtime_error("Unable to read " + std::string{opcode::toString(op)} + " arguments."
                " Stack size is less then 2");
	}

	auto variableValue = m_stack.back();
    m_stack.pop_back();

	auto& variable = m_stack.back();

	visitSymbolsPair([this, op](auto&& argLeft, auto&& argRight) {
	    if (argLeft.getValueCategory() != Symbol::ValueCategory::Lvalue) {
	        throw std::runtime_error("Unable to assign value to rvalue");
	    }

	    if (op == opcode::ASSIGN) {
	        argLeft.assign(argRight);
	    }
	    else if (op == opcode::ASSIGNREF) {
	        if (argRight.getValueCategory() != Symbol::ValueCategory::Lvalue) {
                throw std::runtime_error("Unable to get reference of rvalue");
	        }

	        argLeft = Symbol(&argRight);
	    }
    }, variable, variableValue);

	m_stack.pop_back();

	++position;
}

void app::Evaluator::handleDeref(const app::ByteCode &bytecode, size_t &position)
{
    if (m_stack.empty()) {
        throw std::runtime_error("Unable to read DEREF arguments. Stack is empty");
    }

    auto& value = m_stack.back();

    visitSymbol([&value](const Symbol& symbol) {
        value = symbol.deref();
    }, value);

    ++position;
}

void app::Evaluator::handlePop(const ByteCode& bytecode, size_t& position)
{
	if (!m_stack.empty()) {
        m_stack.pop_back();
    }

	++position;
}

void app::Evaluator::handleUnaryOperator(const app::ByteCode& bytecode, size_t& position, app::opcode::Code op)
{
    if (m_stack.empty()) {
        throw std::runtime_error("Unable to read " + std::string(toString(op)) + " argument. Stack is empty");
    }

    auto& value = m_stack.back();

    visitSymbol([&value, op](const Symbol& arg) {
        value = arg.operationUnary(op);
    }, value);

    ++position;
}

void app::Evaluator::handleBinaryOperator(const app::ByteCode& bytecode, size_t& position, app::opcode::Code op)
{
    if (m_stack.size() < 2) {
        throw std::runtime_error("Unable to read " + std::string(opcode::toString(op)) +
            " arguments. Stack size is less then 2");
    }

    auto valueRight = m_stack.back();
    m_stack.pop_back();

    auto& valueLeft = m_stack.back();

    visitSymbolsPair([&valueLeft, op](const Symbol& symbolLeft, const Symbol& symbolRight) {
        if (opcode::isMathOp(op)) {
            valueLeft = symbolLeft.operationBinaryMath(symbolRight, op);
        }
        else if (opcode::isLogicOp(op)) {
            valueLeft = symbolLeft.operationLogic(symbolRight, op);
        }
        else if (opcode::isComparationOp(op)) {
            valueLeft = symbolLeft.operationCompare(symbolRight, op);
        }
    }, valueLeft, valueRight);

    ++position;
}

void app::Evaluator::handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	const auto opIf = [this, &position]() {
		if (m_stack.empty()) {
			throw std::runtime_error("Unable to read IF arguments. Stack is empty");
		}
		if (m_pointerStack.size() < 2) {
			throw std::runtime_error("Unable to read IF arguments. "
				    "Pointer stack size is less then 2");
		}

        bool value = false;
        visitSymbol([&value](auto&& symbol) {
		    symbol.visit([&value](auto&& arg) {
		        using T = std::decay_t<decltype(arg)>;

		        if constexpr (std::is_same_v<T, std::nullopt_t>) {
		            value = false;
                    return;
		        }
		        else if constexpr (details::is_any_of_v<T, bool, double>) {
		            value = static_cast<bool>(arg);
		            return;
		        }

                throw std::runtime_error("Unable to read IF arguments. Invalid argument type");
		    });
		}, m_stack.back());
		m_stack.pop_back();

		const auto falsePointer = m_pointerStack.top();
		m_pointerStack.pop();

		const auto truePointer = m_pointerStack.top();
		m_pointerStack.pop();

		position = value ? truePointer : falsePointer;
	};

	const auto opJmp = [this, &position](opcode::Code op) {
		if (m_pointerStack.empty()) {
			throw std::runtime_error("Unable to read " + std::string(toString(op)) + " arguments. "
				    "Pointer stack is empty");
		}

		if (op == opcode::RET) {
		    m_argumentsStack.clear();
		}

		const auto pointer = m_pointerStack.top();
		m_pointerStack.pop();

		position = pointer;
	};

	const auto opCall = [this, &position]() {
		if (m_stack.empty()) {
			throw std::runtime_error("Unable to read CALL arguments. Stack is empty");
		}

        auto value = m_stack.back();
		m_stack.pop_back();

        visitSymbol([this, &position](const Symbol& symbol) {
            symbol.visit([this, &position](auto&& arg) {
               using T = std::decay_t<decltype(arg)>;

               if constexpr (std::is_same_v<T, ScriptFunction>) {
                   m_pointerStack.push(position + 1);

                   position = arg.address;
               }
               else {
                   throw std::runtime_error("Wrong CALL argument type");
               }
            });
        }, value);
	};

	switch (op) {
	case opcode::IF:
		opIf();
		return;
	case opcode::JMP:
	case opcode::RET:
		opJmp(op);
		return;
	case opcode::CALL:
		opCall();
		return;
	default:
		break;
	}
}

void app::Evaluator::handleArguments(const app::ByteCode& byteCode, size_t& position, opcode::Code op)
{
    if (op == opcode::PUSHARG) {
        if (m_stack.empty()) {
            throw std::runtime_error("Unable to read PUSHARG arguments. Stack is empty");
        }

        auto& variable = m_stack.back();

		std::visit([this](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (std::is_same_v<T, std::string_view>) {
				m_argumentsStack.emplace_back(Symbol{ &findVariable(arg) });
			}
			else {
				arg.visit([this, &arg](auto&& data) {
					using D = std::decay_t<decltype(data)>;

					if constexpr (std::is_same_v<D, Symbol*>) {
						m_argumentsStack.emplace_back(Symbol{ data });
					}
					else {
						m_argumentsStack.emplace_back(Symbol{ arg, Symbol::ValueCategory::Rvalue });
					}
				});
			}
		}, variable);

		m_stack.pop_back();
    }
    else if (op == opcode::POPARG) {
        if (m_argumentsStack.empty()) {
            throw std::runtime_error("Unable to read POPARG arguments. Arguments stack is empty");
        }

        m_stack.push_back(m_argumentsStack.front());
        m_argumentsStack.pop_front();
    }

    ++position;
}

void app::Evaluator::handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	if (op == opcode::DEFBLOCK) {
		m_blocks.emplace_back();
	}
	else if (op == opcode::DELBLOCK) {
		if (m_blocks.empty()) {
			throw std::runtime_error("Unable to delete scope block");
		}

		for (const auto& var : m_blocks.back()) {
			m_variables.erase(var);
		}

		m_blocks.pop_back();
	}

	++position;
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
                    printf("name: %s", std::string{arg}.c_str());
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
        printf("\t%s: ", std::string{key}.c_str());
        value.print();
        printf("\n");
    }
}
