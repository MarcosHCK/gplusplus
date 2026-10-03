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
#include <array>
#include <config.h>
#include <cstdint>
#include <gplusplus/common/bits.h>
#include <gplusplus/common/hashing.h>
#include <span>
#include <tests/testing.h>
using namespace testing;

/* Known-answer vectors computed independently (see the reference FNV-1a and
 * djb2 definitions), so these tests catch encoding or arithmetic bugs in the
 * implementations rather than only internal inconsistencies. */

static_assert (hashing::fnv_1a<std::uint64_t, std::uint8_t> (std::span<const std::uint8_t> ()) == 14695981039346656037ull);
static_assert (hashing::fnv_1a<std::uint32_t, std::uint8_t> (std::span<const std::uint8_t> ()) == 2166136261u);
static_assert (hashing::fnv_1a<std::uint16_t, std::uint8_t> (std::span<const std::uint8_t> ()) == 1313u);
static_assert (hashing::fnv_1a<std::uint8_t, std::uint8_t> (std::span<const std::uint8_t> ()) == 101u);
static_assert (hashing::djb<std::uint64_t, std::uint8_t> (std::span<const std::uint8_t> ()) == 5381ull);

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/known", []
    {

      const std::array<std::uint8_t, 1> a = { 'a' };
      const std::array<std::uint8_t, 5> hello = { 'h', 'e', 'l', 'l', 'o' };

      g_assert_cmpuint ((hashing::fnv_1a<std::uint64_t, std::uint8_t> (std::span (a))), ==, 12638187200555641996ull);
      g_assert_cmpuint ((hashing::fnv_1a<std::uint64_t, std::uint8_t> (std::span (hello))), ==, 11831194018420276491ull);

      g_assert_cmpuint ((hashing::fnv_1a<std::uint32_t, std::uint8_t> (std::span (a))), ==, 3826002220u);
      g_assert_cmpuint ((hashing::fnv_1a<std::uint32_t, std::uint8_t> (std::span (hello))), ==, 1335831723u);

      g_assert_cmpuint ((hashing::fnv_1a<std::uint16_t, std::uint8_t> (std::span (a))), ==, 22720u);
      g_assert_cmpuint ((hashing::fnv_1a<std::uint16_t, std::uint8_t> (std::span (hello))), ==, 64879u);

      g_assert_cmpuint ((hashing::fnv_1a<std::uint8_t, std::uint8_t> (std::span (a))), ==, 236u);
      g_assert_cmpuint ((hashing::fnv_1a<std::uint8_t, std::uint8_t> (std::span (hello))), ==, 91u);

      g_assert_cmpuint ((hashing::djb<std::uint64_t, std::uint8_t> (std::span (a))), ==, 177670ull);
      g_assert_cmpuint ((hashing::djb<std::uint64_t, std::uint8_t> (std::span (hello))), ==, 210714636441ull);
    });


  g_test_add_action (TESTPATHROOT "/deterministic", []
    {

      for (guint sample = 0; sample < 32; ++sample)
        {

          auto [ memory, length ] = g_test_rand_data ();
          auto duplicate = g_memdup2 (memory, length);

          std::span<const std::uint8_t> a ((const std::uint8_t*) *memory, length);
          std::span<const std::uint8_t> b ((const std::uint8_t*) duplicate, length);

          g_assert_cmpuint ((hashing::fnv_1a<std::uint64_t, std::uint8_t> (a)), ==, (hashing::fnv_1a<std::uint64_t, std::uint8_t> (b)));
          g_assert_cmpuint ((hashing::djb<std::uint64_t, std::uint8_t> (a)), ==, (hashing::djb<std::uint64_t, std::uint8_t> (b)));

          /* a single changed byte changes the digest (64-bit, effectively
           * collision-free for one-byte perturbations) */
          ((std::uint8_t*) duplicate) [0] ^= 0xFF;
          g_assert_cmpuint ((hashing::fnv_1a<std::uint64_t, std::uint8_t> (a)), !=, (hashing::fnv_1a<std::uint64_t, std::uint8_t> (b)));
          g_assert_cmpuint ((hashing::djb<std::uint64_t, std::uint8_t> (a)), !=, (hashing::djb<std::uint64_t, std::uint8_t> (b)));

          g_free (duplicate);
        }
    });

  g_test_add_action (TESTPATHROOT "/diffusion", []
    {

      /* a single bit flip in the input should, on average, flip about half of
       * the output bits; assert the far weaker property that it actually
       * changes more than one output bit, to keep the test robust */
      for (guint sample = 0; sample < 64; ++sample)
        {

          auto first = g_test_rand_int ();
          auto second = first ^ (1u << (guint) g_test_rand_int_range (0, 31));

          auto x = hashing::fnv_1a<std::uint64_t, std::uint8_t> (std::span ((const std::uint8_t*) &first, sizeof (first)));
          auto y = hashing::fnv_1a<std::uint64_t, std::uint8_t> (std::span ((const std::uint8_t*) &second, sizeof (second)));

          auto diff = x ^ y;
          g_assert_cmpuint (diff, !=, 0);
          g_assert_false ((diff & (diff - 1)) == 0);
        }
    });

  g_test_add_action (TESTPATHROOT "/mix", []
    {

      guint64 a = (guint64) g_test_rand_uint64 ();
      guint64 b = (guint64) g_test_rand_uint64 ();
      guint64 c = (guint64) g_test_rand_uint64 ();

      /* deterministic */
      g_assert_cmpuint (hashing::mix (a, b, c), ==, hashing::mix (a, b, c));

      /* every input bit position participates */
      for (guint bit = 0; bit < 64; ++bit)
        {
          auto other = (guint64) (a ^ (1ull << bit));
          auto diff = hashing::mix (a, b, c) ^ hashing::mix (other, b, c);
          g_assert_cmpuint (diff, !=, 0);
          g_assert_false ((diff & (diff - 1)) == 0);
        }

      /* and so does the salt */
      for (guint bit = 0; bit < 64; ++bit)
        {
          auto other = (guint64) (c ^ (1ull << bit));
          auto diff = hashing::mix (a, b, c) ^ hashing::mix (a, b, other);
          g_assert_cmpuint (diff, !=, 0);
        }
    });

return g_test_run ();
}
