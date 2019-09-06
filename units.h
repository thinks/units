#pragma once

#include <cstdint>
#include <ostream>
#include <ratio>
#include <type_traits>

namespace units {
namespace units_internal {

struct LengthTag;
struct AngleTag;

template <typename T>
struct is_ratio : public std::false_type {};
template <std::intmax_t Num, std::intmax_t Denom>
struct is_ratio<std::ratio<Num, Denom>> : public std::true_type {};
template <typename T>
constexpr bool is_ratio_v = is_ratio<T>::value;

template <typename T>
struct is_tag : public std::false_type {};
template <>
struct is_tag<LengthTag> : public std::true_type {};
template <>
struct is_tag<AngleTag> : public std::true_type {};
template <typename T>
constexpr bool is_tag_v = is_tag<T>::value;

template <typename FromScaleT, typename ToScaleT>
struct ScaleHelper {
  static_assert(is_ratio_v<FromScaleT>, "FromScaleT must be a ratio");
  static_assert(is_ratio_v<ToScaleT>, "ToScaleT must be a ratio");
  using ScaleDiv = typename std::ratio_divide<FromScaleT, ToScaleT>::type;

  // clang-format off
  template <typename ToArithT, typename FromArithT>
  constexpr static auto Scale(const FromArithT v) 
      noexcept(noexcept(static_cast<ToArithT>((ScaleDiv::num * v) / ScaleDiv::den))) 
      -> ToArithT {
    static_assert(std::is_arithmetic_v<FromArithT>, 
                  "FromArithT must be arithmetic");
    static_assert(std::is_arithmetic_v<ToArithT>,
                  "ToArithT must be arithmetic");
    return static_cast<ToArithT>((ScaleDiv::num * v) / ScaleDiv::den);
  }
  // clang-format on
};

template <typename ToArithT, typename FromArithT>
constexpr auto numeric_cast(const FromArithT v) noexcept -> ToArithT {
  // TODO(thinks): Implement range checking, similar to boost::numeric_cast.
  return static_cast<ToArithT>(v);
}

using MeterScale = std::ratio<10, 1>::type;
using CentimeterScale = std::ratio<1>::type; // Base.
using MillimeterScale = std::ratio<1, 10>::type;

using DegreeScale = std::ratio<1>::type; // Base
using RadianScale = std::ratio<18000000000000, 314159265359>::type; // Approx 180/pi.

}  // namespace units_internal

// Implement operators that would not change dimensionality, e.g.
// addition/subtraction, but not multiplication/division. We could
// support the latter, but it would be counter-intuitive.
//
// Perhaps the latter should be supported with the other operand being a pure
// scalar, such that it would not affect the dimensionality?
template <typename ArithT, typename ScaleT, typename TagT>
class Unit {
 public:
  using ValueType = ArithT;
  using ScaleType = ScaleT;
  using TagType = TagT;

  // ctor
  // NOTE(thinks):
  //   ctor not explicit here since copy-list-initialization cannot 
  //   use explicit constructors (C++14).
  // clang-format off
  /*explicit*/ constexpr Unit(ValueType&& v) 
    noexcept(noexcept(ValueType{std::move(v)}))
      : value_{std::move(v)} {}
  // clang-format on    

  constexpr ValueType value() const noexcept { return value_; }

  // TODO(thinks): Should we pass in rhs as a universal reference?
  //               Unit<ArithT2, ScaleT, TagType>&& rhs
  template<typename ArithT2>
  constexpr auto operator+=(const Unit<ArithT2, ScaleT, TagType> rhs) 
      // TODO(thinks): noexcept
      -> Unit<ArithT, ScaleT, TagType>& {
    value_ += scale_cast<Unit<ArithT, ScaleT, TagType>>(rhs).value();
    return *this;
  }

  template<typename ArithT2>
  constexpr auto operator-=(const Unit<ArithT2, ScaleT, TagType> rhs) 
      // TODO(thinks): noexcept
      -> Unit<ArithT, ScaleT, TagType>& {
    value_ -= scale_cast<Unit<ArithT, ScaleT, TagType>>(rhs).value();
    return *this;
  }

 private:
  ValueType value_;

  static_assert(std::is_arithmetic_v<ArithT>, "ArithT must be arithmetic");
  static_assert(units_internal::is_ratio_v<ScaleT>, "ScaleT must be a ratio");
  static_assert(units_internal::is_tag_v<TagT>, "TagT must be a tag");
};

// Convert between values with the same tag using the scale factors 
// and value type conversion.
// clang-format off
template <typename ToUnitT, 
          typename FromArithT, typename FromScaleT, typename TagT>
constexpr auto scale_cast(const Unit<FromArithT, FromScaleT, TagT> from) 
    noexcept(noexcept(
      units_internal::ScaleHelper<FromScaleT, typename ToUnitT::ScaleType>::
        Scale<typename ToUnitT::ValueType>(from.value())))
    -> Unit<typename ToUnitT::ValueType, typename ToUnitT::ScaleType, TagT> {
  static_assert(std::is_same_v<typename ToUnitT::TagType, TagT>,
                "units must have same tag");
  return {units_internal::ScaleHelper<FromScaleT, typename ToUnitT::ScaleType>::
              Scale<typename ToUnitT::ValueType>(from.value())};
}
// clang-format on


// Inequality comparison of same-tag-units with different value
// type and/or scale. Normal comparison rules for arithmetic types apply.
// clang-format off
template <typename ArithT, typename ArithT2, 
          typename ScaleT, typename ScaleT2,
          typename TagT>
constexpr auto operator==(const Unit<ArithT, ScaleT, TagT> lhs,
                          const Unit<ArithT2, ScaleT2, TagT> rhs) 
    noexcept(noexcept(scale_cast<Unit<ArithT, ScaleT, TagT>>(rhs)))
    -> bool {
  return lhs.value() == scale_cast<Unit<ArithT, ScaleT, TagT>>(rhs).value();
}
// clang-format on

// Inequality comparison of same-tag-units with different value
// type and/or scale. Normal comparison rules for arithmetic types apply.
// clang-format off
template <typename ArithT, typename ArithT2, 
          typename ScaleT, typename ScaleT2, 
          typename TagT>
constexpr auto operator!=(const Unit<ArithT, ScaleT, TagT> lhs,
                          const Unit<ArithT2, ScaleT2, TagT> rhs) 
    noexcept(noexcept(scale_cast<Unit<ArithT, ScaleT, TagT>>(rhs)))
    -> bool {
  return lhs.value() != scale_cast<Unit<ArithT, ScaleT, TagT>>(rhs).value();
}
// clang-format on

// Unary negation.
// clang-format off
template <typename ArithT, typename ScaleT, typename TagT>
constexpr auto operator-(const Unit<ArithT, ScaleT, TagT> u) 
    noexcept(noexcept(-u.value())) 
    -> Unit<decltype(-u.value()), ScaleT, TagT> {
  return {-u.value()};
}
// clang-format on

// Add units, supports different value types.
// clang-format off
template <typename ArithT, typename ArithT2, typename ScaleT, typename TagT>
constexpr auto operator+(const Unit<ArithT, ScaleT, TagT> lhs,
                         const Unit<ArithT2, ScaleT, TagT> rhs) 
    noexcept(noexcept(lhs.value() + rhs.value()))
    -> Unit<decltype(lhs.value() + rhs.value()), ScaleT, TagT> {
  return {lhs.value() + rhs.value()};
}
// clang-format on

// Subtract units, supports different value types.
// clang-format off
template <typename ArithT, typename ArithT2, typename ScaleT, typename TagT>
constexpr auto operator-(const Unit<ArithT, ScaleT, TagT> lhs,
                         const Unit<ArithT2, ScaleT, TagT> rhs) 
    noexcept(noexcept(lhs.value() - rhs.value()))
    -> Unit<decltype(lhs.value() - rhs.value()), ScaleT, TagT> {
  return {lhs.value() - rhs.value()};
}
// clang-format on

// clang-format off
template <typename ArithT> using Meters = Unit<ArithT, units_internal::MeterScale, units_internal::LengthTag>; 
template <typename ArithT> using Centimeters = Unit<ArithT, units_internal::CentimeterScale, units_internal::LengthTag>; 
template <typename ArithT> using Millimeters = Unit<ArithT, units_internal::MillimeterScale, units_internal::LengthTag>;

template <typename ArithT> using Degrees = Unit<ArithT, units_internal::DegreeScale, units_internal::AngleTag>; 
template <typename ArithT> using Radians = Unit<ArithT, units_internal::RadianScale, units_internal::AngleTag>; 
// clang-format on

// Literals.
constexpr auto operator"" _m(unsigned long long v) 
    noexcept // ??
    -> Meters<long long> {
  return {units_internal::numeric_cast<long long>(v)};
}
constexpr auto operator"" _m(long double v) 
    noexcept // ??
    -> Meters<double> {
  return {units_internal::numeric_cast<double>(v)};
}

constexpr auto operator"" _cm(unsigned long long v) 
    noexcept // ??
    -> Centimeters<long long> {
  return {units_internal::numeric_cast<long long>(v)};
}
constexpr auto operator"" _cm(long double v) 
    noexcept // ??
    -> Centimeters<double> {
  return {units_internal::numeric_cast<double>(v)};
}

constexpr auto operator"" _mm(unsigned long long v) 
    noexcept // ??
    -> Millimeters<long long> {
  return {units_internal::numeric_cast<long long>(v)};
}
constexpr auto operator"" _mm(long double v) 
    noexcept // ??
    -> Millimeters<double> {
  return {units_internal::numeric_cast<double>(v)};
}

constexpr auto operator"" _deg(unsigned long long v) 
    noexcept // ??
    -> Degrees<long long> {
  return {units_internal::numeric_cast<long long>(v)};
}
constexpr auto operator"" _deg(long double v) 
    noexcept // ??
    -> Degrees<double> {
  return {units_internal::numeric_cast<double>(v)};
}

constexpr auto operator"" _rad(unsigned long long v) 
    noexcept // ??
    -> Radians<long long> {
  return {units_internal::numeric_cast<long long>(v)};
}
constexpr auto operator"" _rad(long double v) 
    noexcept // ??
    -> Radians<double> {
  return {units_internal::numeric_cast<double>(v)};
}

template <typename ScaleT, typename TagT>
struct TagSuffix;
template <>
struct TagSuffix<units_internal::MeterScale, units_internal::LengthTag> {
  static constexpr char* value() noexcept { return "[m]"; }
};
template <>
struct TagSuffix<units_internal::CentimeterScale, units_internal::LengthTag> {
  static constexpr char* value() noexcept { return "[cm]"; }
};
template <>
struct TagSuffix<units_internal::MillimeterScale, units_internal::LengthTag> {
  static constexpr char* value() noexcept { return "[mm]"; }
};
template <>
struct TagSuffix<units_internal::DegreeScale, units_internal::AngleTag> {
  static constexpr char* value() noexcept { return "[deg]"; }
};
template <>
struct TagSuffix<units_internal::RadianScale, units_internal::AngleTag> {
  static constexpr char* value() noexcept { return "[rad]"; }
};

template <typename ArithT, typename ScaleT, typename TagT>
std::ostream& operator<<(std::ostream& os,
                         const Unit<ArithT, ScaleT, TagT>& rhs) {
  os << rhs.value() << " " << TagSuffix<ScaleT, TagT>::value();
  return os;
}

}  // namespace units