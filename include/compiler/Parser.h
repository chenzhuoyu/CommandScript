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

private:
    size_t _breakable = 0;
    size_t _returnable = 0;
    size_t _continuable = 0;

public:
    virtual ~Parser() {}
    explicit Parser(const std::shared_ptr<Tokenizer> &tk) : _tk(tk) {}

private:
    void expect(Token::Keyword expect);
    void expect(Token::Operator expect);

private:
    bool isKeyword(Token::Keyword expected);
    bool skipKeyword(Token::Keyword expected);

private:
    bool isOperator(Token::Operator expected);
    bool skipOperator(Token::Operator expected);
    bool readOperators(Token::Operator &op, const std::unordered_set<Token::Operator> &operators);

private:
    bool unpackPointerPair(std::shared_ptr<AST::Expression> &expr, std::shared_ptr<AST::Name> &name);
    bool extractArgumentName(std::shared_ptr<AST::Expression> expr, std::shared_ptr<AST::Name> &name);

/** Language Structures **/
private:
    std::shared_ptr<AST::If         > parseIf               (void);
    std::shared_ptr<AST::For        > parseFor              (void);
/*  std::shared_ptr<AST::Try        > parseTry              (void); */
    std::shared_ptr<AST::While      > parseWhile            (void);
    std::shared_ptr<AST::Define     > parseDefine           (void);
    std::shared_ptr<AST::Import     > parseImport           (void);

/** Statements **/
private:
    std::shared_ptr<AST::Tuple      > parseTupleExpression  (bool &isSeq);
    std::shared_ptr<AST::Component  > parseMutableComponent (void);

public:
    std::shared_ptr<AST::Assign     > parseAssign           (bool &isRewindable);
    std::shared_ptr<AST::Inplace    > parseInplace          (bool &isRewindable);

private:
    std::shared_ptr<AST::Delete     > parseDelete           (void);
    std::shared_ptr<AST::Sequence   > parseSequence         (void);

private:
    std::shared_ptr<AST::Compond    > parseCompond          (void);
    std::shared_ptr<AST::Statement  > parseStatement        (void);

/** Control Flows **/
private:
    std::shared_ptr<AST::Break      > parseBreak            (void);
/*  std::shared_ptr<AST::Raise      > parseRaise            (void); */
    std::shared_ptr<AST::Return     > parseReturn           (void);
    std::shared_ptr<AST::Continue   > parseContinue         (void);

/** Expression Components **/
private:
    std::shared_ptr<AST::Name       > parseName             (void);
    std::shared_ptr<AST::Index      > parseIndex            (void);
    std::shared_ptr<AST::Invoke     > parseInvoke           (void);
    std::shared_ptr<AST::Attribute  > parseAttribute        (void);

/** Expressions **/
private:
    std::shared_ptr<AST::Map        > parseMap              (void);
    std::shared_ptr<AST::List       > parseList             (void);
    std::shared_ptr<AST::Unit       > parseUnit             (void);
    std::shared_ptr<AST::Constant   > parseConstant         (void);
    std::shared_ptr<AST::Component  > parseComponent        (void);
    std::shared_ptr<AST::Expression > parseExpression       (void) { return parseBoolOr(); }

/** Operator Precedence Parsers, from highest precedence (Power) to lowest precedence (BoolOr) **/
private:
    std::shared_ptr<AST::Expression > parsePower            (void);
    std::shared_ptr<AST::Expression > parseUnary            (void);
    std::shared_ptr<AST::Expression > parseFactor           (void);
    std::shared_ptr<AST::Expression > parseTerm             (void);
    std::shared_ptr<AST::Expression > parseBitShift         (void);
    std::shared_ptr<AST::Expression > parseBitAnd           (void);
    std::shared_ptr<AST::Expression > parseBitXor           (void);
    std::shared_ptr<AST::Expression > parseBitOr            (void);
    std::shared_ptr<AST::Expression > parseRelations        (void);
    std::shared_ptr<AST::Expression > parseBoolNot          (void);
    std::shared_ptr<AST::Expression > parseBoolAnd          (void);
    std::shared_ptr<AST::Expression > parseBoolOr           (void);

/** parser wrapper method **/
public:
    std::shared_ptr<AST::Node> parse(void);

};
}
}

#endif /* COMMANDSCRIPT_COMPILER_PARSER_H */
