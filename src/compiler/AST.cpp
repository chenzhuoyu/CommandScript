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
    std::string result = Strings::repeat("| ", level) + ((name == nullptr)
        ? "Define Lambda\n"
        : Strings::format("Define Function %s\n", name->name));

    result += Strings::repeat("| ", level + 1);
    result += Strings::format("Args %d\n", args.size());

    for (const auto &arg : args)
        result += arg->toString(level + 2);

    result += Strings::repeat("| ", level + 1);
    result += "Body\n";
    result += body->toString(level + 2);
    return result;
}

/** Statements **/

std::string Statement::toString(size_t level) const
{
    return "";
}

/** Control Flows **/

/** Expression Components **/

std::string Name::toString(size_t level) const
{
    return Strings::repeat("| ", level)
         + Strings::format("Name %s\n", name);
}

std::string Index::toString(size_t level) const
{
    return Strings::repeat("| ", level) + "Index\n"
         + index->toString(level + 1);
}

std::string Invoke::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level) + Strings::format("Invoke %d\n", args.size());
    std::for_each(args.begin(), args.end(), [&](const auto &x){ result += x->toString(level + 1); });
    return result;
}

std::string Attribute::toString(size_t level) const
{
    return Strings::repeat("| ", level) + "Attribute\n"
         + attribute->toString(level + 1);
}

/** Expressions **/

std::string Map::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level) + Strings::format("Map %d\n", items.size());

    for (const auto &item : items)
    {
        result += Strings::repeat("| ", level + 1);
        result += "Key\n";
        result += item.first->toString(level + 2);
        result += Strings::repeat("| ", level + 1);
        result += "Value\n";
        result += item.second->toString(level + 2);
    }

    return result;
}

std::string List::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level) + Strings::format("List %d\n", items.size());
    std::for_each(items.begin(), items.end(), [&](const auto &x){ result += x->toString(level + 1); });
    return result;
}

std::string Tuple::toString(size_t level) const
{
    std::string result = Strings::repeat("| ", level) + Strings::format("Tuple %d\n", items.size());
    std::for_each(items.begin(), items.end(), [&](const auto &x){ result += x->toString(level + 1); });
    return result;
}

std::string Unit::toString(size_t level) const
{
    switch (type)
    {
        case Type::UnitUnit:
        {
            return Strings::repeat("| ", level)
                 + Strings::format("Unit %s\n", Token::operatorName(op))
                 + unit->toString(level + 1);
        }

        case Type::UnitComponent    : return Strings::repeat("| ", level) + "Component\n" + component->toString(level + 1);
        case Type::UnitExpression   : return Strings::repeat("| ", level) + "Expression\n" + expression->toString(level + 1);

        case Type::UnitMap          : return Strings::repeat("| ", level) + "Map\n" + map->toString(level + 1);
        case Type::UnitList         : return Strings::repeat("| ", level) + "List\n" + list->toString(level + 1);
        case Type::UnitTuple        : return Strings::repeat("| ", level) + "Tuple\n" + tuple->toString(level + 1);
        case Type::UnitDefine       : return Strings::repeat("| ", level) + "Define\n" + define->toString(level + 1);
    }
}

std::string Pair::toString(size_t level) const
{
    return Strings::repeat("| ", level) + "Pair\n"
         + name->toString(level + 1)
         + value->toString(level + 1);
}

std::string Range::toString(size_t level) const
{
    if (upper == nullptr)
    {
        return Strings::repeat("| ", level) + "Point\n"
             + lower->toString(level + 1);
    }
    else
    {
        return Strings::repeat("| ", level) + "Range\n"
             + Strings::repeat("| ", level + 1) + "Lower\n"
             + lower->toString(level + 2)
             + Strings::repeat("| ", level + 1) + "Upper\n"
             + upper->toString(level + 2);
    }
}

std::string Constant::toString(size_t level) const
{
    switch (type)
    {
        case Type::ConstantFloat    : return Strings::repeat("| ", level) + Strings::format("Float %f\n", floatValue);
        case Type::ConstantString   : return Strings::repeat("| ", level) + Strings::format("String %s\n", Strings::repr(stringValue));
        case Type::ConstantInteger  : return Strings::repeat("| ", level) + Strings::format("Integer %ld\n", integerValue);
    }
}

std::string Component::toString(size_t level) const
{
    std::string result;

    switch (type)
    {
        case Type::ComponentName        : result += Strings::repeat("| ", level) + "Component\n" + name->toString(level + 1);       break;
        case Type::ComponentPair        : result += Strings::repeat("| ", level) + "Component\n" + pair->toString(level + 1);       break;
        case Type::ComponentUnit        : result += Strings::repeat("| ", level) + "Component\n" + unit->toString(level + 1);       break;
        case Type::ComponentConstant    : result += Strings::repeat("| ", level) + "Component\n" + constant->toString(level + 1);   break;
    }

    result += Strings::repeat("| ", level + 1);
    result += Strings::format("Modifiers %d\n", modifiers.size());

    for (const auto &mod : modifiers)
    {
        switch (mod.type)
        {
            case ModType::ModifierIndex     : result += mod.index->toString(level + 2); break;
            case ModType::ModifierInvoke    : result += mod.invoke->toString(level + 2); break;
            case ModType::ModifierAttribute : result += mod.attribute->toString(level + 2); break;
        }
    }

    return result;
}

std::string Expression::toString(size_t level) const
{
    std::string result = (right == nullptr)
        ? (Strings::repeat("| ", level) + "Expression\n")
        : (Strings::repeat("| ", level) + Strings::format("Expression \"%s\"\n", Token::operatorName(op)));

    switch (left->type)
    {
        case Type::ExpressionComponent:
        {
            result += Strings::repeat("| ", level + 1) + "Left Component\n" + left->component->toString(level + 2);
            break;
        }

        case Type::ExpressionExpression:
        {
            result += Strings::repeat("| ", level + 1) + "Left Expression\n" + left->expression->toString(level + 2);
            break;
        }
    }

    if (right != nullptr)
    {
        switch (right->type)
        {
            case Type::ExpressionComponent:
            {
                result += Strings::repeat("| ", level + 1) + "Right Component\n" + right->component->toString(level + 2);
                break;
            }

            case Type::ExpressionExpression:
            {
                result += Strings::repeat("| ", level + 1) + "Right Expression\n" + right->expression->toString(level + 2);
                break;
            }
        }
    }

    return result;
}
}
}
}
