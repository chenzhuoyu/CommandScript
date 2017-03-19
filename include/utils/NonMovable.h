#ifndef NONMOVABLE_H
#define NONMOVABLE_H

struct NonMovable
{
    NonMovable() = default;
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

#endif /* NONMOVABLE_H */
