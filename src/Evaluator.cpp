#include "Evaluator.hpp"

void app::Evaluator::eval(const ByteCode& bytecode)
{
	if (bytecode.empty()) {
		return;
	}

	size_t position = 0;
	while (position != bytecode.size()) {
		const auto& item = bytecode[position];

		printf("== step: %zu ==\n", position);
		printStack();
		printf("\n");

		std::visit([this, &position, &bytecode](auto && arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (!std::is_same_v<T, opcode::Code>) {
				if constexpr (std::is_same_v<T, Pointer>) {
					m_pointerStack.push(arg);
				}
				else {
					m_stack.push_back(arg);					
				}

                ++position;
				return;
			}
			else {
				switch (arg) {
				case opcode::DECL:
					handleDecl(bytecode, position);
					break;

				case opcode::ASSIGN:
					handleAssign(bytecode, position);
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

				case opcode::DEFBLOCK:
				case opcode::DELBLOCK:
					handleBlocks(bytecode, position, arg);
					break;

				default:
					throw std::runtime_error("Unknown opcode");
				}
			}
		}, item);
	}

    printf("== end ==\n");
	printStack();
}

void app::Evaluator::handleDecl(const ByteCode& bytecode, size_t& position)
{
	if (m_stack.empty()) {
	    throw std::runtime_error("Unable to read DECL arguments. Stack is empty");
	}

	const auto variableName = std::get_if<std::string_view>(&m_stack.back());
	if (variableName == nullptr) {
        throw std::runtime_error("Unable to read DECL arguments. Invalid argument type");
	}

    m_variables.try_emplace(*variableName);
	if (!m_blocks.empty()) {
	    m_blocks.back().emplace(*variableName);
	}

	m_stack.pop_back();

	++position;
}

void app::Evaluator::handleAssign(const ByteCode& bytecode, size_t& position)
{
	if (m_stack.size() < 2) {
	    throw std::runtime_error("Unable to read ASSIGN arguments. Stack size is less then 2");
	}

	const auto& variableValue = m_stack.back();
    m_stack.pop_back();

	const auto variableName = std::get_if<std::string_view>(&m_stack.back());
	if (variableName == nullptr) {
        throw std::runtime_error("Unable to read ASSIGN arguments. Invalid argument type");
	}

	auto& variable = findVariable(*variableName);

	std::visit([this, &variable](auto&& arg) {
	    using T = std::decay_t<decltype(arg)>;

	    if constexpr (std::is_same_v<T, std::nullopt_t>) {
            variable.assign(nullptr);
        }
	    else if constexpr (details::is_any_of_v<T, bool, double, std::string>) {
	        variable.assign(arg);
	    }
	    else if constexpr (std::is_same_v<T, std::string_view>) {
            variable.assign(&findVariable(arg));
        }
    }, variableValue);

	m_stack.pop_back();

	++position;
}

void app::Evaluator::handleDeref(const app::ByteCode &bytecode, size_t &position)
{
    if (m_stack.empty()) {
        throw std::runtime_error("Unable to read DEREF arguments. Stack is empty");
    }

    auto& value = m_stack.back();

    std::visit([this, &value](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::string_view>) {
            findVariable(arg).deref([&value](auto&& arg) {
                value = arg;
            });
        }
    }, value);

    ++position;
}

void app::Evaluator::handlePop(const ByteCode& bytecode, size_t& position)
{
	if (m_stack.empty()) {
        throw std::runtime_error("Unable complete POP. Stack is empty");
	}

	m_stack.pop_back();

	++position;
}

void app::Evaluator::handleUnaryOperator(const app::ByteCode& bytecode, size_t& position, app::opcode::Code op)
{
    if (m_stack.empty()) {
        throw std::runtime_error("Unable to read " + std::string(toString(op)) + " argument. Stack is empty");
    }

    auto& value = m_stack.back();

    const auto opNot = [](auto&& arg) -> bool {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (details::is_any_of_v<T, bool, double>) {
            return !arg;
        }
        if constexpr (std::is_same_v<T, std::string>) {
            return arg.empty();
        }
        else {
            throw std::runtime_error("NOT is undefined for Null");
        }
    };

    const auto opUnm = [](auto&& arg) -> double {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, bool>) {
            throw std::runtime_error("UNM is undefined for bool argument");
        }
        if constexpr (std::is_same_v<T, double>) {
            return -arg;
        }
        if constexpr (std::is_same_v<T, std::string>) {
            throw std::runtime_error("UNM is undefined for string argument");
        }
        else {
            throw std::runtime_error("UNM is undefined for Null");
        }
    };

    const auto apply = [&value, op, opNot, opUnm](auto&& arg) {
        switch (op) {
            case opcode::NOT:
                value = opNot(arg);
                break;
            case opcode::UNM:
                value = opUnm(arg);
                break;
            default:
                break;
        }
    };

    deref([this, &apply](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (details::is_any_of_v<T, std::nullopt_t, bool, double, std::string>) {
            apply(arg);
        }
    }, value);

    ++position;
}

void app::Evaluator::handleBinaryOperator(const app::ByteCode& bytecode, size_t& position, app::opcode::Code op)
{
    if (m_stack.size() < 2) {
        throw std::runtime_error("Unable to read " + std::string(opcode::toString(op)) +
            " arguments. Stack size is less then 2");
    }

    auto valueRight = std::move(m_stack.back());
    m_stack.pop_back();

    auto& valueLeft = m_stack.back();

    const auto toString = [](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, bool>) {
            return arg ? "True" : "False";
        }
        else if constexpr (std::is_same_v<T, double>) {
            return std::to_string(arg);
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        }
        else {
            return "Null";
        }
    };

    enum class MathOp {
        ADD = opcode::ADD,
        SUB = opcode::SUB,
        MUL = opcode::MUL,
        DIV = opcode::DIV
    };

    const auto mathOp = [](auto&& argLeft, auto&& argRight, MathOp op) -> double {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, double> && std::is_same_v<Tr, double>) {
            switch (op) {
            case MathOp::ADD:
                return argLeft + argRight;
            case MathOp::SUB:
                return argLeft - argRight;
            case MathOp::MUL:
                return argLeft * argRight;
            case MathOp::DIV:
                return argLeft / argRight;
            }
        }

        throw std::runtime_error(std::string(opcode::toString(static_cast<opcode::Code>(op))) +
                                 " is undefined for that argument types");
    };

    enum class BoolOp {
        AND = opcode::AND,
        OR = opcode::OR,
        EQ = opcode::EQ,
        NEQ = opcode::NEQ,
        LT = opcode::LT,
        LE = opcode::LE,
        GT = opcode::GT,
        GE = opcode::GE
    };

    const auto boolOp = [](auto&& argLeft, auto&& argRight, BoolOp op) -> bool {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, bool> && std::is_same_v<Tr, bool>) {
            switch (op) {
            case BoolOp::AND:
                return argLeft && argRight;
            case BoolOp::OR:
                return argLeft || argRight;
            default:
                break;
            }
        }

        if constexpr (std::is_same_v<Tl, std::nullopt_t> && std::is_same_v<Tr, std::nullopt_t>) {
            switch (op) {
            case BoolOp::EQ:
            case BoolOp::LE:
            case BoolOp::GE:
                return true;
            case BoolOp::NEQ:
            case BoolOp::LT:
            case BoolOp::GT:
                return false;
            default:
                break;
            }
        }


        if constexpr ((details::is_any_of_v<Tl, bool, double> && details::is_any_of_v<Tr, bool, double>) ||
                (std::is_same_v<Tl, std::string> && details::is_any_of_v<Tr, std::string>))
        {
			using LType = std::conditional_t<details::is_any_of_v<Tl, bool, double>, double, std::string>;
			using RType = std::conditional_t<details::is_any_of_v<Tr, bool, double>, double, std::string>;

            switch (op) {
            case BoolOp::EQ:
                return static_cast<LType>(argLeft) == static_cast<RType>(argRight);
            case BoolOp::NEQ:
                return static_cast<LType>(argLeft) != static_cast<RType>(argRight);
            case BoolOp::LT:
                return static_cast<LType>(argLeft) < static_cast<RType>(argRight);
            case BoolOp::LE:
                return static_cast<LType>(argLeft) <= static_cast<RType>(argRight);
            case BoolOp::GT:
                return static_cast<LType>(argLeft) > static_cast<RType>(argRight);
            case BoolOp::GE:
                return static_cast<LType>(argLeft) >= static_cast<RType>(argRight);
            default:
                break;
            }
        }

        throw std::runtime_error(std::string(opcode::toString(static_cast<opcode::Code>(op))) +
                                 " is undefined for that argument types");
    };

    deref([&valueLeft, &op, &toString, &mathOp, &boolOp](auto&& argLeft, auto&& argRight) {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if (opcode::isMathOp(op)) {
            if constexpr (std::is_same_v<Tl, std::string> || std::is_same_v<Tr, std::string>) {
                if (op == opcode::ADD) {
                    valueLeft = toString(argLeft) + toString(argRight);
                    return;
                }
            }

            valueLeft = mathOp(argLeft, argRight, static_cast<MathOp>(op));
        }
        else {
            valueLeft = boolOp(argLeft, argRight, static_cast<BoolOp>(op));
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

		const auto value = std::get_if<bool>(&m_stack.back());
		if (value == nullptr) {
			throw std::runtime_error("Unable to read IF arguments. Invalid argument type");
		}

		const auto falsePointer = m_pointerStack.top();
		m_pointerStack.pop();

		const auto truePointer = m_pointerStack.top();
		m_pointerStack.pop();

		position = *value ? truePointer : falsePointer;
	};

	const auto opJmp = [this, &position](opcode::Code op) {
		if (m_pointerStack.empty()) {
			throw std::runtime_error("Unable to read " + std::string(toString(op)) + " arguments. "
				"Pointer stack is empty");
		}

		const auto pointer = m_pointerStack.top();
		m_pointerStack.pop();

		position = pointer;
	};

	const auto opCall = [this, &position]() {
		if (m_pointerStack.empty()) {
			throw std::runtime_error("Unable to read CALL arguments. "
				"Pointer stack is empty");
		}

		const auto pointer = m_pointerStack.top();
		m_pointerStack.pop();

		m_pointerStack.push(position + 1);
		position = pointer;
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

app::Symbol &app::Evaluator::findVariable(std::string_view name)
{
    const auto it = m_variables.find(name);
    if (it == m_variables.end()) {
        throw std::runtime_error("Unable to find variable: '" + std::string(name) + "'");
    }

    return it->second;
}

void app::Evaluator::printStack()
{
    if (m_stack.empty()) {
        printf("[stack is empty]\n");
    }

    for (size_t i = 0; i < m_stack.size(); ++i) {
        printf("[%zu] ", i);

        std::visit(print, m_stack[i]);

        printf("\n");
    }
}
