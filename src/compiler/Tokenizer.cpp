#include <unordered_map>
#include "Tokenizer.h"

namespace CommandScript
{
namespace Compiler
{
static const std::unordered_map<std::string, Token::Keyword> Keywords = {
    { "if"      , Token::Keyword::If        },
    { "else"    , Token::Keyword::Else      },
    { "for"     , Token::Keyword::For       },
    { "while"   , Token::Keyword::While     },
    { "switch"  , Token::Keyword::Switch    },
    { "case"    , Token::Keyword::Case      },
    { "default" , Token::Keyword::Default   },

    { "break"   , Token::Keyword::Break     },
    { "continue", Token::Keyword::Continue  },
    { "return"  , Token::Keyword::Return    },

    { "try"     , Token::Keyword::Try       },
    { "except"  , Token::Keyword::Except    },
    { "finally" , Token::Keyword::Finally   },
    { "raise"   , Token::Keyword::Raise     },

    { "as"      , Token::Keyword::As        },
    { "in"      , Token::Keyword::In        },
    { "def"     , Token::Keyword::Def       },
    { "delete"  , Token::Keyword::Delete    },
    { "import"  , Token::Keyword::Import    },
};

static const std::unordered_map<std::string, Token::Operator> Operators = {
    { "("   , Token::Operator::BracketLeft          },
    { ")"   , Token::Operator::BracketRight         },
    { "["   , Token::Operator::IndexLeft            },
    { "]"   , Token::Operator::IndexRight           },
    { "{"   , Token::Operator::BlockLeft            },
    { "}"   , Token::Operator::BlockRight           },

    { ","   , Token::Operator::Comma                },
    { "."   , Token::Operator::Point                },
    { ":"   , Token::Operator::Colon                },
    { ";"   , Token::Operator::Semicolon            },
    { "\n"  , Token::Operator::NewLine              },

    { "<"   , Token::Operator::Less                 },
    { ">"   , Token::Operator::Greater              },
    { "<="  , Token::Operator::Leq                  },
    { ">="  , Token::Operator::Geq                  },
    { "=="  , Token::Operator::Equ                  },
    { "!="  , Token::Operator::Neq                  },

    { "and" , Token::Operator::BoolAnd              },
    { "or"  , Token::Operator::BoolOr               },
    { "not" , Token::Operator::BoolNot              },

    { "+"   , Token::Operator::Plus                 },
    { "-"   , Token::Operator::Minus                },
    { "/"   , Token::Operator::Divide               },
    { "*"   , Token::Operator::Multiply             },
    { "%"   , Token::Operator::Module               },
    { "**"  , Token::Operator::Power                },

    { "&"   , Token::Operator::BitAnd               },
    { "|"   , Token::Operator::BitOr                },
    { "~"   , Token::Operator::BitNot               },
    { "^"   , Token::Operator::BitXor               },
    { "<<"  , Token::Operator::ShiftLeft            },
    { ">>"  , Token::Operator::ShiftRight           },

    { "+="  , Token::Operator::InplaceAdd           },
    { "-="  , Token::Operator::InplaceSub           },
    { "*="  , Token::Operator::InplaceMul           },
    { "/="  , Token::Operator::InplaceDiv           },
    { "%="  , Token::Operator::InplaceMod           },
    { "**=" , Token::Operator::InplacePower         },

    { "&="  , Token::Operator::InplaceBitAnd        },
    { "|="  , Token::Operator::InplaceBitOr         },
    { "^="  , Token::Operator::InplaceBitXor        },
    { "<<=" , Token::Operator::InplaceShiftLeft     },
    { ">>=" , Token::Operator::InplaceShiftRight    },

    { "="   , Token::Operator::Assign               },
    { "->"  , Token::Operator::Pointer              },
    { ".."  , Token::Operator::Range                },
    { "@"   , Token::Operator::Decorator            },
};

template <typename T> static inline bool in(T c, T a, T b)  { return c >= a && c <= b; }
template <typename T> static inline bool isHex(T c)         { return in(c, '0', '9') || in(c, 'a', 'f') || in(c, 'A', 'F'); }
template <typename T> static inline long toInt(T c)         { return in(c, '0', '9') ? (c - '0') : in(c, 'a', 'f') ? (c - 'a' + 10) : (c - 'A' + 10); }

/****** Tokenizer ******/

Tokenizer::Tokenizer(const std::string &source) : _source(source)
{
    /* initial state */
    _stack.push(State {
        .col = 0,
        .row = 0,
        .pos = 0,
    });

    /* fast reference */
    _state = &(_stack.top());
}

char Tokenizer::peekChar(void)
{
    int row = _state->row;
    int col = _state->col;
    int pos = _state->pos;
    char result = nextChar();

    _state->row = row;
    _state->col = col;
    _state->pos = pos;
    return result;
}

char Tokenizer::nextChar(void)
{
    /* check for overflow */
    if (_state->pos >= _source.size())
        return 0;

    /* peek next char */
    char result = _source[_state->pos++];

    switch (result)
    {
        case 0:
            return 0;

        case '\r':
        case '\n':
        {
            _state->row++;
            _state->col = 0;

            /* '\r\n' or '\n\r' */
            if ((_state->pos < _source.size()) &&
                (_source[_state->pos] == (result == '\n' ? '\r' : '\n')))
                _state->pos++;

            result = '\n';
            break;
        }

        /* line continuation */
        case '\\':
        {
            if ((_state->pos >= _source.size()) ||
                (_source[_state->pos] != '\r' && _source[_state->pos] != '\n'))
                break;

            _state->row++;
            _state->pos++;
            _state->col = 0;

            /* '\r\n' or '\n\r' */
            if (_state->pos < _source.size() &&
                _source[_state->pos] == (result == '\n' ? '\r' : '\n'))
                _state->pos++;

            /* check for overflow */
            if (_state->pos >= _source.size())
                return 0;

            result = _source[_state->pos++];
            break;
        }

        default:
            break;
    }

    _state->col++;
    return result;
}

void Tokenizer::skipSpaces(void)
{
    char ch = peekChar();
    while (isspace(ch) && ch != '\n')
    {
        nextChar();
        ch = peekChar();
    }
}

void Tokenizer::skipComments(void)
{
    while (peekChar() == '#')
    {
        char ch;
        nextChar();

        while ((ch = peekChar()) && (ch != '\n')) nextChar();
        while ((ch = peekChar()) && (ch == '\n')) nextChar();

        skipSpaces();
    }
}

std::shared_ptr<Token> Tokenizer::read(void)
{
    /* skip spaces and comments */
    skipSpaces();
    skipComments();

    /* read first char */
    switch (peekChar())
    {
        /* '\0' means EOF */
        case 0:
            return Token::createEof(_state->row, _state->col);

        /* strings can either be single or double quoted */
        case '\'':
        case '\"':
            return readString();

        /* number constants */
        case '0' ... '9':
            return readNumber();

        /* identifier or keywords */
        case '_':
        case 'a' ... 'z':
        case 'A' ... 'Z':
            return readIdentifier();

        /* operators */
        default:
            return readOperator();
    }
}

std::shared_ptr<Token> Tokenizer::readString(void)
{
    char start = nextChar();
    char remains = nextChar();
    std::string result;

    while (start != remains)
    {
        if (!remains)
            throw Exception::SyntaxError(_state->row, _state->col, "Unexpected EOF when scanning strings");

        if (remains == '\\')
        {
            switch ((remains = nextChar()))
            {
                case 0:
                    throw Exception::SyntaxError(_state->row, _state->col, "Unexpected EOF when parsing escape sequence in strings");

                case '\'':
                case '\"':
                case '\\':
                    break;

                case 'a' : remains = '\a'; break;
                case 'b' : remains = '\b'; break;
                case 'f' : remains = '\f'; break;
                case 'n' : remains = '\n'; break;
                case 'r' : remains = '\r'; break;
                case 't' : remains = '\t'; break;
                case 'v' : remains = '\v'; break;
                case 'e' : remains = '\033'; break;

                case 'x':
                {
                    char msb = nextChar();
                    char lsb = nextChar();

                    if (!isHex(msb) || !isHex(lsb))
                        throw Exception::SyntaxError(_state->row, _state->col, "Invalid '\\x' escape sequence");

                    remains = (char)((toInt(msb) << 4) | toInt(lsb));
                    break;
                }

                case '0' ... '7':
                {
                    /* first digit */
                    remains = (char)toInt(remains);

                    /* may have 2 more digits */
                    for (int i = 0; i < 2 && in(peekChar(), '0', '7'); i++)
                    {
                        remains <<= 3;
                        remains  |= toInt(nextChar());
                    }

                    break;
                }

                default:
                {
                    if (isprint(remains))
                        throw Exception::SyntaxError(_state->row, _state->col, Strings::format("Invalid escape character '%c'", remains));
                    else
                        throw Exception::SyntaxError(_state->row, _state->col, Strings::format("Invalid escape character '\\x%.2x'", remains));
                }
            }
        }

        result += remains;
        remains = nextChar();
    }

    return Token::createString(_state->row, _state->col, result);
}

std::shared_ptr<Token> Tokenizer::readNumber(void)
{
    int base = 10;
    char number = nextChar();
    int64_t integer = toInt(number);

    if (number == '0')
    {
        switch (peekChar())
        {
            /* decimal number */
            case '.':
                break;

            /* binary number */
            case 'b':
            case 'B':
            {
                base = 2;
                nextChar();
                break;
            }

            /* hexadecimal number */
            case 'x':
            case 'X':
            {
                base = 16;
                nextChar();
                break;
            }

            /* octal number */
            case '0' ... '7':
            {
                base = 8;
                break;
            }

            /* simply integer zero */
            default:
                return Token::createValue(_state->row, _state->col, static_cast<int64_t>(0));
        }
    }

    /* make charset */
    char charset[17] = "0123456789abcdef";
    charset[base] = 0;

    /* integer part */
    char follow = peekChar();
    while (follow && strchr(charset, tolower(follow)))
    {
        integer *= base;
        integer += toInt(follow);

        nextChar();
        follow = peekChar();
    }

    /* fraction part only makes sense when it's base 10 */
    if (base != 10 || follow != '.')
        return Token::createValue(_state->row, _state->col, integer);

    /* skip the decimal point */
    nextChar();

    /* the fraction part, but it may also be a "." or ".." opeartor */
    if (!in(peekChar(), '0', '9'))
    {
        _state->col--;
        _state->pos--;
        return Token::createValue(_state->row, _state->col, integer);
    }

    double factor = 1.0;
    double decimal = integer;

    /* merge to final result */
    do
    {
        factor *= 0.1;
        decimal += toInt(nextChar()) * factor;
    } while (in(peekChar(), '0', '9'));

    /* represent as "Float" token */
    return Token::createValue(_state->row, _state->col, decimal);
}

std::shared_ptr<Token> Tokenizer::readOperator(void)
{
    switch (char op = nextChar())
    {
        /* single character operators */
        case '(' : return Token::createOperator(_state->row, _state->col, Token::Operator::BracketLeft  );
        case ')' : return Token::createOperator(_state->row, _state->col, Token::Operator::BracketRight );
        case '[' : return Token::createOperator(_state->row, _state->col, Token::Operator::IndexLeft    );
        case ']' : return Token::createOperator(_state->row, _state->col, Token::Operator::IndexRight   );
        case '{' : return Token::createOperator(_state->row, _state->col, Token::Operator::BlockLeft    );
        case '}' : return Token::createOperator(_state->row, _state->col, Token::Operator::BlockRight   );
        case '~' : return Token::createOperator(_state->row, _state->col, Token::Operator::BitNot       );
        case ',' : return Token::createOperator(_state->row, _state->col, Token::Operator::Comma        );
        case ':' : return Token::createOperator(_state->row, _state->col, Token::Operator::Colon        );
        case ';' : return Token::createOperator(_state->row, _state->col, Token::Operator::Semicolon    );
        case '\n': return Token::createOperator(_state->row, _state->col, Token::Operator::NewLine      );
        case '@' : return Token::createOperator(_state->row, _state->col, Token::Operator::Decorator    );

        /* != */
        case '!':
        {
            if (nextChar() == '=')
                return Token::createOperator(_state->row, _state->col, Token::Operator::Neq);
            else
                throw Exception::SyntaxError(_state->row, _state->col, "Invalid operator '!'");
        }

        /* . .. */
        case '.':
        {
            /* . */
            if (peekChar() != '.')
                return Token::createOperator(_state->row, _state->col, Token::Operator::Point);

            /* .. */
            nextChar();
            return Token::createOperator(_state->row, _state->col, Token::Operator::Range);
        }

        /* = == */
        case '=':
        {
            /* = */
            if (peekChar() != '=')
                return Token::createOperator(_state->row, _state->col, Token::Operator::Assign);

            /* == */
            nextChar();
            return Token::createOperator(_state->row, _state->col, Token::Operator::Equ);
        }

        case '+': /* + += */
        case '/': /* / /= */
        case '%': /* % %= */
        case '&': /* & &= */
        case '|': /* | |= */
        case '^': /* ^ ^= */
        {
            /* + - / % & | ^ */
            if (peekChar() != '=')
                return Token::createOperator(_state->row, _state->col, Operators.at(std::string(1, op)));

            /* += -= /= %= &= |= ^= */
            nextChar();
            return Token::createOperator(_state->row, _state->col, Operators.at(op + std::string("=")));
        }

        /* - -= -> */
        case '-':
        {
            switch (peekChar())
            {
                /* -= */
                case '=':
                {
                    nextChar();
                    return Token::createOperator(_state->row, _state->col, Token::Operator::InplaceSub);
                }

                /* -> */
                case '>':
                {
                    nextChar();
                    return Token::createOperator(_state->row, _state->col, Token::Operator::Pointer);
                }

                /* - */
                default:
                    return Token::createOperator(_state->row, _state->col, Token::Operator::Minus);
            }
        }

        case '*': /* * ** *= **= */
        case '>': /* > >> >= >>= */
        case '<': /* < << <= <<= */
        {
            char follow = peekChar();

            /* *= >= <= */
            if (follow == '=')
            {
                nextChar();
                return Token::createOperator(_state->row, _state->col, Operators.at(op + std::string("=")));
            }
            else if (follow == op)
            {
                nextChar();

                /* ** << >> */
                if (peekChar() != '=')
                    return Token::createOperator(_state->row, _state->col, Operators.at(std::string(2, op)));

                /* **= <<= >>= */
                nextChar();
                return Token::createOperator(_state->row, _state->col, Operators.at(std::string(2, op) + "="));
            }
            else
            {
                /* * < > */
                return Token::createOperator(_state->row, _state->col, Operators.at(std::string(1, op)));
            }
        }

        /* other invalid operators */
        default:
        {
            if (isprint(op))
                throw Exception::SyntaxError(_state->row, _state->col, Strings::format("Invalid operator '%c'", op));
            else
                throw Exception::SyntaxError(_state->row, _state->col, Strings::format("Invalid character '\\x%.2x'", (uint8_t)op));
        }
    }
}

std::shared_ptr<Token> Tokenizer::readIdentifier(void)
{
    char first = nextChar();
    char follow = peekChar();
    std::string token(1, first);

    while (follow)
    {
        switch (follow)
        {
            case '_':
            case '0' ... '9':
            case 'a' ... 'z':
            case 'A' ... 'Z':
            {
                token += nextChar();
                follow = peekChar();
                break;
            }

            default:
            {
                follow = 0;
                break;
            }
        }
    }

    if (Keywords.find(token) != Keywords.end())
        return Token::createKeyword(_state->row, _state->col, Keywords.at(token));
    else if (Operators.find(token) != Operators.end())
        return Token::createOperator(_state->row, _state->col, Operators.at(token));
    else
        return Token::createIdentifier(_state->row, _state->col, token);
}

std::shared_ptr<Token> Tokenizer::next(void)
{
    /* read next token */
    std::shared_ptr<Token> token = nextOrLine();

    /* skip "\n" operator */
    while (token->is<Token::Type::Operators>() &&
          (token->asOperator() == Token::Operator::NewLine))
        token = nextOrLine();

    return std::move(token);
}

std::shared_ptr<Token> Tokenizer::peek(void)
{
    std::shared_ptr<Token> token;
    std::deque<std::shared_ptr<Token>>::const_iterator it;

    /* if no tokens in cache, read directly, otherwise read from cache */
    if (_state->cache.empty())
    {
        token = read();
        _state->cache.push_back(token);
    }
    else
    {
        /* skip first iterator */
        it = _state->cache.cbegin() + 1;
        token = _state->cache.front();

        /* skip "\n" operator, in cache */
        while (it != _state->cache.cend() &&
               token->is<Token::Type::Operators>() &&
              (token->asOperator() == Token::Operator::NewLine))
            token = *it++;
    }

    /* skip "\n" operator */
    while (token->is<Token::Type::Operators>() &&
          (token->asOperator() == Token::Operator::NewLine))
    {
        token = read();
        _state->cache.push_back(token);
    }

    /* preserve cache */
    return std::move(token);
}

std::shared_ptr<Token> Tokenizer::nextOrLine(void)
{
    /* no tokens in cache, read directly */
    if (_state->cache.empty())
        return read();

    /* otherwise read from cache queue */
    std::shared_ptr<Token> token = std::move(_state->cache.front());

    /* pop from cache queue */
    _state->cache.pop_front();
    return std::move(token);
}

std::shared_ptr<Token> Tokenizer::peekOrLine(void)
{
    /* tokens cached, read from cache */
    if (!_state->cache.empty())
        return _state->cache.front();

    /* otherwise read direcly */
    std::shared_ptr<Token> token = read();

    /* cache the token */
    _state->cache.push_back(token);
    return std::move(token);
}
}
}
