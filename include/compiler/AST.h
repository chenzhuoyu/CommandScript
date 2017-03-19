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

struct Switch;
struct Case;
struct Default;

struct Try;
struct Except;
struct Finally;

struct Define;
struct ArgItem;
struct Argument;

/** Statements **/

struct Assign;
struct Delete;
struct Inplace;

struct Item;
struct Sequence;
struct Parallels;

struct Compond;
struct Statement;

/** Control Flows **/

struct Break;
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
struct Range;
struct Constant;
struct Component;
struct Expression;

/**** Parser Structure Definations ****/

/** Language Structures **/

struct If;
struct For;
struct While;

struct Switch;
struct Case;
struct Default;

struct Try;
struct Except;
struct Finally;

struct Define final : public Node
{
public:
    std::string toString(size_t level) const override;

};

struct ArgItem;
struct Argument;

/** Statements **/

struct Assign;
struct Delete;
struct Inplace;

struct Item;
struct Sequence;
struct Parallels;

struct Compond;
struct Statement;

/** Control Flows **/

struct Break;
struct Return;
struct Continue;

/** Expression Components **/

struct Name final : public Node
{
    std::string name;

public:
    std::string toString(size_t level) const override;

};

struct Index : public Node
{
    std::shared_ptr<Expression> index;

public:
    std::string toString(size_t level) const override;

};

struct Invoke : public Node
{
    std::vector<std::shared_ptr<Expression>> args;

public:
    std::string toString(size_t level) const override;

};

struct Attribute : public Node
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
        UnitNested,
        UnitComponent,
        UnitExpression,

        UnitMap,
        UnitList,
        UnitTuple,
        UnitDefine,
    };

public:
    Type type;
    Token::Operator op;

public:
    std::shared_ptr<Unit> nested;
    std::shared_ptr<Component> component;
    std::shared_ptr<Expression> expression;

public:
    std::shared_ptr<Map> map;
    std::shared_ptr<List> list;
    std::shared_ptr<Tuple> tuple;
    std::shared_ptr<Define> define;

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

struct Range final : public Node
{
    std::shared_ptr<Expression> lower;
    std::shared_ptr<Expression> upper;

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

        ComponentIndex,
        ComponentInvoke,
        ComponentAttribute,
    };

public:
    Type type;

public:
    std::shared_ptr<Name> name;
    std::shared_ptr<Pair> pair;
    std::shared_ptr<Unit> unit;
    std::shared_ptr<Constant> constant;

public:
    std::shared_ptr<Index> index;
    std::shared_ptr<Invoke> invoke;
    std::shared_ptr<Attribute> attribute;

public:
    std::string toString(size_t level) const override;

};

struct Expression final : public Node
{
    struct Side
    {
        bool isComponent = false;
        std::shared_ptr<Node> node = nullptr;
    };

public:
    struct Level
    {
        enum
        {
            Power,
            Factor,
            Term,
            Bitwise,
            Relation,
            Expr,

            Lowest  = Expr,
            Highest = Power,
        };
    };

public:
    Side left;
    Side right;
    Token::Operator op;

public:
    std::string toString(size_t level) const override;

};
}
}
}

#endif /* COMMANDSCRIPT_COMPILER_AST_H */
