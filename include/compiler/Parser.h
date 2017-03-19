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
    void expect(Token::Keyword keyword);
    bool skipOperators(const std::unordered_set<Token::Operator> &operators);
    bool readOperators(Token::Operator &op, const std::unordered_set<Token::Operator> &operators);

/** Expression Components **/
private:
    std::shared_ptr<AST::Name       > parseName(void);

/** Expressions **/
private:
    std::shared_ptr<AST::Unit       > parseUnit(void);
    std::shared_ptr<AST::Constant   > parseConstant(void);
    std::shared_ptr<AST::Component  > parseComponent(void);
    std::shared_ptr<AST::Expression > parseExpression(int level = AST::Expression::Level::Lowest);

/** parser wrapper method **/
public:
    std::shared_ptr<AST::Node> parse(void);

};
}
}

#endif /* COMMANDSCRIPT_COMPILER_PARSER_H */
