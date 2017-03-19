#include "Parser.h"
#include "SyntaxError.h"

namespace CommandScript
{
namespace Compiler
{
/** Generic Parser **/

void Parser::expect(Token::Keyword keyword)
{
    if (_tk->next()->asKeyword() != keyword)
        throw Exception::SyntaxError(_tk->row(), _tk->col(), Strings::format("Keyword '%s' expected", Token::keywordName(keyword)));
}

bool Parser::skipOperators(const std::unordered_set<Token::Operator> &operators)
{
    /* peek next token */
    Token::Ptr token = _tk->peek();

    if (token->type() != Token::Type::Operators ||
        operators.find(token->asOperator()) == operators.end())
        return false;

    _tk->next();
    return true;
}

bool Parser::readOperators(Token::Operator &op, const std::unordered_set<Token::Operator> &operators)
{
    /* peek next token */
    Token::Ptr token = _tk->peek();

    if (token->type() != Token::Type::Operators ||
        operators.find(token->asOperator()) == operators.end())
        return false;

    op = _tk->next()->asOperator();
    return true;
}

/** Language Structures **/

/** Statements **/

/** Control Flows **/

/** Expression Components **/

std::shared_ptr<AST::Name> Parser::parseName(void)
{
    std::shared_ptr<AST::Name> result = AST::Node::create<AST::Name>(_tk);
    result->name = _tk->next()->asIdentifier();
    return result;
}

/** Expressions **/

std::shared_ptr<AST::Unit> Parser::parseUnit(void)
{
    return std::shared_ptr<AST::Unit>();
}

std::shared_ptr<AST::Constant> Parser::parseConstant(void)
{
    Token::Ptr token = _tk->next();
    std::shared_ptr<AST::Constant> result = AST::Node::create<AST::Constant>(_tk);

    switch (token->type())
    {
        case Token::Type::Float:
        {
            result->type = AST::Constant::Type::ConstantFloat;
            result->floatValue = token->asFloat();
            break;
        }

        case Token::Type::String:
        {
            result->type = AST::Constant::Type::ConstantString;
            result->stringValue = token->asString();
            break;
        }

        case Token::Type::Integer:
        {
            result->type = AST::Constant::Type::ConstantInteger;
            result->integerValue = token->asInteger();
            break;
        }

        default:
            throw Exception::SyntaxError(_tk->row(), _tk->col(), Strings::format("Unexpected token \"%s\"", token->toString()));
    }

    return result;
}

std::shared_ptr<AST::Component> Parser::parseComponent(void)
{
    Token::Ptr token = _tk->peek();
    std::shared_ptr<AST::Component> result = AST::Node::create<AST::Component>(_tk);

    switch (token->type())
    {
        case Token::Type::Eof:
            throw Exception::SyntaxError(_tk->row(), _tk->col(), "Unexpected \"EOF\"");

        case Token::Type::Float:
        case Token::Type::String:
        case Token::Type::Integer:
        {
            result->type = AST::Component::Type::ComponentConstant;
            result->constant = parseConstant();
            break;
        }

        case Token::Type::Keywords:
        case Token::Type::Operators:
        {
            result->type = AST::Component::Type::ComponentUnit;
            result->unit = parseUnit();
            break;
        }

        case Token::Type::Identifiers:
        {
            result->type = AST::Component::Type::ComponentName;
            result->name = parseName();

            if (skipOperators({ Token::Operator::Pointer }))
            {
                result->type = AST::Component::Type::ComponentPair;
                result->pair = AST::Node::create<AST::Pair>(_tk);
                result->pair->name = std::move(result->name);
                result->pair->value = parseExpression();
            }

            break;
        }
    }

    while ((token = _tk->peekOrLine())->is<Token::Type::Operators>())
    {
        switch (token->asOperator())
        {
            /* attribute access */
            case Token::Operator::Point:
            {
                break;
            }

            /* index-like access */
            case Token::Operator::IndexLeft:
            {
                break;
            }

            /* expression or tuple-building */
            case Token::Operator::BracketLeft:
            {
                break;
            }

            /* new-line encountered */
            case Token::Operator::NewLine:
            {
                break;
            }

            /* other operators encountered, done parsing components */
            default:
                return result;
        }
    }

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseExpression(int level)
{
    /* operator set for each expression level */
    static const std::unordered_set<Token::Operator> OpSets[AST::Expression::Level::Lowest + 1] = {{
        /* Power */
        Token::Operator::Power,
    }, {
        /* Factor */
        Token::Operator::Divide,
        Token::Operator::Module,
        Token::Operator::Multiply,
    }, {
        /* Term */
        Token::Operator::Plus,
        Token::Operator::Minus,
    }, {
        /* Bitwise */
        Token::Operator::BitOr,
        Token::Operator::BitAnd,
        Token::Operator::BitXor,
        Token::Operator::ShiftLeft,
        Token::Operator::ShiftRight,
    }, {
        /* Relations */
        Token::Operator::Leq,
        Token::Operator::Geq,
        Token::Operator::Equ,
        Token::Operator::Neq,
        Token::Operator::Less,
        Token::Operator::Greater,
    }, {
        /* Expr */
        Token::Operator::BoolOr,
        Token::Operator::BoolAnd,
    }};

    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk);

    if (level == AST::Expression::Level::Highest)
    {
        result->left.node = parseComponent();
        result->left.isComponent = true;
    }
    else
    {
        result->left.node = parseExpression(level - 1);
        result->left.isComponent = false;
    }

    if (readOperators(result->op, OpSets[level]))
    {
        if (level == AST::Expression::Level::Highest)
        {
            result->right.node = parseComponent();
            result->right.isComponent = true;
        }
        else
        {
            result->right.node = parseExpression(level - 1);
            result->right.isComponent = false;
        }

        while (readOperators(op, OpSets[level]))
        {
            /* temporary node, for chainning expressions */
            std::shared_ptr<AST::Expression> node = AST::Node::create<AST::Expression>(_tk);

            node->left.node = result;
            node->left.isComponent = false;

            if (level == AST::Expression::Level::Highest)
            {
                node->right.node = parseComponent();
                node->right.isComponent = true;
            }
            else
            {
                node->right.node = parseExpression(level - 1);
                node->right.isComponent = false;
            }

            /* chain AST node */
            result = std::move(node);
        }
    }

    return result;
}

/** parser wrapper method **/

std::shared_ptr<AST::Node> Parser::parse(void)
{
    // TODO: real parsing
    return parseExpression();
}
}
}
