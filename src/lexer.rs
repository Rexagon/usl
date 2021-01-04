use std::ops::Range;
use std::str::Chars;

#[derive(Debug)]
pub struct Token {
    pub kind: TokenKind,
    pub len: usize,
}

impl Token {
    fn new(kind: TokenKind, len: usize) -> Self {
        Self { kind, len }
    }
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum TokenKind {
    /// Simple one line comment
    LineComment,
    /// Multi-line comment
    BlockComment { terminated: bool },
    /// Whitespace characters seq
    Whitespace,
    /// Identifier
    Ident,
    /// Numeric of string literal
    Literal { kind: LiteralKind, suffix_start: usize },

    /// ";"
    Semi,
    /// ","
    Comma,
    /// "."
    Dot,
    /// "("
    OpenParen,
    /// ")"
    CloseParen,
    /// "{"
    OpenBrace,
    /// "}"
    CloseBrace,
    /// "["
    OpenBracket,
    /// "]"
    CloseBracket,
    /// "~"
    Tilde,
    /// "?"
    Question,
    /// ":"
    Colon,
    /// "="
    Eq,
    /// "!"
    Bang,
    /// "<"
    Lt,
    /// ">"
    Gt,
    /// "-"
    Minus,
    /// "&"
    And,
    /// "|"
    Or,
    /// "+"
    Plus,
    /// "*"
    Star,
    /// "/"
    Slash,
    /// "^"
    Caret,
    /// "%"
    Percent,

    /// All other
    Unknown,
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum LiteralKind {
    Int { base: Base, empty_int: bool },
    Float { base: Base },
    Char { terminated: bool },
    Str { terminated: bool },
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum Base {
    Binary,
    Octal,
    Hexadecimal,
    Decimal,
}

pub fn first_token(input: &str) -> Token {
    Cursor::new(input).advance_token()
}

pub fn tokenize(mut input: &str) -> impl Iterator<Item = Token> + '_ {
    std::iter::from_fn(move || {
        if input.is_empty() {
            return None;
        }
        let token = first_token(input);
        input = &input[token.len..];
        Some(token)
    })
}

pub fn is_whitespace(c: char) -> bool {
    match c {
        ' ' // simple space 
        | '\t' // tab
        | '\n' // new line
        | '\r' // caret return
        | '\u{000B}' // vertical tab
        | '\u{000C}' // form feed
        | '\u{0085}' // new line from latin1
        | '\u{200E}' // left-to-right mark
        | '\u{200F}' // right-to-left mark
        | '\u{2028}' // line separator
        | '\u{2029}' => true, // paragraph separator
        _ => false
    }
}

pub fn is_ident(s: &str) -> bool {
    let mut chars = s.chars();
    if let Some(c) = chars.next() {
        is_ident_start(c) && chars.all(is_ident_continue)
    } else {
        false
    }
}

pub fn is_ident_start(c: char) -> bool {
    ('a'..='z').contains(&c) || ('A'..='Z').contains(&c) || c == '_'
}

pub fn is_ident_continue(c: char) -> bool {
    ('a'..='z').contains(&c) || ('A'..='Z').contains(&c) || ('0'..='9').contains(&c) || c == '_'
}

pub struct Cursor<'a> {
    initial_len: usize,
    chars: Chars<'a>,
}

impl<'a> Cursor<'a> {
    pub fn new(input: &'a str) -> Self {
        Self {
            initial_len: input.len(),
            chars: input.chars(),
        }
    }

    fn advance_token(&mut self) -> Token {
        let first_char = self.bump().unwrap();
        let token_kind = match first_char {
            '/' => match self.first() {
                '/' => self.line_comment(),
                '*' => self.block_comment(),
                _ => TokenKind::Slash,
            },
            c if is_whitespace(c) => self.whitespace(),
            c if is_ident_start(c) => self.ident(),
            c @ '0'..='9' => {
                let kind = self.number(c);
                let suffix_start = self.len_consumed();
                self.eat_literal_suffix();
                TokenKind::Literal { kind, suffix_start }
            }
            '\'' => {
                let terminated = self.single_quoted_string();
                let suffix_start = self.len_consumed();
                if terminated {
                    self.eat_literal_suffix();
                }
                let kind = LiteralKind::Char { terminated };
                TokenKind::Literal { kind, suffix_start }
            }
            '"' => {
                let terminated = self.double_quoted_string();
                let suffix_start = self.len_consumed();
                if terminated {
                    self.eat_literal_suffix();
                }
                let kind = LiteralKind::Str { terminated };
                TokenKind::Literal { kind, suffix_start }
            }

            ';' => TokenKind::Semi,
            ',' => TokenKind::Comma,
            '.' => TokenKind::Dot,
            '(' => TokenKind::OpenParen,
            ')' => TokenKind::CloseParen,
            '{' => TokenKind::OpenBrace,
            '}' => TokenKind::CloseBrace,
            '[' => TokenKind::OpenBracket,
            ']' => TokenKind::CloseBracket,
            '~' => TokenKind::Tilde,
            '?' => TokenKind::Question,
            ':' => TokenKind::Colon,
            '=' => TokenKind::Eq,
            '!' => TokenKind::Bang,
            '<' => TokenKind::Lt,
            '>' => TokenKind::Gt,
            '-' => TokenKind::Minus,
            '&' => TokenKind::And,
            '|' => TokenKind::Or,
            '+' => TokenKind::Plus,
            '*' => TokenKind::Star,
            '^' => TokenKind::Caret,
            '%' => TokenKind::Percent,

            _ => TokenKind::Unknown,
        };

        Token::new(token_kind, self.len_consumed())
    }

    fn whitespace(&mut self) -> TokenKind {
        self.eat_while(is_whitespace);
        TokenKind::Whitespace
    }

    fn line_comment(&mut self) -> TokenKind {
        self.bump();
        self.eat_while(|c| c != '\n');
        TokenKind::LineComment
    }

    fn block_comment(&mut self) -> TokenKind {
        self.bump();

        let mut depth = 1usize;
        while let Some(c) = self.bump() {
            match c {
                '/' if self.first() == '*' => {
                    self.bump();
                    depth += 1;
                }
                '*' if self.first() == '/' => {
                    self.bump();
                    depth -= 1;
                    if depth == 0 {
                        break;
                    }
                }
                _ => {}
            }
        }

        TokenKind::BlockComment { terminated: depth == 0 }
    }

    fn ident(&mut self) -> TokenKind {
        self.eat_while(is_ident_continue);
        TokenKind::Ident
    }

    fn number(&mut self, first_digit: char) -> LiteralKind {
        let mut base = Base::Decimal;
        if first_digit == '0' {
            let has_digits = match self.first() {
                'b' => {
                    base = Base::Binary;
                    self.bump();
                    self.eat_decimal_digits()
                }
                'o' => {
                    base = Base::Octal;
                    self.bump();
                    self.eat_decimal_digits()
                }
                'x' => {
                    base = Base::Hexadecimal;
                    self.bump();
                    self.eat_hexadecimal_digits()
                }
                '0'..='9' | '_' | '.' => {
                    self.eat_decimal_digits();
                    true
                }
                _ => return LiteralKind::Int { base, empty_int: false },
            };

            if !has_digits {
                return LiteralKind::Int { base, empty_int: true };
            }
        } else {
            self.eat_decimal_digits();
        }

        match self.first() {
            '.' if self.second() != '.' && !is_ident_start(self.second()) => {
                self.bump();
                if self.first().is_digit(10) {
                    self.eat_decimal_digits();
                }
                LiteralKind::Float { base }
            }
            _ => LiteralKind::Int { base, empty_int: false },
        }
    }

    fn single_quoted_string(&mut self) -> bool {
        if self.second() == '\'' && self.first() != '\\' {
            self.bump();
            self.bump();
            return true;
        }

        loop {
            match self.first() {
                '\'' => {
                    self.bump();
                    return true;
                }
                '/' => break,
                '\n' if self.second() != '\'' => break,
                EOF_CHAR if self.is_eof() => break,
                '\\' => {
                    self.bump();
                    self.bump();
                }
                _ => {
                    self.bump();
                }
            }
        }
        false
    }

    fn double_quoted_string(&mut self) -> bool {
        while let Some(c) = self.bump() {
            match c {
                '"' => {
                    return true;
                }
                '\\' if self.first() == '\\' || self.first() == '"' => {
                    self.bump();
                }
                _ => {}
            }
        }
        false
    }

    fn eat_decimal_digits(&mut self) -> bool {
        let mut has_digits = false;
        loop {
            match self.first() {
                '_' => {
                    self.bump();
                }
                '0'..='9' => {
                    has_digits = true;
                    self.bump();
                }
                _ => break,
            }
        }
        has_digits
    }

    fn eat_hexadecimal_digits(&mut self) -> bool {
        let mut has_digits = false;
        loop {
            match self.first() {
                '_' => {
                    self.bump();
                }
                '0'..='9' | 'a'..='f' | 'A'..='F' => {
                    has_digits = true;
                    self.bump();
                }
                _ => break,
            }
        }
        has_digits
    }

    fn eat_literal_suffix(&mut self) {
        self.eat_identifier();
    }

    fn eat_identifier(&mut self) {
        if !is_ident_start(self.first()) {
            return;
        }
        self.bump();

        self.eat_while(is_ident_continue);
    }

    fn eat_while(&mut self, mut p: impl FnMut(char) -> bool) {
        while p(self.first()) && !self.is_eof() {
            self.bump();
        }
    }

    pub fn nth_char(&self, n: usize) -> char {
        self.chars().nth(n).unwrap_or(EOF_CHAR)
    }

    pub fn first(&self) -> char {
        self.nth_char(0)
    }

    pub fn second(&self) -> char {
        self.nth_char(1)
    }

    pub fn is_eof(&self) -> bool {
        self.chars.as_str().is_empty()
    }

    pub fn len_consumed(&self) -> usize {
        self.initial_len - self.chars.as_str().len()
    }

    pub fn bump(&mut self) -> Option<char> {
        self.chars.next()
    }

    fn chars(&self) -> Chars<'a> {
        self.chars.clone()
    }
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum EscapeError {
    /// Expected 1 char, but 0 were found
    ZeroChars,
    /// Expected 1 char, but more than 1 were found
    MoreThanOneChar,
    /// Escaped '\' character without continuation
    LoneSlash,
    /// Invalid escape character
    InvalidEscape,
    /// Raw '\r' encountered
    BareCarriageReturn,
    /// Unescaped character that was expected to be escaped
    EscapeOnlyChar,
}

#[derive(Debug, Copy, Clone)]
pub enum UnescapeMode {
    Char,
    Str,
}

impl UnescapeMode {
    pub fn in_single_quotes(self) -> bool {
        match self {
            UnescapeMode::Char => true,
            UnescapeMode::Str => false,
        }
    }

    pub fn in_double_quotes(self) -> bool {
        !self.in_single_quotes()
    }
}

pub fn unescape_literal<F>(literal_text: &str, mode: UnescapeMode, callback: &mut F)
where
    F: FnMut(Range<usize>, Result<char, EscapeError>),
{
    match mode {
        UnescapeMode::Char => {
            let mut chars = literal_text.chars();
            let result = unescape_char(&mut chars);
            callback(0..(literal_text.len() - chars.as_str().len()), result);
        }
        UnescapeMode::Str => unescape_str(literal_text, mode, callback),
    }
}

pub fn unescape_char(chars: &mut Chars<'_>) -> Result<char, EscapeError> {
    let first_char = chars.next().ok_or(EscapeError::ZeroChars)?;
    let res = scan_escape(first_char, chars, UnescapeMode::Char)?;
    if chars.next().is_some() {
        return Err(EscapeError::MoreThanOneChar);
    }
    Ok(res)
}

pub fn unescape_str<F>(src: &str, mode: UnescapeMode, callback: &mut F)
where
    F: FnMut(Range<usize>, Result<char, EscapeError>),
{
    fn skip_ascii_whitespace(chars: &mut Chars<'_>) {
        let s = chars.as_str();
        let first_non_space = s
            .bytes()
            .position(|b| b != b' ' && b != b'\t' && b != b'\n' && b != b'\r')
            .unwrap_or(s.len());
        *chars = s[first_non_space..].chars();
    }

    let initial_len = src.len();
    let mut chars = src.chars();
    while let Some(first_char) = chars.next() {
        let start = initial_len - chars.as_str().len() - first_char.len_utf8();

        let unescaped_char = match first_char {
            '\\' => {
                let second_char = chars.clone().next();
                match second_char {
                    Some('\n') => {
                        skip_ascii_whitespace(&mut chars);
                        continue;
                    }
                    _ => scan_escape(first_char, &mut chars, mode),
                }
            }
            '\n' => Ok('\n'),
            '\t' => Ok('\t'),
            _ => scan_escape(first_char, &mut chars, mode),
        };
        let end = initial_len - chars.as_str().len();
        callback(start..end, unescaped_char);
    }
}

fn scan_escape(first_char: char, chars: &mut Chars<'_>, mode: UnescapeMode) -> Result<char, EscapeError> {
    if first_char != '\\' {
        return match first_char {
            '\t' | '\n' => Err(EscapeError::EscapeOnlyChar),
            '\r' => Err(EscapeError::BareCarriageReturn),
            '\'' if mode.in_single_quotes() => Err(EscapeError::EscapeOnlyChar),
            '"' if mode.in_double_quotes() => Err(EscapeError::EscapeOnlyChar),
            _ => Ok(first_char),
        };
    }

    let second_char = chars.next().ok_or(EscapeError::LoneSlash)?;

    let res = match second_char {
        '"' => '"',
        'n' => '\n',
        'r' => '\r',
        't' => '\t',
        '\\' => '\\',
        '\'' => '\'',
        '0' => '\0',
        _ => return Err(EscapeError::InvalidEscape),
    };
    Ok(res)
}

const EOF_CHAR: char = '\0';

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse() {
        const PROGRAM: &str = r##"
        /*
            Strange and useless inefficient javascript parody.
        */

        // with both type of comments

        function inc(ref value, amount) {
            value = value + amount;
        }

        let first = 0.5;

        first = 100 - (first + 4.5) * 10;

        let counter = 0;
        while (first > 10) {
            inc(counter, 1);
            inc(first, -10);

            if (first < 30) {
                break;
            }
        }

        let bool_type_test = false;
        bool_type_test = first == 20;

        function factorial(num) {
            let rval = 1;
            let t = 2;
            while (t <= num) {
                rval = rval * t;
                t = t + 1;
            }

            return rval;
        }

        for (let value = 0; value < 10; inc(value, 1)) {
            std.println(factorial(value));
        }

        let ref counter_reference_test = counter;

        let uninitialized;

        if (uninitialized == null) {
            counter_reference_test = "It also\n\t\t support strings. " + counter_reference_test;
        }

        std.println("Enter 'exit' or something else:");

        while (true) {
            std.print("> ");
            let input = std.readln();
            if (input == "exit") {
                break;
            }
            std.println("");
        }
        "##;

        for (i, token) in tokenize(PROGRAM).enumerate() {
            println!("{:03}: {:?}", i, token);
        }
    }
}
