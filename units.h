#pragma once

namespace units {

template <typename T>
struct Length {
  T value;
};

constexpr Length<long double> operator"" _km(long double d) {
  return Length<long double>{1000 * d};
}



//struct cm;
//struct mm;

template <typename T, typename S>
struct unit {
  T value;
};

// clang-format off
template <typename T> using Centimeter = unit<T, struct cm>;
template <typename T> using Millimeter = unit<T, struct mm>;

template <typename T> using Degrees = unit<T, struct deg>;
template <typename T> using Radians = unit<T, struct rad>;
// clang-format on

template <typename T, typename T2, typename S>
constexpr auto operator+(const unit<T, S>& lhs, const unit<T2, S>& rhs) noexcept
    -> unit<decltype(lhs.value + rhs.value), S> {
  return {lhs.value + rhs.value};
}

// Literals
constexpr auto operator""_cm(unsigned long long v) -> Centimeter<long long> { return {static_cast<long long>(v)}; }
constexpr auto operator""_cm(long double v)        -> Centimeter<double> { return {static_cast<double>(v)}; }

constexpr auto operator""_mm(unsigned long long v) -> Millimeter<long long> { return {static_cast<long long>(v)}; }
constexpr auto operator""_mm(long double v)        -> Millimeter<double> { return {static_cast<double>(v)}; }

constexpr auto operator""_deg(unsigned long long v) -> Degrees<long long> { return {static_cast<long long>(v)}; }
constexpr auto operator""_deg(long double v)        -> Degrees<double> { return {static_cast<double>(v)}; }

constexpr auto operator""_rad(unsigned long long v) -> Radians<long long> { return {static_cast<long long>(v)}; }
constexpr auto operator""_rad(long double v)        -> Radians<double> { return {static_cast<double>(v)}; }


}  // namespace units