#ifndef STRINGS_H
#define STRINGS_H

#include <string>
#include <fmt/format.h>
#include <fmt/printf.h>

namespace Strings
{
std::string repr(const void *data, size_t size);
std::string hexdump(const void *data, size_t size);

static inline std::string repr(const std::string &str) { return repr(str.data(), str.size()); }
static inline std::string hexdump(const std::string &str) { return hexdump(str.data(), str.size()); }

static inline std::string repeat(const std::string &str, size_t count)
{
    std::string result;
    result.reserve(count * str.size());
    while (count--) result += str;
    return result;
}

template <typename ... Args>
static inline std::string format(const fmt::CStringRef &fmt, const Args & ... args)
{
    typedef fmt::internal::ArgArray<sizeof ... (Args)> ArgArray;
    typename ArgArray::Type array { ArgArray::template make<fmt::BasicFormatter<char>>(args) ... };

    fmt::MemoryWriter mw;
    fmt::printf(mw, fmt, fmt::ArgList(fmt::internal::make_type(args ...), array));
    return mw.str();
}
}

#endif /* STRINGS_H */
