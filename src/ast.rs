use std::fmt;
use std::ops::Range;

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum CommentKind {
    /// `// ...`
    Line,
    /// `/* ... */`
    Block,
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum BinOpToken {
    /// `+`
    Plus,
    /// `-`
    Minus,
    /// `*`
    Star,
    /// `/`
    Slash,
    /// `%`
    Percent,
    /// `^`
    Caret,
    /// `&`
    And,
    /// `|`
    Or,
    /// `<<`
    Shl,
    /// `>>`
    Shr,
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum DelimToken {
    /// `(` or `)`
    Paren,
    /// `[` or `]`
    Bracket,
    /// `{` or `}`
    Brace,
    // Empty delimiter
    NoDelim,
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub struct Lit {
    //TODO
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub struct Ident {
    //TODO
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum ReservedIdent {
    /// `true`
    True,
    /// `false`
    False,
    /// `let`
    Let,
    /// `if`
    If,
    /// `else`
    Else,
    /// `while`
    While,
    /// `loop`
    Loop,
    /// `for`
    For,
    /// `break`
    Break,
    /// `continue`
    Continue,
    /// `function`
    Function,
    /// `return`
    Return,
    /// `await`
    Await,
    /// `yield`
    Yield,
}

#[derive(Debug, Copy, Clone, Ord, PartialOrd, Eq, PartialEq)]
pub enum TokenKind {
    /* Expression-operator symbols */
    /// `=`
    Eq,
    /// `<`
    Lt,
    /// `<=`
    Le,
    /// `==`
    EqEq,
    /// `!=`
    Ne,
    /// `>=`
    Ge,
    /// `>`
    Gt,
    /// `&&`
    AndAnd,
    /// `||`
    OrOr,
    /// `!`
    Not,
    /// `~`
    Tilde,
    /// `A op B`
    BinOp(BinOpToken),
    /// `A op= B`
    BinOpEq(BinOpToken),

    /* Structural symbols */
    /// `.`
    Dot,
    /// `..`
    DotDot,
    /// `,`
    Comma,
    /// `;`
    Semi,
    /// `,`
    Colon,
    /// `::`
    ModSep,
    /// `->`
    RArrow,
    /// `<-`
    LArrow,
    /// `=>`
    FatArrow,
    /// `?`
    Question,
    OpenDelim(DelimToken),
    CloseDelim(DelimToken),

    /* Literals */
    Literal(Lit),
    /// Any valid identifier except reserved
    Ident,
    ReservedIdent(ReservedIdent),

    /* Other */
    Eof,
}

impl TokenKind {
    pub fn similar_tokens(&self) -> Option<Vec<TokenKind>> {
        match *self {
            Self::Comma => Some(vec![Self::Dot, Self::Lt, Self::Semi]),
            Self::Semi => Some(vec![Self::Colon, Self::Comma]),
            _ => None,
        }
    }
}

#[derive(Debug, Clone, PartialOrd, Eq, PartialEq)]
pub struct Token {
    pub kind: TokenKind,
}

impl Token {
    pub fn new(kind: TokenKind) -> Self {
        Token { kind }
    }

    pub fn can_begin_expr(&self) -> bool {
        match self.kind {
            TokenKind::Ident // value
            | TokenKind::ReservedIdent(ReservedIdent::If) // if
            | TokenKind::ReservedIdent(ReservedIdent::Loop) // loop
            | TokenKind::OpenDelim(..) // tuple, array or block
            | TokenKind::Literal(..) // literal
            | TokenKind::Not // operator not
            | TokenKind::BinOp(BinOpToken::Minus) => true, // unary minus
            _ => false,
        }
    }

    pub fn can_begin_type(&self) -> bool {
        match self.kind {
            TokenKind::Ident
            | TokenKind::OpenDelim(DelimToken::Paren) // tuple
            | TokenKind::OpenDelim(DelimToken::Bracket) => true, // array
            _ => false
        }
    }

    pub fn is_lit(&self) -> bool {
        match self.kind {
            TokenKind::Literal(..) => true,
            _ => false,
        }
    }

    pub fn can_begin_literal_maybe_minus(&self) -> bool {
        match self.kind {
            TokenKind::Literal(..)
            | TokenKind::BinOp(BinOpToken::Minus)
            | TokenKind::ReservedIdent(ReservedIdent::False)
            | TokenKind::ReservedIdent(ReservedIdent::True) => true,
            _ => false,
        }
    }

    pub fn is_ident(&self) -> bool {
        match self.kind {
            TokenKind::Ident | TokenKind::ReservedIdent(..) => true,
            _ => false,
        }
    }

    pub fn is_reserved_ident(&self) -> bool {
        match self.kind {
            TokenKind::ReservedIdent(..) => true,
            _ => false,
        }
    }

    pub fn is_bool_lit(&self) -> bool {
        match self.kind {
            TokenKind::ReservedIdent(ReservedIdent::False) | TokenKind::ReservedIdent(ReservedIdent::True) => true,
            _ => false,
        }
    }

    pub fn glue(&self, joint: &Token) -> Option<Token> {
        let kind = match self.kind {
            TokenKind::Eq => match joint.kind {
                TokenKind::Eq => TokenKind::EqEq,
                TokenKind::Gt => TokenKind::FatArrow,
                _ => return None,
            },
            TokenKind::Lt => match joint.kind {
                TokenKind::Eq => TokenKind::Le,
                TokenKind::Lt => TokenKind::BinOp(BinOpToken::Shl),
                TokenKind::Le => TokenKind::BinOpEq(BinOpToken::Shl),
                TokenKind::BinOp(BinOpToken::Minus) => TokenKind::LArrow,
                _ => return None,
            },
            TokenKind::Gt => match joint.kind {
                TokenKind::Eq => TokenKind::Ge,
                TokenKind::Gt => TokenKind::BinOp(BinOpToken::Shr),
                TokenKind::Ge => TokenKind::BinOpEq(BinOpToken::Shr),
                _ => return None,
            },
            TokenKind::Not => match joint.kind {
                TokenKind::Eq => TokenKind::Ne,
                _ => return None,
            },
            TokenKind::BinOp(op) => match joint.kind {
                TokenKind::Eq => TokenKind::BinOpEq(op),
                TokenKind::BinOp(BinOpToken::And) if op == BinOpToken::And => TokenKind::AndAnd,
                TokenKind::BinOp(BinOpToken::Or) if op == BinOpToken::Or => TokenKind::OrOr,
                TokenKind::Gt if op == BinOpToken::Minus => TokenKind::RArrow,
                _ => return None,
            },
            TokenKind::Dot => match joint.kind {
                TokenKind::Dot => TokenKind::DotDot,
                _ => return None,
            },
            TokenKind::Colon => match joint.kind {
                TokenKind::Colon => TokenKind::ModSep,
                _ => return None,
            },
            _ => return None,
        };

        Some(Token::new(kind))
    }
}

impl PartialEq<TokenKind> for Token {
    fn eq(&self, rhs: &TokenKind) -> bool {
        self.kind == *rhs
    }
}
