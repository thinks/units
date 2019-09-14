// Copyright(C) Tommy Hinks <tommy.hinks@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#pragma once

#include <cstdint>
#include <ostream>
#include <ratio>
#include <type_traits>

#if (__cplusplus >= 201703L)
  #define NO_DISCARD [[nodiscard]]
#else
  #define NO_DISCARD
#endif

namespace thinks {
namespace units_internal {

// Categories.
struct LengthTag;
struct AngleTag;
struct DoseTag;

// Define scale factors for lengths. 
// Using centimeters as unit length.
using MeterScale = std::ratio<100, 1>::type;
using CentimeterScale = std::ratio<1>::type; // Unit length.
using MillimeterScale = std::ratio<1, 10>::type;

// Define scale factors for angles.
// Using degrees as unit angle.
using DegreeScale = std::ratio<1>::type; // Unit angle.
using RadianScale = std::ratio<18000000000000, 314159265359>::type; // Approx 180/pi.

// Define scale factors for Gray, defined as the absorption of 
// one joule of radiation energy per kilogram of matter.
using GrayScale = std::ratio<1>::type; // Unit absorption.
using CentiGrayScale = std::ratio<1, 100>::type;

// std::ratio traits.
template <typename T>
struct is_ratio : public std::false_type {};
template <std::intmax_t Num, std::intmax_t Denom>
struct is_ratio<std::ratio<Num, Denom>> : public std::true_type {};
template <typename T>
constexpr bool is_ratio_v = is_ratio<T>::value;

// Tag (category) traits.
template <typename T>
struct is_tag : public std::false_type {};
template <>
struct is_tag<LengthTag> : public std::true_type {};
template <>
struct is_tag<AngleTag> : public std::true_type {};
template <>
struct is_tag<DoseTag> : public std::true_type {};
template <typename T>
constexpr bool is_tag_v = is_tag<T>::value;

// TODO(thinks): Implement range checking, similar to boost::numeric_cast.
template <typename ToArithT, typename FromArithT>
NO_DISCARD constexpr auto numeric_cast(
    const FromArithT v) /*noexcept*/ -> ToArithT {
  return static_cast<ToArithT>(v);
}

// Utility for applying scale factors and converting between 
// different value types.
template <typename FromScaleT, typename ToScaleT>
struct ScaleHelper {
  static_assert(is_ratio_v<FromScaleT>, "FromScaleT must be a ratio");
  static_assert(is_ratio_v<ToScaleT>, "ToScaleT must be a ratio");
  using ScaleDiv = typename std::ratio_divide<FromScaleT, ToScaleT>::type;

  // clang-format off
  template <typename ToArithT, typename FromArithT>
  NO_DISCARD constexpr static auto Scale(const FromArithT v) 
      //noexcept(noexcept(static_cast<ToArithT>((ScaleDiv::num * v) / ScaleDiv::den))) 
      -> ToArithT {
    static_assert(std::is_arithmetic_v<FromArithT>, 
                  "FromArithT must be arithmetic");
    static_assert(std::is_arithmetic_v<ToArithT>,
                  "ToArithT must be arithmetic");

    // Denominator is guaranteed to be non-zero.
    return numeric_cast<ToArithT>((ScaleDiv::num * v) / ScaleDiv::den);
  }
  // clang-format on
};

// Suffix string based on scale and category tag.
template <typename ScaleT, typename TagT>
struct TagSuffix;  // Generic, not implementd.
template <>
struct TagSuffix<units_internal::MeterScale, units_internal::LengthTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "m"; }
};
template <>
struct TagSuffix<units_internal::CentimeterScale, units_internal::LengthTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "cm"; }
};
template <>
struct TagSuffix<units_internal::MillimeterScale, units_internal::LengthTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "mm"; }
};
template <>
struct TagSuffix<units_internal::DegreeScale, units_internal::AngleTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "deg"; }
};
template <>
struct TagSuffix<units_internal::RadianScale, units_internal::AngleTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "rad"; }
};
template <>
struct TagSuffix<units_internal::GrayScale, units_internal::DoseTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "Gy"; }
};
template <>
struct TagSuffix<units_internal::CentiGrayScale, units_internal::DoseTag> {
  static NO_DISCARD constexpr const char* c_str() noexcept { return "cGy"; }
};

// Value types for units created using literals.
using LiteralFloatType = double; // literal: long double
using LiteralIntType = long long; // literal: unsigned long long

}  // namespace units_internal

// Template that can be customized to hold a value representing
// a unit of some sort, e.g. centimeters, radians, etc.
template <typename ArithT, typename ScaleT, typename TagT>
class Unit {
  static_assert(std::is_arithmetic_v<ArithT>, "ArithT must be arithmetic");
  static_assert(units_internal::is_ratio_v<ScaleT>, "ScaleT must be a ratio");
  static_assert(units_internal::is_tag_v<TagT>, "TagT must be a tag");
  ArithT value_;

 public:
  using ValueType = ArithT;
  using ScaleType = ScaleT;
  using TagType = TagT;

  // Ctor.
  //
  // NOTE(thinks):
  //   ctor not explicit here since copy-list-initialization cannot 
  //   use explicit constructors (C++14).
  // clang-format off
  /*explicit*/ constexpr Unit(ValueType&& v) 
    //noexcept(noexcept(ValueType{std::move(v)}))
      : value_{std::move(v)} {}
  // clang-format on    

  NO_DISCARD constexpr ValueType value() const noexcept { return value_; }

  // Add a unit.
  // Supports different scales since there is no ambiguity in return type.
  template<typename ArithT2, typename ScaleT2>
  constexpr auto operator+=(const Unit<ArithT2, ScaleT2, TagT> rhs) 
      // TODO(thinks): noexcept
      -> Unit& {
    value_ += unit_cast<Unit>(rhs).value();
    return *this;
  }

  // Subtract a unit.
  // Supports different scales since there is no ambiguity in return type.
  template<typename ArithT2, typename ScaleT2>
  constexpr auto operator-=(const Unit<ArithT2, ScaleT2, TagT> rhs) 
      // TODO(thinks): noexcept
      -> Unit& {
    value_ -= unit_cast<Unit>(rhs).value();
    return *this;
  }

  // Multiply by a scalar.
  // Here we consider the scalar to be dimension-less such that 
  // multiplication with the underlying value preserves the unit
  // dimension.
  template<typename ArithT2>
  constexpr auto operator*=(const ArithT2 rhs) 
      // TODO(thinks): noexcept
      -> Unit& {
    static_assert(std::is_arithmetic_v<ArithT2>, "ArithT2 must be arithmetic");    
    value_ *= rhs;    
    return *this;
  } 

  // Divide by a scalar.
  // Here we consider the scalar to be dimension-less such that 
  // multiplication with the underlying value preserves the unit
  // dimension.
  template<typename ArithT2>
  constexpr auto operator/=(const ArithT2 rhs) 
      // TODO(thinks): noexcept
      -> Unit& {
    static_assert(std::is_arithmetic_v<ArithT2>, "ArithT2 must be arithmetic");    
    value_ /= rhs;    
    return *this;
  } 

  // Equality comparison of same-tag-units with different value
  // type and/or scale. Normal comparison rules for arithmetic types apply.
  //
  // Allowing units of different scale here since there is no ambiguity in
  // return type.
  //
  // clang-format off
  template <typename ArithT2, typename ScaleT2>
  NO_DISCARD
  friend constexpr auto operator==(const Unit lhs,
                                   const Unit<ArithT2, ScaleT2, TagT> rhs) 
      //noexcept(noexcept(unit_cast<Unit<ArithT, ScaleT, TagT>>(rhs)))
      -> bool {
    return lhs.value() == unit_cast<Unit>(rhs).value();
  }
  // clang-format on

  // Inequality comparison of same-tag-units with different value
  // type and/or scale. Normal comparison rules for arithmetic types apply.
  //
  // Allowing units of different scale here since there is no ambiguity in
  // return type.
  //
  // clang-format off
  template <typename ArithT2, typename ScaleT2>
  NO_DISCARD
  friend constexpr auto operator!=(const Unit lhs,
                                   const Unit<ArithT2, ScaleT2, TagT> rhs) 
      //noexcept(noexcept(unit_cast<Unit<ArithT, ScaleT, TagT>>(rhs)))
      -> bool {
    return lhs.value() != unit_cast<Unit>(rhs).value();
  }
  // clang-format on

  // Unary negation.
  // The value type of the returned unit follows normal arithmetic promotion.
  //
  // clang-format off
  NO_DISCARD
  friend constexpr auto operator-(const Unit u)
      //noexcept(noexcept(-u.value())) 
      -> Unit<decltype(-u.value()), ScaleT, TagT> {
    return {-u.value()};
  }
  // clang-format on

  // Binary subtraction.
  // Supports different value types.
  // The value type of the returned unit follows normal arithmetic promotion.
  //
  // Requires the units to have the same scale factor, since the return type
  // would otherwise be ambiguous.
  //
  // clang-format off
  template <typename ArithT2>
  NO_DISCARD
  friend constexpr auto operator-(const Unit lhs,
                                  const Unit<ArithT2, ScaleT, TagT> rhs) 
      //noexcept(noexcept(lhs.value() - rhs.value()))
      -> Unit<decltype(lhs.value() - rhs.value()), ScaleT, TagT> {
    return {lhs.value() - rhs.value()};
  }
  // clang-format on

  // Binary addition.
  // Supports different value types.
  // The value type of the returned unit follows normal arithmetic promotion.
  //
  // Requires the units to have the same scale factor, since the return type
  // would otherwise be ambiguous.
  //
  // clang-format off
  template <typename ArithT2>
  NO_DISCARD
  friend constexpr auto operator+(const Unit lhs,
                                  const Unit<ArithT2, ScaleT, TagT> rhs) 
      //noexcept(noexcept(lhs.value() + rhs.value()))
      -> Unit<decltype(lhs.value() + rhs.value()), ScaleT, TagT> {
    return {lhs.value() + rhs.value()};
  }
  // clang-format on

  // Multiply by scalar (rhs). 
  // Preserves multiplicative ordering.
  // The value type of the returned unit follows normal arithmetic promotion.
  // 
  // clang-format off
  template <typename ArithT2>
  NO_DISCARD
  friend constexpr auto operator*(const Unit lhs,
                                  const ArithT2 rhs) 
      // noexcept...
      -> Unit<decltype(lhs.value() * rhs), ScaleT, TagT> {
    static_assert(std::is_arithmetic_v<ArithT2>, "ArithT2 must be arithmetic");    
    return {lhs.value() * rhs};    
  }
  // clang-format on

  // Multiply by scalar (lhs).  
  // Preserves multiplicative ordering.
  // The value type of the returned unit follows normal arithmetic promotion.
  // 
  // clang-format off
  template <typename ArithT2>
  NO_DISCARD
  friend constexpr auto operator*(const ArithT2 lhs,
                                  const Unit rhs) 
      // noexcept...
      -> Unit<decltype(lhs * rhs.value()), ScaleT, TagT> {
    static_assert(std::is_arithmetic_v<ArithT2>, "ArithT2 must be arithmetic");
    return {lhs * rhs.value()};
  }
  // clang-format on

  // Divide two same-tag-units to produce a scalar value.
  // Essentially, dimensionality is cancelled out by this operation.
  //
  // Allowing units of different scale here since there is no ambiguity in
  // return type, which is a scalar.
  // 
  // clang-format off
  template <typename ArithT2, typename ScaleT2>
  NO_DISCARD
  friend constexpr auto operator/(const Unit lhs, 
                                  const Unit<ArithT2, ScaleT2, TagT> rhs) 
      // noexcept...
      -> decltype(lhs.value() / unit_cast<Unit>(rhs).value()) {
    return lhs.value() / unit_cast<Unit>(rhs).value();
  }
  // clang-format on

  // Divide by scalar, preserves unit dimensionality.
  // 
  // clang-format off
  template <typename ArithT2>
  NO_DISCARD
  friend constexpr auto operator/(const Unit lhs, const ArithT2 rhs) 
      // noexcept...
      -> Unit<decltype(lhs.value() / rhs), ScaleT, TagT> {
    static_assert(std::is_arithmetic_v<ArithT2>, "ArithT2 must be arithmetic");
    return {lhs.value() / rhs};
  }
  // clang-format on
};

// Convert between units with the same tag that have potentially different
// scale factors and value types.
//
// clang-format off
template <typename ToUnitT, 
          typename FromArithT, typename FromScaleT, typename TagT>
NO_DISCARD          
constexpr auto unit_cast(const Unit<FromArithT, FromScaleT, TagT> from) 
    //noexcept(noexcept(
    //  units_internal::ScaleHelper<FromScaleT, typename ToUnitT::ScaleType>::
    //    Scale<typename ToUnitT::ValueType>(from.value())))
    -> Unit<typename ToUnitT::ValueType, typename ToUnitT::ScaleType, TagT> {
  static_assert(std::is_same_v<typename ToUnitT::TagType, TagT>,
                "units must have same tag");
  using ToScaleT = typename ToUnitT::ScaleType;
  using ScaleHelper = units_internal::ScaleHelper<FromScaleT, ToScaleT>;
  return {ScaleHelper::Scale<typename ToUnitT::ValueType>(from.value())};
}
// clang-format on

// Define user-visible types.
//
// clang-format off
template <typename ArithT> using Meters = Unit<ArithT, units_internal::MeterScale, units_internal::LengthTag>; 
template <typename ArithT> using Centimeters = Unit<ArithT, units_internal::CentimeterScale, units_internal::LengthTag>; 
template <typename ArithT> using Millimeters = Unit<ArithT, units_internal::MillimeterScale, units_internal::LengthTag>;

template <typename ArithT> using Degrees = Unit<ArithT, units_internal::DegreeScale, units_internal::AngleTag>; 
template <typename ArithT> using Radians = Unit<ArithT, units_internal::RadianScale, units_internal::AngleTag>; 

template <typename ArithT> using Gray = Unit<ArithT, units_internal::GrayScale, units_internal::DoseTag>; 
template <typename ArithT> using CentiGray = Unit<ArithT, units_internal::CentiGrayScale, units_internal::DoseTag>; 
// clang-format on

inline namespace unit_literals {

// Literals.
//
// NOTE(thinks): 
//   Would be nice if there was a way to specify the value type of 
//   the constructed units. Currently (C++14), there is no way to 
//   pass a type argument to a literal operator since templating
//   is limited for literals. Hence, we fall back on hard-coding
//   the value types for the constructed unit objects.
NO_DISCARD constexpr auto operator"" _m(unsigned long long v)
    // noexcept
    -> Meters<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _m(long double v)
    // noexcept
    -> Meters<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _cm(unsigned long long v)
    // noexcept
    -> Centimeters<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _cm(long double v)
    // noexcept
    -> Centimeters<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _mm(unsigned long long v)
    // noexcept
    -> Millimeters<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _mm(long double v)
    // noexcept
    -> Millimeters<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _deg(unsigned long long v)
    // noexcept
    -> Degrees<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _deg(long double v)
    // noexcept 
    -> Degrees<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _rad(unsigned long long v)
    // noexcept 
    -> Radians<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _rad(long double v)
    // noexcept 
    -> Radians<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _Gy(unsigned long long v)
    // noexcept 
    -> Gray<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _Gy(long double v)
    // noexcept 
    -> Gray<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

NO_DISCARD constexpr auto operator"" _cGy(unsigned long long v)
    // noexcept 
    -> CentiGray<units_internal::LiteralIntType> {
  return {units_internal::numeric_cast<units_internal::LiteralIntType>(v)};
}
NO_DISCARD constexpr auto operator"" _cGy(long double v)
    // noexcept 
    -> CentiGray<units_internal::LiteralFloatType> {
  return {units_internal::numeric_cast<units_internal::LiteralFloatType>(v)};
}

} // namespace literals

// Simple output overload, prints unit suffix after value.
//
// clang-format off
template <typename ArithT, typename ScaleT, typename TagT>
std::ostream& operator<<(std::ostream& os,
                         const Unit<ArithT, ScaleT, TagT>& rhs) {
  os << rhs.value() 
     << " [" << units_internal::TagSuffix<ScaleT, TagT>::c_str() << "]";
  return os;
}
// clang-format on

#undef NO_DISCARD

}  // namespace thinks
