#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H

struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
};

#endif /* NONCOPYABLE_H */
