#pragma once
#include <cmath>


namespace luna {

// https://www.gamedev.net/articles/programming/general-and-gameplay-programming/inverse-lerp-a-super-useful-yet-often-overlooked-function-r5230/
// thank you Freya Holmér, you are so cool and awesome

/**
 * @brief Returns a blend between a and be based on fraction t.
 * Example: lerp(5, 25, 0.5) = 15
 * 
 * @param a first value
 * @param b second value
 * @param t fraction
 * @return constexpr T 
 */
template <typename T, typename U>
inline constexpr T lerp (T a, T b, U t) {
    return ((U)1 - t) * a + b * t;
}

/**
 * @brief Returns fraction t, based on value v between a and b.
 * Example: inv_lerp(5, 25, 15) = 0.5
 * 
 * @param a first value
 * @param b second value
 * @param v value between a and b
 * @return constexpr T fraction t
 */
template <typename T, typename U>
inline constexpr T inv_lerp (T a, T b, U v) {
    return a == b ? (U)0 : ((v - a) / (b - a));
}

/**
 * @brief Remaps a value from one range to another.
 * Example: remap(5, 25, 0, 100, 15) = 50
 * 
 * @param i_min initial range minimum
 * @param i_max initial range maximum
 * @param o_min output range minimum
 * @param o_max output range maximum
 * @param v remapped value
 * @return constexpr T 
 */
template <typename T, typename U, typename V>
inline constexpr U remap (T i_min, T i_max, U o_min, U o_max, V v) {
    return lerp(o_min, o_max, inv_lerp(i_min, i_max, v));
}

template <typename T>
inline constexpr T clamp (T value, T min, T max) {
    return value < min ? min : (value > max ? max : value);
}


template <typename T, typename U>
inline constexpr T smooth_lerp (T a, T b, U r, U dt) {
    return b + (a - b) * std::pow(r, dt);
}

template <typename T>
inline constexpr int sign (T a) {
    return a > 0 ? 1 : (a < 0 ? -1 : 0);
}


} // namespace luna


