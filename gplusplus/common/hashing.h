/* Copyright (C) 2025-2026 MarcosHCK
 * This file is part of gplusplus.
 *
 * gplusplus is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gplusplus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once
#include <gplusplus/common/bits.h>
#include <ranges>

namespace hashing
{

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
                                       static inline constexpr uint32_t value = 5381; };

      template<>
        struct __djb_magic<uint64_t> { static inline constexpr bool exists = true;
                                       static inline constexpr uint64_t value = 5381; };

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
                                           static inline constexpr uint32_t basis = 2166136261;
                                           static inline constexpr uint32_t prime = 16777619; };

      template<>
        struct __fnv_1a_magics<uint64_t> { static inline constexpr bool exists = true;
                                           static inline constexpr uint64_t basis = 14695981039346656037llu;
                                           static inline constexpr uint64_t prime = 1099511628211llu; };
    }

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T djb (V view) noexcept
    {

      static_assert (details::__djb_magic<T>::exists, "DjB magic value is not defined for this type");

      T hash = details::__djb_magic<T>::value; for (const D item: view)
        hash = ((hash << 5) + hash) + static_cast<T> (item);
    return hash;
    }

  template<std::unsigned_integral T, std::integral D,
           std::ranges::view V>
  requires std::same_as<D, std::ranges::range_value_t<V>>
  [[gnu::always_inline]] [[gnu::pure]]
  static inline constexpr T fnv_1a (V view) noexcept
    {

      static_assert (details::__fnv_1a_magics<T>::exists, "FNV-1a magic values are not defined for this type");

      T hash = details::__fnv_1a_magics<T>::basis;

      for (const D item: view)
        {

          /* widen the operands before multiplying: for uint8/uint16 hash and
           * prime would otherwise be promoted to (signed) int, and
           * (basis ^ item) * prime overflows it for many inputs (UB) */
          const auto key = static_cast<T> (item);

          if constexpr (sizeof (T) < sizeof (unsigned int))

            hash = static_cast<T> ((static_cast<unsigned int> (hash) ^ static_cast<unsigned int> (key))
                                  * static_cast<unsigned int> (details::__fnv_1a_magics<T>::prime));
          else
            hash = static_cast<T> ((hash ^ key) * details::__fnv_1a_magics<T>::prime);
        }
    return hash;
    }
}