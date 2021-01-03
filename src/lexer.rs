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
    /// Whitespace characters seq
    Whitespace,
    /// Simple one line comment
    LineComment,
    /// Multi-line comment
    BlockComment { terminated: bool },
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
    /// "Bang
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
    Byte { terminated: bool },
    Str { terminated: bool },
    ByteStr { terminated: bool },
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum Base {
    Binary,
    Octal,
    Hexadecimal,
    Decimal,
}

pub fn is_whitespace(c: char) -> bool {
    matches! {
        c,
        ' ' | // simple space
        '\t' | // tab
        '\n' | // new line
        '\r' | // caret return
        '\u{000B}' | // vertical tab
        '\u{000C}' | // form feed
        '\u{0085}' | // new line from latin1
        '\u{200E}' | // left-to-right mark
        '\u{200F}' | // right-to-left mark
        '\u{2028}' | // line separator
        '\u{2029}' // paragraph separator
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
            },
        };
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

    fn eat_while(&mut self, mut p: impl FnMut(char) -> bool) {
        while p(self.first()) && !self.is_eof() {
            self.bump()
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

const EOF_CHAR: char = '\0';
