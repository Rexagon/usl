#include "Evaluator.hpp"

#include <cassert>

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
				m_stack.push_back(arg);
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
					handleUnary(bytecode, position, arg);
					break;

				case opcode::ADD:
				case opcode::SUB:
				case opcode::MUL:
				case opcode::DIV:
					handleBinaryMath(bytecode, position, arg);
					break;

				case opcode::AND:
				case opcode::OR:
				case opcode::EQ:
				case opcode::NEQ:
				case opcode::LT:
				case opcode::LE:
				case opcode::GT:
				case opcode::GE:
					handleBinaryLogic(bytecode, position, arg);
					break;

				case opcode::JMP:
				case opcode::CALL:
				case opcode::RET:
					handleControl(bytecode, position, arg);
					break;

				case opcode::DEFBLOCK:
				case opcode::DELBLOCK:
					handleBlocks(bytecode, position, arg);

				case opcode::Count:
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

void app::Evaluator::handleUnary(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
    if (m_stack.empty()) {
        throw std::runtime_error("Unable to read " + std::string(opcode::getString(op)) + " argument. Stack is empty");
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

void app::Evaluator::handleBinaryMath(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
    if (m_stack.size() < 2) {
        throw std::runtime_error("Unable to read " + std::string(opcode::getString(op)) +
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

    const auto opAdd = [&toString](auto&& argLeft, auto&& argRight) -> std::conditional_t<
        std::is_same_v<std::decay_t<decltype(argLeft)>, double> &&
                std::is_same_v<std::decay_t<decltype(argRight)>, double>,
        double,
        std::string
    > {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, double>) {
            if constexpr (std::is_same_v<Tr, double>) {
                return argLeft + argRight;
            }
            else if constexpr (std::is_same_v<Tr, std::string>) {
                return toString(argLeft) + argRight;
            }
        }
        if constexpr (std::is_same_v<Tl, std::string>) {
            return argLeft + toString(argRight);
        }

        throw std::runtime_error("AND is undefined for that argument types");
    };

    enum class CurrentOp {
        Sub = opcode::SUB,
        Mul = opcode::MUL,
        Div = opcode::DIV
    };

    const auto opSubMulDiv = [&toString](auto&& argLeft, auto&& argRight, CurrentOp op) -> double {
        using Tl = std::decay_t<decltype(argLeft)>;
        using Tr = std::decay_t<decltype(argRight)>;

        if constexpr (std::is_same_v<Tl, double> && std::is_same_v<Tr, double>) {
            switch (op) {
            case CurrentOp::Sub:
                return argLeft - argRight;
            case CurrentOp::Mul:
                return argLeft * argRight;
            case CurrentOp::Div:
                return argLeft / argRight;
            }
        }

        throw std::runtime_error("SUB is undefined for that argument types");
    };

    deref([&valueLeft, &op, &opAdd, &opSubMulDiv](auto&& argLeft, auto&& argRight) {
        if (op == opcode::ADD) {
            valueLeft = opAdd(argLeft, argRight);
        }
        else {
            valueLeft = opSubMulDiv(argLeft, argRight, static_cast<CurrentOp>(op));
        }
    }, valueLeft, valueRight);

    ++position;
}

void app::Evaluator::handleBinaryLogic(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	//TODO: implement binary boolean operations
}

void app::Evaluator::handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	//TODO: implement evaluation control functions
}

void app::Evaluator::handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
	//TODO: implement scope block definition and deletion
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
        print(m_stack[i]);
        printf("\n");
    }
}
