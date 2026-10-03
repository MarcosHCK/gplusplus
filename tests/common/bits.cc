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
#include <config.h>
#include <cstdint>
#include <gplusplus/common/bits.h>
#include <tests/testing.h>
using namespace testing;

/* compile-time behavior */

static_assert (bits::log2_v<1> == 0);
static_assert (bits::log2_v<2> == 1);
static_assert (bits::log2_v<4> == 2);
static_assert (bits::log2_v<8> == 3);
static_assert (bits::log2_v<16> == 4);
static_assert (bits::log2_v<32> == 5);
static_assert (bits::log2_v<64> == 6);
static_assert (bits::log2_v<128> == 7);
static_assert (bits::log2_v<256> == 8);
static_assert (bits::log2_v<65536> == 16);

static_assert (bits::is_pow2_v<1>);
static_assert (bits::is_pow2_v<2>);
static_assert (bits::is_pow2_v<4>);
static_assert (bits::is_pow2_v<65536>);
static_assert (! bits::is_pow2_v<0>);
static_assert (! bits::is_pow2_v<3>);
static_assert (! bits::is_pow2_v<6>);
static_assert (! bits::is_pow2_v<12>);

static_assert (bits::align_up<1> (0uz) == 0);
static_assert (bits::align_up<1> (3uz) == 3);
static_assert (bits::align_up<2> (0uz) == 0);
static_assert (bits::align_up<2> (1uz) == 2);
static_assert (bits::align_up<2> (2uz) == 2);
static_assert (bits::align_up<4> (5uz) == 8);
static_assert (bits::align_up<8> (41uz) == 48);
static_assert (bits::align_up<8> (48uz) == 48);
static_assert (bits::align_up<8> (49uz) == 56);
static_assert (bits::align_up<5> (13uz) == 15);
static_assert (bits::align_up<5> (14uz) == 15);
static_assert (bits::align_up<5> (15uz) == 15);
static_assert (bits::align_up<5> (16uz) == 20);

static_assert (bits::rot<1, std::uint8_t> (0x80) == 0x01);
static_assert (bits::rot<1, std::uint8_t> (0x01) == 0x02);
static_assert (bits::rot<4, std::uint8_t> (0x0F) == 0xF0);
static_assert (bits::rot<7, std::uint8_t> (0x7F) == 0xBF);
static_assert (bits::rot<8, std::uint16_t> (0x00FF) == 0xFF00);
static_assert (bits::rot<1, std::uint16_t> (0xFFFF) == 0xFFFF);
static_assert (bits::rot<32, std::uint64_t> (0x0000000000000001ull) == 0x0000000100000000ull);
static_assert (bits::rot<1, std::uint64_t> (0x8000000000000000ull) == 0x0000000000000001ull);

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/align_up", []
    {

      for (guint sample = 0; sample < 128; ++sample)
        {

          auto value = (gsize) ((guint64) g_test_rand_int () << 32 | (guint64) g_test_rand_int ());
          auto aligned = bits::align_up<8> (value);

          g_assert_cmpuint (aligned, >=, value);
          g_assert_cmpuint (aligned % 8, ==, 0);
          g_assert_cmpuint (aligned - value, <, 8);
          g_assert_cmpuint (bits::align_up<8> (aligned), ==, aligned);
        }
    });

  g_test_add_action (TESTPATHROOT "/align_up_nontrivial", []
    {

      /* alignment to a value that is not a power of two */
      for (guint sample = 0; sample < 128; ++sample)
        {

          auto value = (gsize) ((guint64) g_test_rand_int () << 32 | (guint64) g_test_rand_int ());
          auto aligned = bits::align_up<6> (value);

          g_assert_cmpuint (aligned, >=, value);
          g_assert_cmpuint (aligned % 6, ==, 0);
          g_assert_cmpuint (aligned - value, <, 6);
          g_assert_cmpuint (bits::align_up<6> (aligned), ==, aligned);
        }
    });

  g_test_add_action (TESTPATHROOT "/rot", []
    {
      for (guint sample = 0; sample < 128; ++sample)
        {

          guint64 value = g_test_rand_uint64 ();
          guint64 rotated = bits::rot<1, std::uint64_t> (value);

          /* a rotate is an involution across complementary widths: rot<63> then rot<1> */
          g_assert_cmpuint ((guint64) (bits::rot<63, std::uint64_t> (rotated)), ==, (guint64) value);

          std::uint32_t v32 = (std::uint32_t) value;
          g_assert_cmpuint ((guint64) (bits::rot<16, std::uint32_t> (bits::rot<16, std::uint32_t> (v32))), ==, (guint64) v32);
          g_assert_cmpuint ((guint64) (bits::rot<1, std::uint32_t> (bits::rot<31, std::uint32_t> (v32))), ==, (guint64) v32);

          std::uint16_t v16 = (std::uint16_t) value;
          g_assert_cmpuint ((guint64) (bits::rot<5, std::uint16_t> (bits::rot<11, std::uint16_t> (v16))), ==, (guint64) v16);

          std::uint8_t v8 = (std::uint8_t) value;
          g_assert_cmpuint ((guint64) (bits::rot<3, std::uint8_t> (bits::rot<5, std::uint8_t> (v8))), ==, (guint64) v8);
        }
    });

return g_test_run ();
}