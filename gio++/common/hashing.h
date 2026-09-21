/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of gio++.
 *
 * gio++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gio++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once
#define __GLIB_H_INSIDE__
#include <glibconfig.h>
#include <gio++/common/bits.h>
#include <ranges>

#ifndef G_GUINT32_CONSTANT
# define G_GUINT32_CONSTANT(val) val##u;
#endif // G_GUINT32_CONSTANT

namespace hashing
{

  template<std::unsigned_integral T>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T mix (T key_a, T key_b, T salt) noexcept G_GNUC_PURE;

  template<std::unsigned_integral T>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T mix (T key_a, T key_b, T salt) noexcept
    {
      key_a -= salt; key_a ^= bits::rot<4, T> (salt); salt += key_b;
      key_b -= key_a; key_b ^= bits::rot<6, T> (key_a); key_a += salt;
      salt -= key_b; salt ^= bits::rot<8, T> (key_b); key_b += key_a;
      key_a -= salt; key_a ^= bits::rot<16, T> (salt); salt += key_b;
      key_b -= key_a; key_b ^= bits::rot<19, T> (key_a); key_a += salt;
      salt -= key_b; salt ^= bits::rot<4, T> (key_b);
    return salt;
    }

  namespace details
    {

      template<std::integral T>
        struct __djb_magic { static inline constexpr bool exists = false; };

      template<>
        struct __djb_magic<uint32_t> { static inline constexpr bool exists = true;
                                       static inline constexpr uint32_t value = G_GUINT32_CONSTANT (5381); };

      template<>
        struct __djb_magic<uint64_t> { static inline constexpr bool exists = true;
                                       static inline constexpr uint64_t value = G_GUINT64_CONSTANT (14695981039346656037); };

      template<std::integral T>
        struct __fnv_1a_magics { static inline constexpr bool exists = false; };

      template<>
        struct __fnv_1a_magics<uint8_t> { static inline constexpr bool exists = true;
                                          static inline constexpr uint8_t basis = 101;
                                          static inline constexpr uint8_t prime = 251; };

      template<>
        struct __fnv_1a_magics<uint16_t> { static inline constexpr bool exists = true;
                                           static inline constexpr uint16_t basis = 1313;
                                           static inline constexpr uint16_t prime = 40343; };

      template<>
        struct __fnv_1a_magics<uint32_t> { static inline constexpr bool exists = true;
                                           static inline constexpr uint32_t basis = G_GUINT32_CONSTANT (2166136261);
                                           static inline constexpr uint32_t prime = G_GUINT32_CONSTANT (16777619); };

      template<>
        struct __fnv_1a_magics<uint64_t> { static inline constexpr bool exists = true;
                                           static inline constexpr uint64_t basis = G_GUINT64_CONSTANT (14695981039346656037);
                                           static inline constexpr uint64_t prime = G_GUINT64_CONSTANT (1099511628211); };
    }

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T djb (V&& view) noexcept G_GNUC_PURE;

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T djb (V&& view) noexcept
    {

      static_assert (details::__djb_magic<T>::exists, "DjB magic value is not defined for this type");

      T hash = details::__djb_magic<T>::value; for (const D item: view)
        hash = ((hash << 5) + hash) + item;
    return hash;
    }

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T fnv_1a (V&& view) noexcept G_GNUC_PURE;

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T fnv_1a (V&& view) noexcept
    {

      static_assert (details::__fnv_1a_magics<T>::exists, "FNV-1a magic values are not defined for this type");

      T hash = details::__fnv_1a_magics<T>::basis; for (const D item: view)
        hash = (hash ^ (T) item) * details::__fnv_1a_magics<T>::prime;
    return hash;
    }
}