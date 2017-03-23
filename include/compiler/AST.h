#ifndef COMMANDSCRIPT_COMPILER_AST_H
#define COMMANDSCRIPT_COMPILER_AST_H

#include <memory>
#include <vector>
#include <utility>

#include "Strings.h"
#include "Tokenizer.h"
#include "NonMovable.h"
#include "NonCopyable.h"

namespace CommandScript
{
namespace Compiler
{
namespace AST
{
struct Node : public NonMovable, public NonCopyable
{
    int row = -1;
    int col = -1;

public:
    virtual ~Node() {}
    explicit Node() {}

public:
    virtual std::string toString(size_t level = 0) const = 0;

private:
    template <typename NodeType>
    NodeType *bindTokenizer(const std::shared_ptr<Tokenizer> &tk)
    {
        row = tk->row();
        col = tk->col();
        return static_cast<NodeType *>(this);
    }

public:
    template <typename NodeType, typename ... Args>
    static std::shared_ptr<NodeType> create(const std::shared_ptr<Tokenizer> &tk, Args && ... args)
    {
        static_assert(std::is_convertible<NodeType *, Node *>::value, "`NodeType *` must be convertiable to `Node *`");
        return std::shared_ptr<NodeType>((new NodeType(std::forward<Args>(args) ...))->template bindTokenizer<NodeType>(tk));
    }
};

/**** Parser Structure Declarations ****/

/** Language Structures **/

struct If;
struct For;
struct While;
struct Define;
struct Import;

struct Try;
struct Except;
struct Finally;

/** Statements **/

struct Assign;
struct Delete;
struct Inplace;
struct Sequence;

struct Compond;
struct Statement;

/** Control Flows **/

struct Break;
struct Raise;
struct Return;
struct Continue;

/** Expression Components **/

struct Name;
struct Index;
struct Invoke;
struct Attribute;

/** Expressions **/

struct Map;
struct List;
struct Tuple;

struct Unit;
struct Pair;
struct Constant;
struct Component;
struct Expression;

/**** Parser Structure Definations ****/

/** Language Structures **/

struct If final : public Node
{
    std::shared_ptr<Expression> expr;

public:
    std::shared_ptr<Statement> positive;
    std::shared_ptr<Statement> negative;

public:
    std::string toString(size_t level) const override;

};

struct For final : public Node
{
    std::shared_ptr<Sequence> seq;
    std::shared_ptr<Statement> body;
    std::shared_ptr<Expression> expr;

public:
    std::string toString(size_t level) const override;

};

struct While final : public Node
{
    std::shared_ptr<Statement> body;
    std::shared_ptr<Expression> expr;

public:
    std::string toString(size_t level) const override;

};

struct Define final : public Node
{
    std::shared_ptr<Name> name;
    std::shared_ptr<Statement> body;

public:
    std::vector<std::shared_ptr<Name>> args;

public:
    std::string toString(size_t level) const override;

};

struct Import final : public Node
{
    std::vector<std::shared_ptr<Name>> names;

public:
    std::string toString(size_t level) const override;

};

struct Try;
struct Except;
struct Finally;

/** Statements **/

struct Assign final : public Node
{
    bool isSeq;
    std::shared_ptr<Tuple> tuple;
    std::shared_ptr<Sequence> target;

public:
    std::string toString(size_t level) const override;

};

struct Delete final : public Node
{
    std::shared_ptr<Component> target;

public:
    std::string toString(size_t level) const override;

};

struct Inplace final : public Node
{
    Token::Operator op;
    std::shared_ptr<Component> target;
    std::shared_ptr<Expression> expression;

public:
    std::string toString(size_t level) const override;

};

struct Sequence final : public Node
{
    enum class Type : int
    {
        SequenceSequence,
        SequenceComponent,
    };

public:
    struct Item
    {
        Type type;
        std::shared_ptr<Sequence > sequence;
        std::shared_ptr<Component> component;

    public:
        Item(const std::shared_ptr<Sequence > &value) : type(Type::SequenceSequence ), sequence (value) {}
        Item(const std::shared_ptr<Component> &value) : type(Type::SequenceComponent), component(value) {}

    };

public:
    bool isSeq;
    std::vector<Item> items;

public:
    std::string toString(size_t level) const override;

};

struct Compond final : public Node
{
    std::vector<std::shared_ptr<Statement>> statements;

public:
    std::string toString(size_t level) const override;

};

struct Statement final : public Node
{
    enum class Type : int
    {
        StatementIf,
        StatementFor,
/*      StatementTry,       */
        StatementWhile,
        StatementCompond,

        StatementDefine,
        StatementDelete,
        StatementImport,

        StatementBreak,
/*      StatementRaise,     */
        StatementReturn,
        StatementContinue,

        StatementAssign,
        StatementInplace,
        StatementComponent,
    };

public:
    Type type;

public:
    std::shared_ptr<If       > ifStatement;
    std::shared_ptr<For      > forStatement;
/*  std::shared_ptr<For      > tryStatement;     */
    std::shared_ptr<While    > whileStatement;
    std::shared_ptr<Compond  > compondStatement;

public:
    std::shared_ptr<Define   > defineStatement;
    std::shared_ptr<Delete   > deleteStatement;
    std::shared_ptr<Import   > importStatement;

public:
    std::shared_ptr<Break    > breakStatement;
/*  std::shared_ptr<Raise    > raiseStatement;   */
    std::shared_ptr<Return   > returnStatement;
    std::shared_ptr<Continue > continueStatement;

public:
    std::shared_ptr<Assign   > assignStatement;
    std::shared_ptr<Inplace  > inplaceStatement;
    std::shared_ptr<Component> componentStatement;

public:
    void setStatement(const std::shared_ptr<If      > &value) { type = Type::StatementIf      ; ifStatement       = value; }
    void setStatement(const std::shared_ptr<For     > &value) { type = Type::StatementFor     ; forStatement      = value; }
/*  void setStatement(const std::shared_ptr<Try     > &value) { type = Type::StatementTry     ; tryStatement      = value; }    */
    void setStatement(const std::shared_ptr<While   > &value) { type = Type::StatementWhile   ; whileStatement    = value; }

public:
    void setStatement(const std::shared_ptr<Define  > &value) { type = Type::StatementDefine  ; defineStatement   = value; }
    void setStatement(const std::shared_ptr<Delete  > &value) { type = Type::StatementDelete  ; deleteStatement   = value; }
    void setStatement(const std::shared_ptr<Import  > &value) { type = Type::StatementImport  ; importStatement   = value; }

public:
    void setStatement(const std::shared_ptr<Break   > &value) { type = Type::StatementBreak   ; breakStatement    = value; }
/*  void setStatement(const std::shared_ptr<Raise   > &value) { type = Type::StatementRaise   ; raiseStatement    = value; }    */
    void setStatement(const std::shared_ptr<Return  > &value) { type = Type::StatementReturn  ; returnStatement   = value; }
    void setStatement(const std::shared_ptr<Continue> &value) { type = Type::StatementContinue; continueStatement = value; }

public:
    std::string toString(size_t level) const override;

};

/** Control Flows **/

struct Break final : public Node
{
    std::string toString(size_t level) const override { return Strings::repeat("| ", level) + "Break\n"; }
};

struct Raise;

struct Return final : public Node
{
    bool isSeq;
    std::shared_ptr<Tuple> tuple;

public:
    std::string toString(size_t level) const override;

};

struct Continue final : public Node
{
    std::string toString(size_t level) const override { return Strings::repeat("| ", level) + "Continue\n"; }
};

/** Expression Components **/

struct Name final : public Node
{
    std::string name;

public:
    std::string toString(size_t level) const override;

};

struct Index final : public Node
{
    std::shared_ptr<Expression> index;

public:
    std::string toString(size_t level) const override;

};

struct Invoke final : public Node
{
    std::vector<std::shared_ptr<Expression>> args;

public:
    std::string toString(size_t level) const override;

};

struct Attribute final : public Node
{
    std::shared_ptr<Name> attribute;

public:
    std::string toString(size_t level) const override;

};

/** Expressions **/

struct Map final : public Node
{
    std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>> items;

public:
    std::string toString(size_t level) const override;

};

struct List final : public Node
{
    std::vector<std::shared_ptr<Expression>> items;

public:
    std::string toString(size_t level) const override;

};

struct Tuple final : public Node
{
    std::vector<std::shared_ptr<Expression>> items;

public:
    std::string toString(size_t level) const override;

};

struct Unit final : public Node
{
    enum class Type : int
    {
        UnitMap,
        UnitList,
        UnitTuple,
        UnitLambda,
        UnitExpression,
    };

public:
    Type type;

public:
    std::shared_ptr<Map       > map;
    std::shared_ptr<List      > list;
    std::shared_ptr<Tuple     > tuple;
    std::shared_ptr<Define    > lambda;
    std::shared_ptr<Expression> expression;

public:
    std::string toString(size_t level) const override;

};

struct Pair final : public Node
{
    std::shared_ptr<Name> name;
    std::shared_ptr<Expression> value;

public:
    std::string toString(size_t level) const override;

};

struct Constant final : public Node
{
    enum class Type : int
    {
        ConstantFloat,
        ConstantString,
        ConstantInteger,
    };

public:
    Type type;

public:
    double floatValue;
    int64_t integerValue;
    std::string stringValue;

public:
    std::string toString(size_t level) const override;

};

struct Component final : public Node
{
    enum class Type : int
    {
        ComponentName,
        ComponentPair,
        ComponentUnit,
        ComponentConstant,
    };

public:
    enum class ModType : int
    {
        ModifierIndex,
        ModifierInvoke,
        ModifierAttribute,
    };

public:
    struct Modifier
    {
        ModType type;
        std::shared_ptr<Index    > index;
        std::shared_ptr<Invoke   > invoke;
        std::shared_ptr<Attribute> attribute;

    public:
        Modifier(const std::shared_ptr<Index    > &value) : type(ModType::ModifierIndex    ), index    (value) {}
        Modifier(const std::shared_ptr<Invoke   > &value) : type(ModType::ModifierInvoke   ), invoke   (value) {}
        Modifier(const std::shared_ptr<Attribute> &value) : type(ModType::ModifierAttribute), attribute(value) {}

    };

public:
    Type type;
    bool isStandalone = false;

public:
    std::shared_ptr<Name    > name;
    std::shared_ptr<Pair    > pair;
    std::shared_ptr<Unit    > unit;
    std::shared_ptr<Constant> constant;

public:
    std::vector<Modifier> modifiers;

public:
    std::string toString(size_t level) const override;

};

struct Expression final : public Node
{
    enum class Type : int
    {
        TermComponent,
        TermExpression,
    };

public:
    struct Term
    {
        Type type;
        std::shared_ptr<Component > component;
        std::shared_ptr<Expression> expression;

    public:
        Term(const std::shared_ptr<Component > &value) : type(Type::TermComponent ), component (value) {}
        Term(const std::shared_ptr<Expression> &value) : type(Type::TermExpression), expression(value) {}

    };

public:
    Term first;
    Token::Operator op;
    std::vector<std::pair<Token::Operator, Term>> remains;

public:
    bool isUnary;
    bool isRelations;

public:
    explicit Expression(const std::shared_ptr<Component > &value) : first(value), isUnary(false), isRelations(false) {}
    explicit Expression(const std::shared_ptr<Expression> &value) : first(value), isUnary(false), isRelations(false) {}

public:
    explicit Expression(Token::Operator op, const std::shared_ptr<Component > &value) : first(value), op(op), isUnary(true), isRelations(false) {}
    explicit Expression(Token::Operator op, const std::shared_ptr<Expression> &value) : first(value), op(op), isUnary(true), isRelations(false) {}

public:
    std::string toString(size_t level) const override;

};
}
}
}

#endif /* COMMANDSCRIPT_COMPILER_AST_H */
