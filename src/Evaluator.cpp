#include "Evaluator.hpp"

#include <stdexcept>

void app::Evaluator::eval(const std::vector<Item>& bytecode)
{
	if (bytecode.empty()) {
		return;
	}

	size_t position = 0;
	while (position != bytecode.size()) {
		const auto& item = bytecode[position];

		std::visit([this, &position, &bytecode](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;

			if constexpr (!std::is_same_v<T, opcode::Code>) {
				m_stack.push(arg);
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

				case opcode::PUSH:
				case opcode::POP:
					handlePushPop(bytecode, position, arg);
					break;

				case opcode::NOT:
				case opcode::UNM:
				case opcode::INC:
				case opcode::DEC:
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
}

void app::Evaluator::handleDecl(const ByteCode& bytecode, size_t& position)
{
}

void app::Evaluator::handleAssign(const ByteCode& bytecode, size_t& position)
{
}

void app::Evaluator::handlePushPop(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}

void app::Evaluator::handleUnary(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}

void app::Evaluator::handleBinaryMath(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}

void app::Evaluator::handleBinaryLogic(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}

void app::Evaluator::handleControl(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}

void app::Evaluator::handleBlocks(const ByteCode& bytecode, size_t& position, opcode::Code op)
{
}
