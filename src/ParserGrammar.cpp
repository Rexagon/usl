#include "ParserGrammar.hpp"

#include <cassert>

using namespace app::lexer_grammar;
using namespace app::parser_grammar;

namespace app {
    class RulesBuilder final
    {
    public:
        Rules generate()
        {
            return Rules{ m_sets };
        }

        RulesBuilder& set()
        {
            m_sets.emplace_back();
            return *this;
        }

        RulesBuilder& empty()
        {
            return *this;
        }

        RulesBuilder& term(size_t type)
        {
            assert(!m_sets.empty());
            m_sets.back().rules.emplace_back(Term {type});
            return *this;
        }

        RulesBuilder& nonterm(size_t type)
        {
            assert(!m_sets.empty());
            m_sets.back().rules.emplace_back(NonTerm {type});
            return *this;
        }

        RulesBuilder& translate(const RuleSet::Translator& translator)
        {
            assert(!m_sets.empty());
            m_sets.back().translator = translator;
            return *this;
        }

        RulesBuilder& hide()
        {
            assert(!m_sets.empty());
            m_sets.back().isImportant = false;
            return *this;
        }

    private:
        std::vector<RuleSet> m_sets;
    };
}

app::ParserGrammar::ParserGrammar()
{
	m_rules[STARTING_RULE] = RulesBuilder()
	        .set().empty().hide()
	        .set().nonterm(GeneralStatement).nonterm(STARTING_RULE).hide()
	        .generate();

	m_rules[GeneralStatement] = RulesBuilder()
	        .set().nonterm(Statement).hide()
	        .set().nonterm(FunctionDeclaration).hide()
	        .generate();

	m_rules[Statement] = RulesBuilder()
	        .set().nonterm(WhileLoop).hide()
	        .set().nonterm(Branch).hide()
	        .set().nonterm(VariableDeclaration).term(Semicolon).hide()
	        .set().nonterm(Expression).term(Semicolon).hide()
	        .set().term(KeywordReturn).nonterm(Expression).term(Semicolon).translate([](auto& cb, auto& node) {
	            cb.push(opcode::RET);
	        })
	        //.set().term(KeywordBreak).nonterm(Semicolon)
	        //.set().term(KeywordContinue).nonterm(Semicolon)
	        .generate();

	m_rules[FunctionDeclaration] = RulesBuilder()
	        .set().term(KeywordFunction).term(Identifier).term(ParenthesisOpen).nonterm(FunctionArguments)
	            .term(ParenthesisClose).nonterm(Block)
            .generate();

	m_rules[FunctionArguments] = RulesBuilder()
	        .set().empty()
	        .set().nonterm(FunctionArgument).hide()
	        .generate();

	m_rules[FunctionArgument] = RulesBuilder()
	        .set().term(Identifier).nonterm(CommaFunctionArgument)
	        .generate();

	m_rules[CommaFunctionArgument] = RulesBuilder()
	        .set().empty()
	        .set().term(Comma).term(Identifier).nonterm(CommaFunctionArgument)
	        .generate();

	m_rules[Block] = RulesBuilder()
	        .set().nonterm(Statement)
	        .set().term(BraceOpen).nonterm(BlockStatement).term(BraceClose)
	        .generate();

	m_rules[BlockStatement] = RulesBuilder()
	        .set().empty()
	        .set().nonterm(Statement).nonterm(BlockStatement).hide()
	        .generate();

	m_rules[Condition] = RulesBuilder()
	        .set().term(ParenthesisOpen).nonterm(Expression).term(ParenthesisClose)
	        .generate();

	m_rules[WhileLoop] = RulesBuilder()
	        .set().term(KeywordWhile).nonterm(Condition).nonterm(Block)
	        .generate();

	m_rules[Branch] = RulesBuilder()
	        .set().term(KeywordIf).nonterm(Condition).nonterm(Block)
	        .set().term(KeywordIf).nonterm(Condition).nonterm(Block).nonterm(ElseBranch)
	        .generate();

	m_rules[ElseBranch] = RulesBuilder()
	        .set().term(KeywordElse).nonterm(Block)
	        .generate();

	const auto declareVariable = [](CommandBuffer& cb, SyntaxNode& node, bool push = false) {
        const Token* token = *std::get_if<const Token *>(&node.children[1]->value);
        cb.push(convert(*token));
	    cb.push(opcode::DECL);

	    if (push) {
            cb.push(convert(*token));
	    }
	};

	m_rules[VariableDeclaration] = RulesBuilder()
	        .set().term(KeywordLet).term(Identifier).translate(declareVariable)
	        .set().term(KeywordLet).term(Identifier).term(OperatorAssignment).nonterm(Expression)
	            .translate([declareVariable](CommandBuffer& cb, SyntaxNode& node) {
                    declareVariable(cb, *node.children[0], true);
                    cb.translate(*node.children[2]);
                    cb.push(opcode::ASSIGN);
	            })
	        .generate();

	const auto createBinaryTranslator = [](opcode::Code op) {
	    return [op](CommandBuffer& cb, SyntaxNode& node) {
			if (node.children.size() < 3) {
				RuleSet::defaultTranslator(cb, node);
				return;
			}

            cb.translate(*node.children[0]);
            cb.translate(*node.children[2]);
            cb.push(op);
        };
    };

	m_rules[Expression] = RulesBuilder()
	        .set().nonterm(LogicalOrExpression).hide()
	        .set().nonterm(UnaryExpression).term(OperatorAssignment).nonterm(Expression)
	            .translate(createBinaryTranslator(opcode::ASSIGN))
	        .generate();

	m_rules[LogicalOrExpression] = RulesBuilder()
	        .set().nonterm(LogicalAndExpression).hide()
	        .set().nonterm(LogicalOrExpression).term(OperatorOr).nonterm(LogicalAndExpression)
	            .translate(createBinaryTranslator(opcode::OR))
	        .generate();

	m_rules[LogicalAndExpression] = RulesBuilder()
	        .set().nonterm(EqualityExpression).hide()
	        .set().nonterm(LogicalAndExpression).term(OperatorAnd).nonterm(EqualityExpression)
	            .translate(createBinaryTranslator(opcode::AND))
	        .generate();

	m_rules[EqualityExpression] = RulesBuilder()
	        .set().nonterm(RelationalExpression).hide()
	        .set().nonterm(EqualityExpression).term(OperatorEq).nonterm(RelationalExpression)
                .translate(createBinaryTranslator(opcode::EQ))
	        .set().nonterm(EqualityExpression).term(OperatorNeq).nonterm(RelationalExpression)
                .translate(createBinaryTranslator(opcode::NEQ))
	        .generate();

	m_rules[RelationalExpression] = RulesBuilder()
	        .set().nonterm(AdditiveExpression).hide()
	        .set().nonterm(RelationalExpression).term(OperatorLt).nonterm(AdditiveExpression)
                .translate(createBinaryTranslator(opcode::LT))
	        .set().nonterm(RelationalExpression).term(OperatorLeq).nonterm(AdditiveExpression)
                .translate(createBinaryTranslator(opcode::LE))
	        .set().nonterm(RelationalExpression).term(OperatorGt).nonterm(AdditiveExpression)
                .translate(createBinaryTranslator(opcode::GT))
	        .set().nonterm(RelationalExpression).term(OperatorGeq).nonterm(AdditiveExpression)
                .translate(createBinaryTranslator(opcode::GE))
	        .generate();

	m_rules[AdditiveExpression] = RulesBuilder()
	        .set().nonterm(MultiplicativeExpression).hide()
	        .set().nonterm(AdditiveExpression).term(OperatorPlus).nonterm(MultiplicativeExpression)
                .translate(createBinaryTranslator(opcode::ADD))
	        .set().nonterm(AdditiveExpression).term(OperatorMinus).nonterm(MultiplicativeExpression)
                .translate(createBinaryTranslator(opcode::SUB))
	        .generate();

	m_rules[MultiplicativeExpression] = RulesBuilder()
	        .set().nonterm(UnaryExpression).hide()
	        .set().nonterm(MultiplicativeExpression).term(OperatorMul).nonterm(UnaryExpression)
                .translate(createBinaryTranslator(opcode::MUL))
	        .set().nonterm(MultiplicativeExpression).term(OperatorDiv).nonterm(UnaryExpression)
                .translate(createBinaryTranslator(opcode::DIV))
	        .generate();

    const auto createUnaryTranslator = [](opcode::Code op, bool prefix) {
        return [op, prefix](CommandBuffer& cb, SyntaxNode& node) {
            cb.translate(*node.children[prefix ? 1 : 0]);
            cb.push(op);
        };
    };

	m_rules[UnaryExpression] = RulesBuilder()
	        .set().nonterm(PostfixExpression).hide()
	        //.set().term(OperatorIncrement).nonterm(UnaryExpression)
	        //.set().term(OperatorDecrement).nonterm(UnaryExpression)
	        .set().term(OperatorPlus).nonterm(UnaryExpression).hide()
	        .set().term(OperatorMinus).nonterm(UnaryExpression)
	            .translate(createUnaryTranslator(opcode::UNM, true))
	        .set().term(OperatorNegate).nonterm(UnaryExpression)
                .translate(createUnaryTranslator(opcode::NOT, true))
	        .generate();

	m_rules[PostfixExpression] = RulesBuilder()
	        .set().nonterm(PrimaryExpression).hide()
	        //.set().nonterm(PostfixExpression).term(OperatorIncrement)
	        //.set().nonterm(PostfixExpression).term(OperatorDecrement)
	        //.set().nonterm(PostfixExpression).term(StructureReference).term(Identifier)
	        //.set().nonterm(PostfixExpression).term(ParenthesisOpen).nonterm(CallArguments).term(ParenthesisClose)
	        .generate();

	const auto translateToken = [](CommandBuffer& cb, SyntaxNode& node) {
        const Token* token = *std::get_if<const Token *>(&node.children.front()->value);
        cb.push(convert(*token));
    };

	m_rules[PrimaryExpression] = RulesBuilder()
	        .set().term(Identifier).translate(translateToken)
	        .set().term(Null).translate(translateToken)
	        .set().term(Boolean).translate(translateToken)
	        .set().term(Number).translate(translateToken)
	        .set().term(String).translate(translateToken)
	        .set().term(ParenthesisOpen).nonterm(Expression).term(ParenthesisClose)
	        .generate();

	m_rules[CallArguments] = RulesBuilder()
	        .set().empty()
	        .set().nonterm(CallArgument).hide()
	        .generate();

	m_rules[CallArgument] = RulesBuilder()
	        .set().nonterm(Expression).nonterm(CommaCallArgument)
	        .generate();

	m_rules[CommaCallArgument] = RulesBuilder()
	        .set().empty()
	        .set().term(Comma).nonterm(Expression).nonterm(CommaCallArgument)
	        .generate();

	finalize();
}

const app::ParserGrammar& app::ParserGrammar::create()
{
	static ParserGrammar grammar;
	return grammar;
}

std::vector<app::EarleyItem> app::ParserGrammar::generateStartingEarleyItems() const
{
	return (*this)[STARTING_RULE].generateEarleyItems(0);
}

std::vector<app::EarleyItem> app::ParserGrammar::generateEarleyItems(const size_t name, const size_t origin) const
{
	assert(name < RuleName::Count);
	return (*this)[name].generateEarleyItems(origin);
}

bool app::ParserGrammar::isNullable(const size_t name)
{
	return m_nullableRules.find(name) != m_nullableRules.end();
}

const app::Rules& app::ParserGrammar::operator[](const size_t name) const
{
	assert(name < RuleName::Count);
	return m_rules[name];
}

void app::ParserGrammar::finalize()
{
	for (size_t i = 0; i < m_rules.size(); ++i) {
		m_rules[i].setName(i);
	}

	while (true) {
		const auto oldSize = m_nullableRules.size();

		for (size_t i = 0; i < m_rules.size(); ++i) {
			for (const auto& set : m_rules[i].getRuleSets()) {
				auto nullable = true;
				for (const auto& item : set.rules) {
					const auto* nonterm = std::get_if<NonTerm>(&item);
					if (nonterm == nullptr || !isNullable(nonterm->name)) {
						nullable = false;
					}
				}

				if (nullable) {
					m_nullableRules.emplace(i);
				}
			}
		}

		if (m_nullableRules.size() == oldSize) {
			break;
		}
	}
}

const char* app::parser_grammar::getString(const size_t name)
{
	switch (name) {
	case Program: 
		return "program";
	case GeneralStatement: 
		return "general_statement";
	case Statement: 
		return "statement";
	case FunctionDeclaration: 
		return "function_declaration";
	case FunctionArguments: 
		return "function_arguments";
	case FunctionArgument: 
		return "function_argument";
	case CommaFunctionArgument: 
		return "comma_function_argument";
	case Block: 
		return "block";
	case BlockStatement: 
		return "statement";
	case Condition: 
		return "condition";
	case WhileLoop: 
		return "while_loop";
	case Branch: 
		return "branch";
	case ElseBranch: 
		return "else_branch";
	case VariableDeclaration: 
		return "variable_declaration";
	case Expression: 
		return "expression";
	case LogicalOrExpression: 
		return "logical_or_expression";
	case LogicalAndExpression: 
		return "logical_and_expression";
	case EqualityExpression: 
		return "equality_expression";
	case RelationalExpression: 
		return "relational_expression";
	case AdditiveExpression: 
		return "additive_expression";
	case MultiplicativeExpression: 
		return "multiplicative_expression";
	case UnaryExpression: 
		return "unary_expression";
	case PostfixExpression: 
		return "postfix_expression";
	case PrimaryExpression: 
		return "primary_expression";
	case CallArguments: 
		return "call_arguments";
	case CallArgument: 
		return "call_argument";
	case CommaCallArgument: 
		return "comma_call_argument";

	case Count:
	default: 
		return "invalid";
	}
}