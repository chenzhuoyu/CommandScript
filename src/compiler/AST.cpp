#include "AST.h"

namespace CommandScript
{
namespace Compiler
{
namespace AST
{
/** Language Structures **/

/** Statements **/

/** Control Flows **/

/** Expression Components **/

std::string Name::toString(size_t level) const
{
    return Strings::repeat("| ", level + 1)
         + Strings::format("Name %s\n", name);
}

std::string Index::toString(size_t level) const
{
    return "";
}

std::string Invoke::toString(size_t level) const
{
    return "";
}

std::string Attribute::toString(size_t level) const
{
    return "";
}

/** Expressions **/

std::string Unit::toString(size_t level) const
{
    return "";
}

std::string Pair::toString(size_t level) const
{
    return Strings::repeat("| ", level + 1) + "Pair\n"
         + name->toString(level + 1)
         + value->toString(level + 1);
}

std::string Range::toString(size_t level) const
{
    if (upper == nullptr)
    {
        return Strings::repeat("| ", level + 1) + "Point\n"
             + lower->toString(level + 2);
    }
    else
    {
        return Strings::repeat("| ", level + 1) + "Range\n"
             + Strings::repeat("| ", level + 2) + "Lower\n"
             + lower->toString(level + 3)
             + Strings::repeat("| ", level + 2) + "Upper\n"
             + upper->toString(level + 3);
    }
}

std::string Constant::toString(size_t level) const
{
    switch (type)
    {
        case Type::ConstantFloat    : return Strings::repeat("| ", level + 1) + Strings::format("Float %f\n", floatValue);
        case Type::ConstantString   : return Strings::repeat("| ", level + 1) + Strings::format("String %s\n", Strings::repr(stringValue));
        case Type::ConstantInteger  : return Strings::repeat("| ", level + 1) + Strings::format("Integer %ld\n", integerValue);
    }
}

std::string Component::toString(size_t level) const
{
    switch (type)
    {
        case Type::ComponentName        : return Strings::repeat("| ", level + 1) + "Component\n" + name->toString(level + 2);
        case Type::ComponentPair        : return Strings::repeat("| ", level + 1) + "Component\n" + pair->toString(level + 2);
        case Type::ComponentUnit        : return Strings::repeat("| ", level + 1) + "Component\n" + unit->toString(level + 2);
        case Type::ComponentConstant    : return Strings::repeat("| ", level + 1) + "Component\n" + constant->toString(level + 2);

        case Type::ComponentIndex       : return Strings::repeat("| ", level + 1) + "Component\n" + index->toString(level + 2);
        case Type::ComponentInvoke      : return Strings::repeat("| ", level + 1) + "Component\n" + invoke->toString(level + 2);
        case Type::ComponentAttribute   : return Strings::repeat("| ", level + 1) + "Component\n" + attribute->toString(level + 2);
    }
}

std::string Expression::toString(size_t level) const
{
    if (right.node == nullptr)
    {
        return Strings::repeat("| ", level + 1) + "Expression\n"
             + Strings::repeat("| ", level + 2) + Strings::format("Left %s\n", left.isComponent ? "Component" : "Expression")
             + left.node->toString(level + 3);
    }
    else
    {
        return Strings::repeat("| ", level + 1) + "Expression\n"
             + Strings::repeat("| ", level + 2) + Strings::format("Left %s\n", left.isComponent ? "Component" : "Expression")
             + left.node->toString(level + 3)
             + Strings::repeat("| ", level + 2) + Strings::format("Right %s\n", right.isComponent ? "Component" : "Expression")
             + right.node->toString(level + 3);
    }
}
}
}
}
