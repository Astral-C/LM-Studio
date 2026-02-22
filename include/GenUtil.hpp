#pragma once

#include <vector>
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <climits>
#include <glm/glm.hpp>

namespace GenUtility
{
    // Returns an endian-swapped version of the given value.
    // Copied from https://stackoverflow.com/a/4956493
    template <typename T>
    T SwapEndian(T u)
    {
        static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

        union
        {
            T u;
            unsigned char u8[sizeof(T)];
        } source, dest;

        source.u = u;

        for (size_t k = 0; k < sizeof(T); k++)
            dest.u8[k] = source.u8[sizeof(T) - k - 1];

        return dest.u;
    }
}
