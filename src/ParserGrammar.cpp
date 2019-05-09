#include "ParserGrammar.hpp"

#include <cassert>

using namespace app::lexer_grammar;
using namespace app::parser_grammar;

namespace app {
    class RulesBuilder final
    {
    public:
        Rules generate() const
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

        RulesBuilder& term(const size_t type)
        {
            assert(!m_sets.empty());
            m_sets.back().rules.emplace_back(Term{ type });
            return *this;
        }

        RulesBuilder& nonterm(const size_t type)
        {
            assert(!m_sets.empty());
            m_sets.back().rules.emplace_back(NonTerm{ type });
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
    m_rules[STARTING_RULE] = RulesBuilder{}
        .set().empty().hide()
        .set().nonterm(GeneralStatement).nonterm(STARTING_RULE).hide()
        .generate();

    m_rules[GeneralStatement] = RulesBuilder{}
        .set().nonterm(Statement).hide()
        .set().nonterm(FunctionDeclaration).hide()
        .generate();

    m_rules[Statement] = RulesBuilder{}
        .set().nonterm(ForLoop).hide()
        .set().nonterm(DoWhileLoop).hide()
        .set().nonterm(WhileLoop).hide()
        .set().nonterm(BranchIf).hide()
        .set().nonterm(BranchIfElse).hide()
        .set().nonterm(VariableDeclaration).term(Semicolon).hide()
        .set().nonterm(VariableDeclarationEmpty).term(Semicolon)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                const auto token = std::get<const Token*>(node.children[1]->value);
                cb.push(convert(*token));
                cb.push(OpCode::DECLVAR);
            })
        .set().nonterm(Expression).term(Semicolon).hide()
        .set().term(KeywordReturn).term(Semicolon)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                cb.push(OpCode::DELBLOCK);
                cb.push(OpCode::RET);
            })
        .set().term(KeywordReturn).nonterm(Expression).term(Semicolon)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                cb.translate(*node.children[1]);
                cb.push(OpCode::DEREF);
                cb.push(OpCode::DELBLOCK);
                cb.push(OpCode::RET);
            })
        .set().term(KeywordBreak).term(Semicolon)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                cb.push(OpCode::DELBLOCK);
                cb.requestPosition(cb.getLoopEndPointerIndex());
                cb.push(OpCode::JMP);
            })
        .set().term(KeywordContinue).term(Semicolon)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                cb.push(OpCode::DELBLOCK);
                cb.requestPosition(cb.getLoopStartPointerIndex());
                cb.push(OpCode::JMP);
            })
        .generate();

    m_rules[FunctionDeclaration] = RulesBuilder{}
        .set().term(KeywordFunction).term(Identifier).nonterm(FunctionArguments).nonterm(FunctionBlock)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                const auto startPosition = cb.createPositionIndex();
                const auto endPosition = cb.createPositionIndex();

                const auto* token = std::get<const Token*>(node.children[1]->value);
                cb.push(convert(*token));
                cb.requestPosition(startPosition);
                cb.push(OpCode::DECLFUN);
                cb.requestPosition(endPosition);
                cb.push(OpCode::JMP);

                cb.replyPosition(startPosition);
                cb.push(OpCode::DEFBLOCK);
                cb.translate(*node.children[2]);

                cb.translate(*node.children[3]);

                cb.push(OpCode::DELBLOCK);
                cb.push(OpCode::RET);
                cb.replyPosition(endPosition);
            })
        .generate();

    m_rules[FunctionArguments] = RulesBuilder{}
        .set().term(ParenthesisOpen).term(ParenthesisClose)
        .set().term(ParenthesisOpen).nonterm(FunctionArgument).term(ParenthesisClose)
        .generate();

    m_rules[FunctionArgument] = RulesBuilder{}
        .set().nonterm(FunctionArgumentIdentifier).nonterm(CommaFunctionArgument).hide()
        .generate();

    m_rules[CommaFunctionArgument] = RulesBuilder{}
        .set().empty().hide()
        .set().term(Comma).nonterm(FunctionArgumentIdentifier).nonterm(CommaFunctionArgument).hide()
        .generate();

    const auto createArgumentIdentifierTranslator = [](size_t offset, bool isReference) {
        return [offset, isReference](CommandBuffer& cb, SyntaxNode& node) {
            const auto token = std::get<const Token*>(node.children[offset]->value);
            cb.push(convert(*token));
            cb.push(OpCode::DECLVAR);
            cb.push(convert(*token));
            cb.push(OpCode::POPARG);
            cb.push(isReference ? OpCode::ASSIGNREF : OpCode::ASSIGN);
        };
    };

    m_rules[FunctionArgumentIdentifier] = RulesBuilder{}
        .set().term(Identifier)
            .translate(createArgumentIdentifierTranslator(0, false))
        .set().term(KeywordRef).term(Identifier)
            .translate(createArgumentIdentifierTranslator(1, true))
        .generate();

    m_rules[FunctionBlock] = RulesBuilder{}
        .set().nonterm(Statement)
        .set().term(BraceOpen).nonterm(BlockStatement).term(BraceClose)
        .generate();

    m_rules[ForLoop] = RulesBuilder{}
        .set().term(KeywordFor).nonterm(ForCondition).nonterm(Block)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                const auto conditionStartPosition = cb.createPositionIndex();
                const auto blockStartPosition = cb.createPositionIndex();
                const auto blockEndPosition = cb.createPositionIndex();

                cb.push(OpCode::DEFBLOCK);

                cb.translate([conditionStartPosition, blockEndPosition](CommandBuffer & cb) {
                    cb.pushLoopBounds(conditionStartPosition, blockEndPosition);
                });

                const auto& condition = *node.children[1];
                cb.translate(*condition.children[1]);

                cb.replyPosition(conditionStartPosition);
                cb.translate(*condition.children[3]);
                cb.requestPosition(blockStartPosition);
                cb.requestPosition(blockEndPosition);
                cb.push(OpCode::IF);

                cb.replyPosition(blockStartPosition);
                cb.translate(*node.children[2]);

                cb.translate(*condition.children[5]);

                cb.requestPosition(conditionStartPosition);
                cb.push(OpCode::JMP);

                cb.replyPosition(blockEndPosition);

                cb.translate([](CommandBuffer & cb) {
                    cb.popLoopBounds();
                });

                cb.push(OpCode::DELBLOCK);
            })
        .generate();

    m_rules[ForCondition] = RulesBuilder{}
        .set().term(ParenthesisOpen).nonterm(ForVariableDeclaration).term(Semicolon).nonterm(ForVariableCondition)
            .term(Semicolon).nonterm(ForVariableMutation).term(ParenthesisClose)
        .generate();

    m_rules[ForVariableDeclaration] = RulesBuilder{}
        .set().empty()
        .set().nonterm(VariableDeclaration).hide()
        .generate();

    m_rules[ForVariableCondition] = RulesBuilder{}
        .set().empty()
        .set().nonterm(Expression).hide()
        .generate();

    m_rules[ForVariableMutation] = RulesBuilder{}
        .set().empty()
        .set().nonterm(Expression).hide()
        .generate();

    const auto createBlockTranslator = [](bool single) {
        return [single](CommandBuffer& cb, SyntaxNode& node) {
            cb.push(OpCode::DEFBLOCK);
            cb.translate(*node.children[single ? 0 : 1]);
            cb.push(OpCode::DELBLOCK);
        };
    };

    m_rules[Block] = RulesBuilder{}
        .set().nonterm(Statement).translate(createBlockTranslator(true))
        .set().term(BraceOpen).nonterm(BlockStatement).term(BraceClose).translate(createBlockTranslator(false))
        .generate();

    m_rules[BlockStatement] = RulesBuilder{}
        .set().empty()
        .set().nonterm(BlockStatement).nonterm(Statement)
        .generate();

    m_rules[Condition] = RulesBuilder{}
        .set().term(ParenthesisOpen).nonterm(Expression).term(ParenthesisClose)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                cb.translate(*node.children[1]);
            })
        .generate();

    m_rules[DoWhileLoop] = RulesBuilder{}
        .set().term(KeywordDo).nonterm(Block).term(KeywordWhile).nonterm(Condition)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                const auto bodyPosition = cb.createPositionIndex();
                const auto endPosition = cb.createPositionIndex();

                cb.translate([bodyPosition, endPosition](CommandBuffer& cb) {
                    cb.pushLoopBounds(bodyPosition, endPosition);
                });

                cb.replyPosition(bodyPosition);
                cb.translate(*node.children[1]);

                cb.translate(*node.children[3]);
                cb.requestPosition(bodyPosition);
                cb.requestPosition(endPosition);
                cb.push(OpCode::IF);

                cb.replyPosition(endPosition);

                cb.translate([](CommandBuffer& cb) {
                    cb.popLoopBounds();
                });
            })
        .generate();

    m_rules[WhileLoop] = RulesBuilder{}
        .set().term(KeywordWhile).nonterm(Condition).nonterm(Block)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                const auto startPosition = cb.createPositionIndex();
                const auto bodyPosition = cb.createPositionIndex();
                const auto endPosition = cb.createPositionIndex();

                cb.translate([startPosition, endPosition](CommandBuffer& cb) {
                    cb.pushLoopBounds(startPosition, endPosition);
                });

                cb.replyPosition(startPosition);
                cb.translate(*node.children[1]);
                cb.requestPosition(bodyPosition);
                cb.requestPosition(endPosition);
                cb.push(OpCode::IF);
                cb.replyPosition(bodyPosition);
                cb.translate(*node.children[2]);
                cb.requestPosition(startPosition);
                cb.push(OpCode::JMP);
                cb.replyPosition(endPosition);

                cb.translate([](CommandBuffer& cb) {
                    cb.popLoopBounds();
                });
            })
        .generate();

    m_rules[BranchIf] = RulesBuilder{}
        .set().term(KeywordIf).nonterm(Condition).nonterm(Block)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                const auto truePosition = cb.createPositionIndex();
                const auto falsePosition = cb.createPositionIndex();

                cb.translate(*node.children[1]);
                cb.requestPosition(truePosition);
                cb.requestPosition(falsePosition);
                cb.push(OpCode::IF);
                cb.replyPosition(truePosition);
                cb.translate(*node.children[2]);
                cb.replyPosition(falsePosition);
            })
        .generate();

    m_rules[BranchIfElse] = RulesBuilder{}
        .set().term(KeywordIf).nonterm(Condition).nonterm(Block).nonterm(ElseBranch)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                const auto truePosition = cb.createPositionIndex();
                const auto falsePosition = cb.createPositionIndex();
                const auto endPosition = cb.createPositionIndex();

                const auto& ifBranch = *node.children[0];
                cb.translate([&ifBranch, truePosition, falsePosition, endPosition](CommandBuffer& cb) {
                    cb.translate(*ifBranch.children[1]);
                    cb.requestPosition(truePosition);
                    cb.requestPosition(falsePosition);
                    cb.push(OpCode::IF);
                    cb.replyPosition(truePosition);
                    cb.translate(*ifBranch.children[2]);
                    cb.requestPosition(endPosition);
                    cb.push(OpCode::JMP);
                });

                cb.replyPosition(falsePosition);
                cb.translate(*node.children[1]);

                cb.replyPosition(endPosition);
            })
        .generate();

    m_rules[ElseBranch] = RulesBuilder{}
        .set().term(KeywordElse).nonterm(Block)
        .generate();

    m_rules[VariableDeclaration] = RulesBuilder{}
        .set().term(KeywordLet).term(Identifier).term(OperatorAssignment).nonterm(Expression)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                if (node.children.size() < 4) {
                    RuleSet::defaultTranslator(cb, node);
                }
                else {
                    const auto token = std::get<const Token*>(node.children[1]->value);
                    cb.push(convert(*token));
                    cb.push(OpCode::DECLVAR);
                    cb.push(convert(*token));
                    cb.translate(*node.children[3]);
                    cb.push(OpCode::ASSIGN);
                }
            })
        .set().term(KeywordLet).term(KeywordRef).term(Identifier).term(OperatorAssignment).nonterm(Expression)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                if (node.children.size() < 5) {
                    RuleSet::defaultTranslator(cb, node);
                }
                else {
                    const auto token = std::get<const Token*>(node.children[2]->value);
                    cb.push(convert(*token));
                    cb.push(OpCode::DECLVAR);
                    cb.push(convert(*token));
                    cb.translate(*node.children[4]);
                    cb.push(OpCode::ASSIGNREF);
                }
            })
        .generate();

    m_rules[VariableDeclarationEmpty] = RulesBuilder{}
        .set().term(KeywordLet).term(Identifier).hide()
        .generate();

    const auto createBinaryTranslator = [](const OpCode op) {
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

    m_rules[Expression] = RulesBuilder{}
        .set().nonterm(LogicalOrExpression).hide()
        .set().nonterm(UnaryExpression).term(OperatorAssignment).nonterm(Expression)
            .translate(createBinaryTranslator(OpCode::ASSIGN))
        .generate();

    m_rules[LogicalOrExpression] = RulesBuilder{}
        .set().nonterm(LogicalAndExpression).hide()
        .set().nonterm(LogicalOrExpression).term(OperatorOr).nonterm(LogicalAndExpression)
            .translate(createBinaryTranslator(OpCode::OR))
        .generate();

    m_rules[LogicalAndExpression] = RulesBuilder{}
        .set().nonterm(EqualityExpression).hide()
        .set().nonterm(LogicalAndExpression).term(OperatorAnd).nonterm(EqualityExpression)
            .translate(createBinaryTranslator(OpCode::AND))
        .generate();

    m_rules[EqualityExpression] = RulesBuilder{}
        .set().nonterm(RelationalExpression).hide()
        .set().nonterm(EqualityExpression).term(OperatorEq).nonterm(RelationalExpression)
            .translate(createBinaryTranslator(OpCode::EQ))
        .set().nonterm(EqualityExpression).term(OperatorNeq).nonterm(RelationalExpression)
            .translate(createBinaryTranslator(OpCode::NEQ))
        .generate();

    m_rules[RelationalExpression] = RulesBuilder{}
        .set().nonterm(AdditiveExpression).hide()
        .set().nonterm(RelationalExpression).term(OperatorLt).nonterm(AdditiveExpression)
            .translate(createBinaryTranslator(OpCode::LT))
        .set().nonterm(RelationalExpression).term(OperatorLeq).nonterm(AdditiveExpression)
            .translate(createBinaryTranslator(OpCode::LE))
        .set().nonterm(RelationalExpression).term(OperatorGt).nonterm(AdditiveExpression)
            .translate(createBinaryTranslator(OpCode::GT))
        .set().nonterm(RelationalExpression).term(OperatorGeq).nonterm(AdditiveExpression)
            .translate(createBinaryTranslator(OpCode::GE))
        .generate();

    m_rules[AdditiveExpression] = RulesBuilder{}
        .set().nonterm(MultiplicativeExpression).hide()
        .set().nonterm(AdditiveExpression).term(OperatorPlus).nonterm(MultiplicativeExpression)
            .translate(createBinaryTranslator(OpCode::ADD))
        .set().nonterm(AdditiveExpression).term(OperatorMinus).nonterm(MultiplicativeExpression)
            .translate(createBinaryTranslator(OpCode::SUB))
        .generate();

    m_rules[MultiplicativeExpression] = RulesBuilder{}
        .set().nonterm(UnaryExpression).hide()
        .set().nonterm(MultiplicativeExpression).term(OperatorMul).nonterm(UnaryExpression)
            .translate(createBinaryTranslator(OpCode::MUL))
        .set().nonterm(MultiplicativeExpression).term(OperatorDiv).nonterm(UnaryExpression)
            .translate(createBinaryTranslator(OpCode::DIV))
        .generate();

    const auto createUnaryTranslator = [](const OpCode op, const size_t offset) {
        return [op, offset](CommandBuffer& cb, SyntaxNode& node) {
            cb.translate(*node.children[offset]);
            cb.push(op);
        };
    };

    m_rules[UnaryExpression] = RulesBuilder{}
        .set().nonterm(PostfixExpression).hide()
        //.set().term(OperatorIncrement).nonterm(UnaryExpression)
        //.set().term(OperatorDecrement).nonterm(UnaryExpression)
        .set().term(OperatorPlus).nonterm(UnaryExpression).hide()
        .set().term(OperatorMinus).nonterm(UnaryExpression)
            .translate(createUnaryTranslator(OpCode::UNM, 1))
        .set().term(OperatorNegate).nonterm(UnaryExpression)
            .translate(createUnaryTranslator(OpCode::NOT, 1))
        .generate();

    m_rules[PostfixExpression] = RulesBuilder{}
        .set().nonterm(PrimaryExpression).hide()
        //.set().nonterm(PostfixExpression).term(OperatorIncrement)
        //.set().nonterm(PostfixExpression).term(OperatorDecrement)
        .set().nonterm(PostfixExpression).term(StructureReference).term(Identifier)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                cb.translate(*node.children[0]);
                const auto token = std::get<const Token*>(node.children[2]->value);
                cb.push(convert(*token));
                cb.push(OpCode::STRUCTREF);
            })
        .set().nonterm(PostfixExpression).nonterm(CallArguments)
            .translate([](CommandBuffer & cb, SyntaxNode & node) {
                cb.translate(*node.children[1]);
                cb.translate(*node.children[0]);
                cb.push(OpCode::CALL);
            })
        .generate();

    const auto translateToken = [](CommandBuffer& cb, SyntaxNode& node) {
        const auto token = std::get<const Token*>(node.children[0]->value);
        cb.push(convert(*token));
    };

    m_rules[PrimaryExpression] = RulesBuilder{}
        .set().term(Identifier).translate(translateToken)
        .set().term(Null).translate(translateToken)
        .set().term(Boolean).translate(translateToken)
        .set().term(Number).translate(translateToken)
        .set().term(String).translate(translateToken)
        .set().term(ParenthesisOpen).nonterm(Expression).term(ParenthesisClose)
        .generate();

    m_rules[CallArguments] = RulesBuilder{}
        .set().term(ParenthesisOpen).term(ParenthesisClose)
        .set().term(ParenthesisOpen).nonterm(CallArgument).term(ParenthesisClose)
            .translate([](CommandBuffer& cb, SyntaxNode& node) {
                for (size_t i = 1; i < node.children.size(); i += 2) {
                    cb.translate(*node.children[i]);
                    cb.push(OpCode::PUSHARG);
                }
            })
        .generate();

    m_rules[CallArgument] = RulesBuilder{}
        .set().nonterm(Expression).nonterm(CommaCallArgument).hide()
        .generate();

    m_rules[CommaCallArgument] = RulesBuilder{}
        .set().empty().hide()
        .set().term(Comma).nonterm(Expression).nonterm(CommaCallArgument).hide()
        .generate();

    finalize();
}

const app::ParserGrammar & app::ParserGrammar::create()
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
    case FunctionArgumentIdentifier:
        return "function_argument_identifier";
    case ForLoop:
        return "for_loop";
    case ForCondition:
        return "for_condition";
    case ForVariableDeclaration:
        return "for_variable_declaration";
    case ForVariableCondition:
        return "for_variable_condition";
    case ForVariableMutation:
        return "for_variable_mutation";
    case Block:
        return "block";
    case BlockStatement:
        return "statement";
    case Condition:
        return "condition";
    case DoWhileLoop:
        return "do_while_loop";
    case WhileLoop:
        return "while_loop";
    case BranchIf:
        return "branch_if";
    case BranchIfElse:
        return "branch_if_else";
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
