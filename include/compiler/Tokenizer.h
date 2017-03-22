#ifndef COMMANDSCRIPT_COMPILER_TOKENIZER_H
#define COMMANDSCRIPT_COMPILER_TOKENIZER_H

#include <stack>
#include <memory>
#include <string>

#include "Strings.h"
#include "NonCopyable.h"
#include "SyntaxError.h"

namespace CommandScript
{
namespace Compiler
{
class Token : public NonCopyable
{
    int _row;
    int _col;

public:
    enum class Type : int
    {
        Eof,
        Float,
        String,
        Integer,
        Keywords,
        Operators,
        Identifiers,
    };

    enum class Keyword : int
    {
        If,
        For,
        While,
        Switch,
        Case,
        Default,

        Break,
        Continue,
        Return,

        Try,
        Except,
        Finally,
        Raise,

        As,
        In,
        Def,
        Delete,
        Import,
    };

    enum class Operator : int
    {
        BracketLeft,
        BracketRight,
        IndexLeft,
        IndexRight,
        BlockLeft,
        BlockRight,

        Comma,
        Point,
        Colon,
        Semicolon,
        NewLine,

        Less,
        Greater,
        Leq,
        Geq,
        Equ,
        Neq,

        BoolAnd,
        BoolOr,
        BoolNot,

        Plus,
        Minus,
        Divide,
        Multiply,
        Module,
        Power,

        BitAnd,
        BitOr,
        BitNot,
        BitXor,
        ShiftLeft,
        ShiftRight,

        InplaceAdd,
        InplaceSub,
        InplaceMul,
        InplaceDiv,
        InplaceMod,
        InplacePower,

        InplaceBitAnd,
        InplaceBitOr,
        InplaceBitXor,
        InplaceShiftLeft,
        InplaceShiftRight,

        Assign,
        Pointer,
        Range,
        Decorator,
    };

public:
    typedef std::shared_ptr<Token> Ptr;

private:
    Type _type;

private:
    double _float = 0.0;
    int64_t _integer = 0;
    std::string _string = "";

private:
    Keyword _keyword;
    Operator _operator;

private:
    explicit Token(int row, int col) : _row(row), _col(col), _type(Type::Eof) {}
    explicit Token(int row, int col, Type type, const std::string &value) : _row(row), _col(col), _type(type), _string(value) {}

private:
    explicit Token(int row, int col, double value) : _row(row), _col(col), _type(Type::Float), _float(value) {}
    explicit Token(int row, int col, int64_t value) : _row(row), _col(col), _type(Type::Integer), _integer(value) {}

private:
    explicit Token(int row, int col, Keyword value) : _row(row), _col(col), _type(Type::Keywords), _keyword(value) {}
    explicit Token(int row, int col, Operator value) : _row(row), _col(col), _type(Type::Operators), _operator(value) {}

public:
    int row(void) const { return _row; }
    int col(void) const { return _col; }
    Type type(void) const { return _type; }

public:
    template <Type T>
    bool is(void) const { return _type == T; }

public:
    double asFloat(void) const
    {
        if (_type == Type::Float)
            return _float;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Float\" expected, but got \"%s\"", toString()));
    }

public:
    int64_t asInteger(void) const
    {
        if (_type == Type::Integer)
            return _integer;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Integer\" expected, but got \"%s\"", toString()));
    }

public:
    Keyword asKeyword(void) const
    {
        if (_type == Type::Keywords)
            return _keyword;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Keyword\" expected, but got \"%s\"", toString()));
    }

public:
    Operator asOperator(void) const
    {
        if (_type == Type::Operators)
            return _operator;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Operator\" expected, but got \"%s\"", toString()));
    }

public:
    const std::string &asString(void) const
    {
        if (_type == Type::String)
            return _string;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"String\" expected, but got \"%s\"", toString()));
    }

public:
    const std::string &asIdentifier(void) const
    {
        if (_type == Type::Identifiers)
            return _string;
        else
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Identifier\" expected, but got \"%s\"", toString()));
    }

public:
    void asEof(void) const
    {
        if (_type != Type::Eof)
            throw Exception::SyntaxError(_row, _col, Strings::format("\"Eof\" expected, but got \"%s\"", toString()));
    }

public:
    std::string toString(void) const
    {
        switch (_type)
        {
            case Type::Eof          : return "<Eof>";
            case Type::Float        : return Strings::format("<Float %f>"       , _float);
            case Type::String       : return Strings::format("<String %s>"      , Strings::repr(_string.data(), _string.size()));
            case Type::Integer      : return Strings::format("<Integer %ld>"    , _integer);
            case Type::Identifiers  : return Strings::format("<Identifier %s>"  , _string);

            case Type::Keywords     : return Strings::format("<Keyword %s>"     , keywordName(_keyword));
            case Type::Operators    : return Strings::format("<Operator '%s'>"  , operatorName(_operator));
        }
    }

public:
    static inline Ptr createEof(int row, int col) { return Ptr(new Token(row, col)); }

public:
    static inline Ptr createValue(int row, int col, double value) { return Ptr(new Token(row, col, value)); }
    static inline Ptr createValue(int row, int col, int64_t value) { return Ptr(new Token(row, col, value)); }

public:
    static inline Ptr createKeyword(int row, int col, Keyword value) { return Ptr(new Token(row, col, value)); }
    static inline Ptr createOperator(int row, int col, Operator value) { return Ptr(new Token(row, col, value)); }

public:
    static inline Ptr createString(int row, int col, const std::string &value) { return Ptr(new Token(row, col, Type::String, value)); }
    static inline Ptr createIdentifier(int row, int col, const std::string &value) { return Ptr(new Token(row, col, Type::Identifiers, value)); }

public:
    static constexpr const char *typeName(Type value)
    {
        switch (value)
        {
            case Type::Eof          : return "Eof";
            case Type::Float        : return "Float";
            case Type::String       : return "String";
            case Type::Integer      : return "Integer";
            case Type::Keywords     : return "Keyword";
            case Type::Operators    : return "Operator";
            case Type::Identifiers  : return "Identifier";
        }
    }

public:
    static constexpr const char *keywordName(Keyword value)
    {
        switch (value)
        {
            case Keyword::If        : return "if";
            case Keyword::For       : return "for";
            case Keyword::While     : return "while";
            case Keyword::Switch    : return "switch";
            case Keyword::Case      : return "case";
            case Keyword::Default   : return "default";

            case Keyword::Break     : return "break";
            case Keyword::Continue  : return "continue";
            case Keyword::Return    : return "return";

            case Keyword::Try       : return "try";
            case Keyword::Except    : return "except";
            case Keyword::Finally   : return "finally";
            case Keyword::Raise     : return "raise";

            case Keyword::As        : return "as";
            case Keyword::In        : return "in";
            case Keyword::Def       : return "def";
            case Keyword::Delete    : return "delete";
            case Keyword::Import    : return "import";
        }
    }

public:
    static constexpr const char *operatorName(Operator value)
    {
        switch (value)
        {
            case Operator::BracketLeft          : return "(";
            case Operator::BracketRight         : return ")";
            case Operator::IndexLeft            : return "[";
            case Operator::IndexRight           : return "]";
            case Operator::BlockLeft            : return "{";
            case Operator::BlockRight           : return "}";

            case Operator::Comma                : return ",";
            case Operator::Point                : return ".";
            case Operator::Colon                : return ":";
            case Operator::Semicolon            : return ";";
            case Operator::NewLine              : return "<NewLine>";

            case Operator::Less                 : return "<";
            case Operator::Greater              : return ">";
            case Operator::Leq                  : return "<=";
            case Operator::Geq                  : return ">=";
            case Operator::Equ                  : return "==";
            case Operator::Neq                  : return "!=";

            case Operator::BoolAnd              : return "and";
            case Operator::BoolOr               : return "or";
            case Operator::BoolNot              : return "not";

            case Operator::Plus                 : return "+";
            case Operator::Minus                : return "-";
            case Operator::Divide               : return "/";
            case Operator::Multiply             : return "*";
            case Operator::Module               : return "%";
            case Operator::Power                : return "**";

            case Operator::BitAnd               : return "&";
            case Operator::BitOr                : return "|";
            case Operator::BitNot               : return "~";
            case Operator::BitXor               : return "^";
            case Operator::ShiftLeft            : return "<<";
            case Operator::ShiftRight           : return ">>";

            case Operator::InplaceAdd           : return "+=";
            case Operator::InplaceSub           : return "-=";
            case Operator::InplaceMul           : return "*=";
            case Operator::InplaceDiv           : return "/=";
            case Operator::InplaceMod           : return "%=";
            case Operator::InplacePower         : return "**=";

            case Operator::InplaceBitAnd        : return "&=";
            case Operator::InplaceBitOr         : return "|=";
            case Operator::InplaceBitXor        : return "^=";
            case Operator::InplaceShiftLeft     : return "<<=";
            case Operator::InplaceShiftRight    : return ">>=";

            case Operator::Assign               : return "=";
            case Operator::Pointer              : return "->";
            case Operator::Range                : return "..";
            case Operator::Decorator            : return "@";
        }
    }
};

class Tokenizer : public NonCopyable
{
    struct State
    {
        int row;
        int col;
        int pos;
        Token::Ptr cache;
    };

private:
    State *_state;
    std::string _source;
    std::stack<State> _stack;

public:
    explicit Tokenizer(const std::string &source);

public:
    int row(void) const { return _state->row; }
    int col(void) const { return _state->col; }
    int pos(void) const { return _state->pos; }

private:
    char peekChar(void);
    char nextChar(void);

private:
    void skipSpaces(void);
    void skipComments(void);

private:
    Token::Ptr read(void);
    Token::Ptr readString(void);
    Token::Ptr readNumber(void);
    Token::Ptr readOperator(void);
    Token::Ptr readIdentifier(void);

public:
    void popState(void)
    {
        _stack.pop();
        _state = &(_stack.top());
    }

public:
    void pushState(void)
    {
        _stack.push((const State &) _stack.top());
        _state = &(_stack.top());
    }

public:
    Token::Ptr next(void);
    Token::Ptr peek(void);

public:
    Token::Ptr nextOrLine(void);
    Token::Ptr peekOrLine(void);

};
}
}

namespace std
{
template <typename T>
struct EnumHash : public hash<typename std::underlying_type<T>::type>
{
    size_t operator()(const T &value) const
    {
        /* forward to underlying type hash */
        return hash<typename std::underlying_type<T>::type>::operator()(static_cast<typename std::underlying_type<T>::type>(value));
    }
};

template <> struct hash<CommandScript::Compiler::Token::Type    > : public EnumHash<CommandScript::Compiler::Token::Type    > {};
template <> struct hash<CommandScript::Compiler::Token::Keyword > : public EnumHash<CommandScript::Compiler::Token::Keyword > {};
template <> struct hash<CommandScript::Compiler::Token::Operator> : public EnumHash<CommandScript::Compiler::Token::Operator> {};
}

#endif /* COMMANDSCRIPT_COMPILER_TOKENIZER_H */
