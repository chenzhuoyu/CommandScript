#ifndef COMMANDSCRIPT_COMPILER_PARSER_H
#define COMMANDSCRIPT_COMPILER_PARSER_H

#include <memory>
#include <unordered_set>

#include "AST.h"
#include "Tokenizer.h"
#include "NonMovable.h"
#include "NonCopyable.h"

namespace CommandScript
{
namespace Compiler
{
class Parser : public NonCopyable
{
    std::shared_ptr<Tokenizer> _tk;

public:
    virtual ~Parser() {}
    explicit Parser(const std::shared_ptr<Tokenizer> &tk) : _tk(tk) {}

private:
    void expect(Token::Keyword expect);
    void expect(Token::Operator expect);

private:
    bool isOperator(Token::Operator expected);
    bool skipOperator(Token::Operator expected);
    bool readOperators(Token::Operator &op, const std::unordered_set<Token::Operator> &operators);

private:
    bool unpackPointerPair(std::shared_ptr<AST::Expression> &expr, std::shared_ptr<AST::Name> &name);
    bool extractArgumentName(std::shared_ptr<AST::Expression> expr, std::shared_ptr<AST::Name> &name);

/** Language Structures **/
private:

/** Statements **/
private:
    std::shared_ptr<AST::Statement  > parseStatement(void);

/** Control Flows **/
private:

/** Expression Components **/
private:
    std::shared_ptr<AST::Name       > parseName(void);
    std::shared_ptr<AST::Index      > parseIndex(void);
    std::shared_ptr<AST::Invoke     > parseInvoke(void);
    std::shared_ptr<AST::Attribute  > parseAttribute(void);

/** Expressions **/
private:
    std::shared_ptr<AST::Map        > parseMap(void);
    std::shared_ptr<AST::List       > parseList(void);

private:
    std::shared_ptr<AST::Unit       > parseUnit(void);
    std::shared_ptr<AST::Constant   > parseConstant(void);
    std::shared_ptr<AST::Component  > parseComponent(void);
    std::shared_ptr<AST::Expression > parseExpression(int priority = AST::Expression::Priority::Lowest);

/** parser wrapper method **/
public:
    std::shared_ptr<AST::Node> parse(void);

};
}
}

#endif /* COMMANDSCRIPT_COMPILER_PARSER_H */
