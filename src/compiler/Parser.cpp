#include "Parser.h"
#include "SyntaxError.h"

namespace CommandScript
{
namespace Compiler
{
/** Generic Parser **/

void Parser::expect(Token::Keyword expect)
{
    if (_tk->next()->asKeyword() != expect)
        throw Exception::SyntaxError(_tk->row(), _tk->col(), Strings::format("Keyword \"%s\" expected", Token::keywordName(expect)));
}

void Parser::expect(Token::Operator expect)
{
    if (_tk->next()->asOperator() != expect)
        throw Exception::SyntaxError(_tk->row(), _tk->col(), Strings::format("Operator \"%s\" expected", Token::operatorName(expect)));
}

bool Parser::isKeyword(Token::Keyword expected)
{
    std::shared_ptr<Token> token = _tk->peek();
    return token->is<Token::Type::Keywords>() && (token->asKeyword() == expected);
}

bool Parser::skipKeyword(Token::Keyword expected)
{
    if (!isKeyword(expected))
        return false;

    _tk->next();
    return true;
}

bool Parser::isOperator(Token::Operator expected)
{
    std::shared_ptr<Token> token = _tk->peek();
    return token->is<Token::Type::Operators>() && (token->asOperator() == expected);
}

bool Parser::skipOperator(Token::Operator expected)
{
    if (!isOperator(expected))
        return false;

    _tk->next();
    return true;
}

bool Parser::readOperators(Token::Operator &op, const std::unordered_set<Token::Operator> &operators)
{
    /* peek next token */
    std::shared_ptr<Token> token = _tk->peek();

    if (!token->is<Token::Type::Operators>() ||
        (operators.find(token->asOperator()) == operators.end()))
        return false;

    op = _tk->next()->asOperator();
    return true;
}

bool Parser::unpackPointerPair(std::shared_ptr<AST::Expression> &expr, std::shared_ptr<AST::Name> &name)
{
    while (expr->first.type == AST::Expression::Type::TermExpression)
    {
        /* a valid arg name cannot have right branch */
        if (!expr->remains.empty())
            return false;

        /* move to next level */
        expr = expr->first.expression;
    }

    /* a valid arg name must be a single name expression */
    if (!expr->remains.empty() ||
        (expr->first.component->type != AST::Component::Type::ComponentPair))
        return false;

    name = expr->first.component->pair->name;
    expr = expr->first.component->pair->value;
    return true;
}

bool Parser::extractArgumentName(std::shared_ptr<AST::Expression> expr, std::shared_ptr<AST::Name> &name)
{
    while (expr->first.type == AST::Expression::Type::TermExpression)
    {
        /* a valid arg name cannot have right branch */
        if (!expr->remains.empty())
            return false;

        /* move to next level */
        expr = expr->first.expression;
    }

    /* a valid arg name must be a single name expression */
    if (!expr->remains.empty() ||
        (expr->first.component->type != AST::Component::Type::ComponentPair))
        return false;

    name = expr->first.component->name;
    return true;
}

/** Language Structures **/

std::shared_ptr<AST::If> Parser::parseIf(void)
{
    expect(Token::Keyword::If);
    std::shared_ptr<AST::If> result = AST::Node::create<AST::If>(_tk);

    expect(Token::Operator::BracketLeft);
    result->expr = parseExpression();
    expect(Token::Operator::BracketRight);
    result->positive = parseStatement();

    /* may have `else` section */
    if (skipKeyword(Token::Keyword::Else))
        result->negative = parseStatement();

    return result;
}

std::shared_ptr<AST::For> Parser::parseFor(void)
{
    expect(Token::Keyword::For);
    std::shared_ptr<AST::For> result = AST::Node::create<AST::For>(_tk);

    expect(Token::Operator::BracketLeft);
    result->seq = AST::Node::create<AST::Sequence>(_tk);
    result->seq->isSeq = false;

    do
    {
        if (!skipOperator(Token::Operator::BracketLeft))
        {
            /* parse selectively add to items list */
            result->seq->items.push_back(parseSubSequence());
        }
        else
        {
            result->seq->isSeq = true;
            result->seq->items.push_back(parseSequence());
            expect(Token::Operator::BracketRight);
        }

        if (skipOperator(Token::Operator::Comma))
            result->seq->isSeq = true;

    } while (!isOperator(Token::Operator::In));

    expect(Token::Operator::In);
    result->expr = parseExpression();
    expect(Token::Operator::BracketRight);
    result->body = parseStatement();
    return result;
}

std::shared_ptr<AST::While> Parser::parseWhile(void)
{
    expect(Token::Keyword::While);
    std::shared_ptr<AST::While> result = AST::Node::create<AST::While>(_tk);

    expect(Token::Operator::BracketLeft);
    result->expr = parseExpression();
    expect(Token::Operator::BracketRight);
    result->body = parseStatement();
    return result;
}

std::shared_ptr<AST::Define> Parser::parseDefine(void)
{
    expect(Token::Keyword::Def);
    std::shared_ptr<AST::Define> result = AST::Node::create<AST::Define>(_tk);

    result->name = parseName();
    expect(Token::Operator::BracketLeft);

    if (!isOperator(Token::Operator::BracketRight))
    {
        do result->args.push_back(parseName());
        while (skipOperator(Token::Operator::Comma));
    }

    expect(Token::Operator::BracketRight);
    result->body = parseStatement();
    return result;
}

std::shared_ptr<AST::Import> Parser::parseImport(void)
{
    expect(Token::Keyword::Import);
    std::shared_ptr<AST::Import> result = AST::Node::create<AST::Import>(_tk);

    do result->names.push_back(parseName());
    while (skipOperator(Token::Operator::Point));
    return result;
}

/** Statements **/

std::shared_ptr<AST::Sequence> Parser::parseSequence(void)
{
    /* create new seqnece */
    std::shared_ptr<AST::Sequence> result = AST::Node::create<AST::Sequence>(_tk);

    do
    {
        /* check for nested sequence */
        if (!skipOperator(Token::Operator::BracketLeft))
        {
            /* parse selectively add to items list */
            result->items.push_back(parseSubSequence());
        }
        else
        {
            result->items.push_back(parseSequence());
            expect(Token::Operator::BracketRight);
        }

        /* continues iff the next token is a comma */
        if (!skipOperator(Token::Operator::Comma))
        {
            if (result->items.size() > 1)
                break;
            else
                throw Exception::SyntaxError(_tk->row(), _tk->col(), "Single-item sequences must have an extra comma");
        }
    } while (!isOperator(Token::Operator::BracketRight));

    /* this result is definately a sequence */
    result->isSeq = true;
    return result;
}

std::shared_ptr<AST::Component> Parser::parseSubSequence(void)
{
    /* parse next component item */
    std::shared_ptr<AST::Component> result = parseComponent();

    /* component must be mutable */
    if (result->modifiers.empty())
    {
        /* only names are mutable */
        if (result->type != AST::Component::Type::ComponentName)
            throw Exception::SyntaxError(_tk->row(), _tk->col(), "Component must be mutable");
    }
    else
    {
        /* invoke modifier is not mutable */
        if (result->modifiers.back().type == AST::Component::ModType::ModifierInvoke)
            throw Exception::SyntaxError(_tk->row(), _tk->col(), "Component must be mutable");
    }

    return result;
}

std::shared_ptr<AST::Tuple> Parser::parseTupleExpression(bool &isSeq)
{
    /* create tuple result */
    std::shared_ptr<AST::Tuple> result = AST::Node::create<AST::Tuple>(_tk);

    /* we assume it's not sequence at start */
    for (isSeq = false;;)
    {
        /* read next expression */
        result->items.push_back(parseExpression());

        /* read next token */
        bool isEnd = false;
        std::shared_ptr<Token> token = _tk->peekOrLine();

        /* once it encountered a comma, it definately a sequence */
        if (token->is<Token::Type::Operators>() &&
            token->asOperator() == Token::Operator::Comma)
        {
            /* skip this comma */
            _tk->nextOrLine();

            /* and peek next token */
            isEnd = true;
            isSeq = true;
            token = _tk->peekOrLine();
        }

        /* stop sequencing when encounters "\n", ";" or `EOF` */
        switch (token->type())
        {
            case Token::Type::Eof:
                return result;

            case Token::Type::Keywords:
                throw Exception::SyntaxError(_tk->row(), _tk->col(), "Unexpected token " + token->toString());

            case Token::Type::Operators:
            {
                switch (token->asOperator())
                {
                    case Token::Operator::NewLine:
                    case Token::Operator::Semicolon:
                    {
                        _tk->nextOrLine();
                        return result;
                    }

                    default:
                        break;
                }

                break;
            }

            default:
            {
                if (isEnd)
                    break;
                else
                    throw Exception::SyntaxError(_tk->row(), _tk->col(), "Unexpected token " + token->toString());
            }
        }
    }
}

std::shared_ptr<AST::Compond> Parser::parseCompond(void)
{
    expect(Token::Operator::BlockLeft);
    std::shared_ptr<AST::Compond> result = AST::Node::create<AST::Compond>(_tk);

    while (!isOperator(Token::Operator::BlockRight))
        result->statements.push_back(parseStatement());

    expect(Token::Operator::BlockRight);
    return std::shared_ptr<AST::Compond>();
}

std::shared_ptr<AST::Statement> Parser::parseStatement(void)
{
    // TODO: actual parse statement
    return std::shared_ptr<AST::Statement>();
}

/** Control Flows **/

std::shared_ptr<AST::Break> Parser::parseBreak(void)
{
    expect(Token::Keyword::Break);
    return AST::Node::create<AST::Break>(_tk);
}

std::shared_ptr<AST::Return> Parser::parseReturn(void)
{
    expect(Token::Keyword::Return);
    std::shared_ptr<AST::Return> result = AST::Node::create<AST::Return>(_tk);

    result->tuple = parseTupleExpression(result->isSeq);
    return result;
}

std::shared_ptr<AST::Continue> Parser::parseContinue(void)
{
    expect(Token::Keyword::Continue);
    return AST::Node::create<AST::Continue>(_tk);
}

/** Expression Components **/

std::shared_ptr<AST::Name> Parser::parseName(void)
{
    std::shared_ptr<AST::Name> result = AST::Node::create<AST::Name>(_tk);
    result->name = _tk->next()->asIdentifier();
    return result;
}

std::shared_ptr<AST::Index> Parser::parseIndex(void)
{
    expect(Token::Operator::IndexLeft);
    std::shared_ptr<AST::Index> result = AST::Node::create<AST::Index>(_tk);
    result->index = parseExpression();
    expect(Token::Operator::IndexRight);
    return result;
}

std::shared_ptr<AST::Invoke> Parser::parseInvoke(void)
{
    expect(Token::Operator::BracketLeft);
    std::shared_ptr<AST::Invoke> result = AST::Node::create<AST::Invoke>(_tk);

    if (!isOperator(Token::Operator::BracketRight))
    {
        do result->args.push_back(parseExpression());
        while (skipOperator(Token::Operator::Comma));
    }

    expect(Token::Operator::BracketRight);
    return result;
}

std::shared_ptr<AST::Attribute> Parser::parseAttribute(void)
{
    expect(Token::Operator::Point);
    std::shared_ptr<AST::Attribute> result = AST::Node::create<AST::Attribute>(_tk);
    result->attribute = parseName();
    return result;
}

/** Expressions **/

std::shared_ptr<AST::Map> Parser::parseMap(void)
{
    /* the "{" operator is already skipped */
    std::shared_ptr<AST::Map> result = AST::Node::create<AST::Map>(_tk);

    while (!isOperator(Token::Operator::BlockRight))
    {
        std::shared_ptr<AST::Name> name;
        std::shared_ptr<AST::Expression> item = parseExpression();

        if (!unpackPointerPair(item, name))
        {
            /* standard type item */
            expect(Token::Operator::Colon);
            result->items.push_back(std::make_pair(std::move(item), parseExpression()));
        }
        else
        {
            /* simple pointer-pair item */
            std::shared_ptr<AST::Constant  > val  = AST::Node::create<AST::Constant  >(_tk);
            std::shared_ptr<AST::Component > comp = AST::Node::create<AST::Component >(_tk);

            /* build string constant */
            val->type = AST::Constant::Type::ConstantString;
            val->stringValue = std::move(name->name);

            /* build component node out of constant */
            comp->type = AST::Component::Type::ComponentConstant;
            comp->constant = std::move(val);

            /* wrap component node with expression and add to map items list */
            result->items.push_back(std::make_pair(AST::Node::create<AST::Expression>(_tk, std::move(comp)), std::move(item)));
        }

        /* single comma at the end of map is supported */
        if (!skipOperator(Token::Operator::Comma))
            if (!isOperator(Token::Operator::BlockRight))
                throw Exception::SyntaxError(_tk->row(), _tk->col(), "Operator \",\" expected");
    }

    expect(Token::Operator::BlockRight);
    return result;
}

std::shared_ptr<AST::List> Parser::parseList(void)
{
    /* the "[" operator is already skipped */
    std::shared_ptr<AST::List> result = AST::Node::create<AST::List>(_tk);

    while (!isOperator(Token::Operator::IndexRight))
    {
        /* parse single element */
        result->items.push_back(parseExpression());

        /* single comma at the end of list is supported */
        if (!skipOperator(Token::Operator::Comma))
            if (!isOperator(Token::Operator::IndexRight))
                throw Exception::SyntaxError(_tk->row(), _tk->col(), "Operator \",\" expected");
    }

    expect(Token::Operator::IndexRight);
    return result;
}

std::shared_ptr<AST::Unit> Parser::parseUnit(void)
{
    std::shared_ptr<Token> token = _tk->next();
    std::shared_ptr<AST::Unit> result = AST::Node::create<AST::Unit>(_tk);

    switch (token->asOperator())
    {
        case Token::Operator::BlockLeft:
        {
            /* map literal */
            result->map = parseMap();
            result->type = AST::Unit::Type::UnitMap;
            break;
        }

        case Token::Operator::IndexLeft:
        {
            /* list literal */
            result->list = parseList();
            result->type = AST::Unit::Type::UnitList;
            break;
        }

        case Token::Operator::BracketLeft:
        {
            /* empty tuple literal or maybe lambda expression with no arguments */
            if (skipOperator(Token::Operator::BracketRight))
            {
                if (!skipOperator(Token::Operator::Pointer))
                {
                    /* empty tuple literal */
                    result->type = AST::Unit::Type::UnitTuple;
                    result->tuple = AST::Node::create<AST::Tuple>(_tk);
                }
                else
                {
                    /* lambda expression with no arguments */
                    result->type = AST::Unit::Type::UnitLambda;
                    result->lambda = AST::Node::create<AST::Define>(_tk);
                    result->lambda->name = nullptr;
                    result->lambda->body = parseStatement();
                }
            }
            else
            {
                /* first argument, first element, or maybe nested expression, they looks like the same at this point */
                std::shared_ptr<AST::Name> name;
                std::shared_ptr<AST::Expression> item = parseExpression();

                if (skipOperator(Token::Operator::BracketRight))
                {
                    /* it's a lambda iff it follows with pointer operator and the first expression is a valid arg name */
                    if (!skipOperator(Token::Operator::Pointer) || !extractArgumentName(item, name))
                    {
                        result->type = AST::Unit::Type::UnitExpression;
                        result->expression = item;
                    }
                    else
                    {
                        result->type = AST::Unit::Type::UnitLambda;
                        result->lambda = AST::Node::create<AST::Define>(_tk);
                        result->lambda->name = nullptr;
                        result->lambda->body = parseStatement();
                        result->lambda->args.push_back(name);
                    }
                }
                else
                {
                    /* tuple literals, or maybe lambda expression */
                    bool maybeLambda = true;
                    std::vector<std::shared_ptr<AST::Expression>> items({ std::move(item) });

                    while (skipOperator(Token::Operator::Comma))
                    {
                        /* found a single comma, that must be a tuple literal */
                        if (isOperator(Token::Operator::BracketRight))
                        {
                            maybeLambda = false;
                            break;
                        }

                        /* next item */
                        items.push_back(parseExpression());
                    }

                    /* must ends with right bracket */
                    expect(Token::Operator::BracketRight);

                    /* check for lambda possibility */
                    if (maybeLambda)
                    {
                        bool isLambda = true;
                        std::shared_ptr<AST::Define> define = AST::Node::create<AST::Define>(_tk);

                        for (const auto &arg : items)
                        {
                            /* convert to arg name */
                            if (!extractArgumentName(arg, name))
                            {
                                isLambda = false;
                                break;
                            }

                            /* add to argument list */
                            define->args.push_back(name);
                        }

                        if (isLambda)
                        {
                            define->name = nullptr;
                            define->body = parseStatement();
                            result->type = AST::Unit::Type::UnitLambda;
                            result->lambda = std::move(define);
                            break;
                        }
                    }

                    /* it's definately a tuple literal */
                    result->type = AST::Unit::Type::UnitTuple;
                    result->tuple = AST::Node::create<AST::Tuple>(_tk);
                    result->tuple->items = std::move(items);
                }
            }

            break;
        }

        default:
            throw Exception::SyntaxError(_tk->row(), _tk->col(), "Unexpected token " + token->toString());
    }

    return result;
}

std::shared_ptr<AST::Constant> Parser::parseConstant(void)
{
    std::shared_ptr<Token> token = _tk->next();
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
    std::shared_ptr<Token> token = _tk->peek();
    std::shared_ptr<AST::Component> result = AST::Node::create<AST::Component>(_tk);

    switch (token->type())
    {
        case Token::Type::Eof:
        case Token::Type::Keywords:
            throw Exception::SyntaxError(_tk->row(), _tk->col(), "Unexpected token " + token->toString());

        case Token::Type::Float:
        case Token::Type::String:
        case Token::Type::Integer:
        {
            result->type = AST::Component::Type::ComponentConstant;
            result->constant = parseConstant();
            break;
        }

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

            if (skipOperator(Token::Operator::Pointer))
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
                /* add an attribute modifier */
                result->modifiers.push_back(parseAttribute());
                break;
            }

            /* index-like access */
            case Token::Operator::IndexLeft:
            {
                /* add an index modifier */
                result->modifiers.push_back(parseIndex());
                break;
            }

            /* object invoking */
            case Token::Operator::BracketLeft:
            {
                /* add an invoke modifier */
                result->modifiers.push_back(parseInvoke());
                break;
            }

            /* new-line encountered */
            case Token::Operator::NewLine:
            {
                /* save tokenizer state */
                _tk->pushState();

                /* skip all remaining new-lines */
                while (token->is<Token::Type::Operators>() &&
                      (token->asOperator() == Token::Operator::NewLine))
                {
                    _tk->nextOrLine();
                    token = _tk->peekOrLine();
                }

                /* check for eof */
                if (token->is<Token::Type::Eof>())
                {
                    _tk->popState();
                    return result;
                }

                /* only attribute access can wrap to next line */
                if (!token->is<Token::Type::Operators>() ||
                    (token->asOperator() != Token::Operator::Point))
                {
                    _tk->popState();
                    return result;
                }

                /* add an attribute modifier */
                _tk->killState();
                result->modifiers.push_back(parseAttribute());
                break;
            }

            /* other operators encountered, done parsing components */
            default:
                return result;
        }
    }

    return result;
}

std::shared_ptr<AST::Expression> Parser::parsePower(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseComponent());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::Power }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseComponent())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseUnary(void)
{
    /* the unary operator */
    Token::Operator op;

    /* recursively parsing unary operators */
    if (!readOperators(op, { Token::Operator::Plus, Token::Operator::Minus, Token::Operator::BitNot }))
        return parsePower();
    else
        return AST::Node::create<AST::Expression>(_tk, op, parseUnary());
}

std::shared_ptr<AST::Expression> Parser::parseFactor(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseUnary());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::Multiply, Token::Operator::Divide, Token::Operator::Module }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseUnary())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseTerm(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseFactor());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::Plus, Token::Operator::Minus }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseFactor())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBitShift(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseTerm());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::ShiftLeft, Token::Operator::ShiftRight }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseTerm())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBitAnd(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBitShift());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::BitAnd }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBitShift())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBitXor(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBitAnd());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::BitXor }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBitAnd())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBitOr(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBitXor());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::BitOr }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBitXor())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseRelations(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBitXor());

    /* operator chaining */
    for (;;)
    {
        if (skipOperator(Token::Operator::BoolNot))
        {
            expect(Token::Operator::In);
            result->remains.push_back(std::make_pair(Token::Operator::NotIn, AST::Node::create<AST::Expression>(_tk, parseBitXor())));
        }
        else
        {
            if (!readOperators(op, {
                Token::Operator::Is,
                Token::Operator::In,
                Token::Operator::Leq,
                Token::Operator::Geq,
                Token::Operator::Neq,
                Token::Operator::Equ,
                Token::Operator::Less,
                Token::Operator::Greater }))
                break;

            if ((op != Token::Operator::Is) || !skipOperator(Token::Operator::BoolNot))
                result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBitXor())));
            else
                result->remains.push_back(std::make_pair(Token::Operator::IsNot, AST::Node::create<AST::Expression>(_tk, parseBitXor())));
        }
    }

    /* relation operators need to be treated seperately, so mark here */
    result->isRelations = true;
    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBoolNot(void)
{
    /* recursively parsing `BoolNot` operator */
    if (!skipOperator(Token::Operator::BoolNot))
        return parseRelations();
    else
        return AST::Node::create<AST::Expression>(_tk, Token::Operator::BoolNot, parseBoolNot());
}

std::shared_ptr<AST::Expression> Parser::parseBoolAnd(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBoolNot());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::BoolAnd }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBoolNot())));

    return result;
}

std::shared_ptr<AST::Expression> Parser::parseBoolOr(void)
{
    /* build result expression */
    Token::Operator op;
    std::shared_ptr<AST::Expression> result = AST::Node::create<AST::Expression>(_tk, parseBoolAnd());

    /* operator chaining */
    while (readOperators(op, { Token::Operator::BoolOr }))
        result->remains.push_back(std::make_pair(op, AST::Node::create<AST::Expression>(_tk, parseBoolAnd())));

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
