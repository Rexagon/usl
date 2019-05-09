#pragma once

#include "LexerGrammar.hpp"

namespace app
{
    class Lexer final
    {
    public:
        explicit Lexer();

        std::vector<Token> run(std::string_view text) const;

    private:
        const RegexArray& m_regexes;
    };
}
