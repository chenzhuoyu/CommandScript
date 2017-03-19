#include "AST.h"

namespace CommandScript
{
namespace Compiler
{
namespace AST
{
/** Language Structures **/

std::string Define::toString(size_t level) const
{
    return "";
}

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
    return Strings::repeat("| ", level + 1) + "Index\n"
         + index->toString(level + 2);
}

std::string Invoke::toString(size_t level) const
{
    return "";
}

std::string Attribute::toString(size_t level) const
{
    return Strings::repeat("| ", level + 1) + "Attribute\n"
         + attribute->toString(level + 2);
}

/** Expressions **/

std::string Map::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level + 1) + Strings::format("Map %d\n", items.size());

    for (const auto &item : items)
    {
        result += Strings::repeat("| ", level + 2);
        result += "Key\n";
        result += item.first->toString(level + 3);
        result += Strings::repeat("| ", level + 2);
        result += "Value\n";
        result += item.second->toString(level + 3);
    }

    return result;
}

std::string List::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level + 1) + Strings::format("List %d\n", items.size());
    std::for_each(items.begin(), items.end(), [&](const auto &x){ result += x->toString(level + 2); });
    return result;
}

std::string Tuple::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level + 1) + Strings::format("Tuple %d\n", items.size());
    std::for_each(items.begin(), items.end(), [&](const auto &x){ result += x->toString(level + 2); });
    return result;
}

std::string Unit::toString(size_t level) const
{
    switch (type)
    {
        case Type::UnitNested       : return Strings::repeat("| ", level + 1) + Strings::format("Nested %s\n", Token::operatorName(op)) + nested->toString(level + 2);
        case Type::UnitComponent    : return Strings::repeat("| ", level + 1) + "Component\n" + component->toString(level + 2);
        case Type::UnitExpression   : return Strings::repeat("| ", level + 1) + "Expression\n" + expression->toString(level + 2);

        case Type::UnitMap          : return Strings::repeat("| ", level + 1) + "Map\n" + map->toString(level + 2);
        case Type::UnitList         : return Strings::repeat("| ", level + 1) + "List\n" + list->toString(level + 2);
        case Type::UnitTuple        : return Strings::repeat("| ", level + 1) + "Tuple\n" + tuple->toString(level + 2);
        case Type::UnitDefine       : return Strings::repeat("| ", level + 1) + "Define\n" + define->toString(level + 2);
    }
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
