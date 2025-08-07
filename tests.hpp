#pragma once

#ifdef _MSC_VER
    #pragma warning(disable : 4706)
#endif

// IWYU pragma: always_keep

#include <ostream>

#include <doctest/doctest.h>
#include <tcob/tcob.hpp>

using namespace tcob;
using namespace tcob::gfx;
using namespace tcob::ui;

namespace tcob {
template <Arithmetic T>
auto operator<<(std::ostream& os, point<T> const& m) -> std::ostream&
{
    return os << "x:" << m.X << "|y:" << m.Y;
}

template <Arithmetic T>
auto operator<<(std::ostream& os, size<T> const& m) -> std::ostream&
{
    return os << "width:" << m.Width << "|height:" << m.Height;
}

template <Arithmetic T>
auto operator<<(std::ostream& os, rect<T> const& m) -> std::ostream&
{
    return os << "left:" << m.left() << "|top:" << m.top() << "|width:" << m.width() << "|height:" << m.height();
}

template <FloatingPoint ValueType, f64 OneTurn>
auto operator<<(std::ostream& os, angle_unit<ValueType, OneTurn> const& m) -> std::ostream&
{
    return os << "value:" << m.Value;
}

auto inline operator<<(std::ostream& os, color const& m) -> std::ostream&
{
    return os << "r:" << static_cast<u32>(m.R) << "|g:" << static_cast<u32>(m.G) << "|b:" << static_cast<u32>(m.B) << "|a:" << static_cast<u32>(m.A);
}

}

using namespace std::literals::chrono_literals;
using namespace std::literals::string_literals;
using Approx = doctest::Approx;

void inline PrepareFile(std::string const& file)
{
    io::delete_file(file);
    REQUIRE_FALSE(io::exists(file));
    io::create_file(file);
    REQUIRE(io::exists(file));
}
