#include <algorithm>
#include <limits>
#include <type_traits>

/// T can contain Min and Max.
template <typename T, auto Min, auto Max>
concept CanContainBounds =
    std::is_arithmetic_v<T> && std::numeric_limits<T>::max() >= Max &&
    std::numeric_limits<T>::lowest() <= Min && Min <= Max;

/// T and U are either both integrals or both floating point types.
template <typename T, typename U>
concept SameNumericType = (std::integral<T> && std::integral<U>) ||
                          (std::floating_point<T> && std::floating_point<U>);

/*
 * A numeric type that is bounded by Min and Max.
 * It requires the underlying storage type T to be able to represent Min and
 * Max.
 */
template <typename T, auto Min, auto Max>
  requires CanContainBounds<T, Min, Max>
class BoundedNumber {
 public:
  constexpr explicit BoundedNumber(auto v) { set(v); }

  /*
   * Sets a value by first clamping it in the range [Min, Max].
   * Always allows setting integral values.
   * Allows setting floating point values iff the underlying storage type is a
   * floating point number.
   */
  template <typename U>
    requires(SameNumericType<T, U> ||
             (std::floating_point<T> && std::integral<U>))
  constexpr void set(U v) {
    if constexpr (CanContainBounds<U, Min, Max>) {
      m_value = static_cast<T>(
          std::clamp(v, static_cast<U>(Min), static_cast<U>(Max)));
    } else {
      // T can represent v
      m_value = std::clamp(static_cast<T>(v), static_cast<T>(Min),
                           static_cast<T>(Max));
    }
  }

  [[nodiscard]] constexpr auto value() const noexcept { return m_value; }

 private:
  T m_value;
};

// Example of a template instantiation for decibels and their integer
// representation
using dB = BoundedNumber<double, -100, 0>;
using dBn = BoundedNumber<int, 0, 1000>;

[[nodiscard]] inline consteval dBn operator""_dbn(unsigned long long value) {
  return dBn(value);
}

int main() {
  // Example 1: Valid construction with in-range value
  constexpr auto dbnInRange = dBn(10);
  static_assert(dbnInRange.value() == 10);

  // Example 2: Clamping to upper bound
  constexpr auto dbnClampUpper =
      dBn(std::numeric_limits<unsigned long long>::max());
  static_assert(dbnClampUpper.value() == 1000);

  // Example 3: Clamping to lower bound
  constexpr auto dbnClampLower = dBn(std::numeric_limits<long long>::min());
  static_assert(dbnClampLower.value() == 0);

  // Example 4: User-defined literal with valid integer value
  constexpr auto dbnLiteral = 10_dbn;
  static_assert(dbnLiteral.value() == 10);

  // Compile-time Errors:
  // constexpr auto dbnInvalidType = dBn(10.0); // Error: double not allowed
  // constexpr auto dbnInvalidBounds = BoundedNumber<unsigned char, -128, 127>(128); // Error: -128 not valid for unsigned char
  // constexpr auto dbnInvalidLiteral = 10.3_dbn; // Error: fractional literals not allowed for _dbn

  // Example 5: Valid construction with in-range value
  constexpr auto dbInRange = dB(-10.5);
  static_assert(dbInRange.value() == -10.5);

  // Example 6: Construction with implicit conversion from integer
  constexpr auto dbFromInteger = dB(-10);
  static_assert(dbFromInteger.value() == -10);

  // Example 7: Clamping to upper bound
  constexpr auto dbClampUpper = dB(std::numeric_limits<long double>::max());
  static_assert(dbClampUpper.value() == 0);

  // Example 8: Clamping to lower bound
  constexpr auto dbClampLower = dB(std::numeric_limits<long double>::lowest());
  static_assert(dbClampLower.value() == -100);

  // Example 10: Exact match with upper bound
  constexpr auto dbExactUpper = dB(0.0);
  static_assert(dbExactUpper.value() == 0.0);

  // Example 11: Exact match with lower bound
  constexpr auto dbExactLower = dB(-100.0);
  static_assert(dbExactLower.value() == -100.0);

  // Example 12: Exact match with upper bound
  constexpr auto dbnExactUpper = dBn(1000);
  static_assert(dbnExactUpper.value() == 1000);

  // Example 13: Exact match with lower bound
  constexpr auto dbnExactLower = dBn(0);
  static_assert(dbnExactLower.value() == 0);

  return 0;
}
