#pragma once

namespace Utils {
template <typename Container, typename Op>
inline void reverseForeach(const Container &c, const Op &operation)
{
    auto rend = c.rend();
    for (auto it = c.rbegin(); it != rend; ++it)
        operation(*it);
}
}
