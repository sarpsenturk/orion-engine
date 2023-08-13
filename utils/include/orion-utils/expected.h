#pragma once

#include <tl/expected.hpp>

namespace orion
{
    template<typename T, typename E>
    using expected = tl::expected<T, E>;

    template<typename E>
    using unexpected = tl::unexpected<E>;

    template<typename E>
    using bad_expected_access = tl::bad_expected_access<E>;

    using tl::unexpect;
    using tl::unexpect_t;

    using tl::make_unexpected;
} // namespace orion
