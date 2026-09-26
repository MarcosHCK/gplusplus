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
#include <config.h>
#include <functional>
#include <gio++/common/boxing.h>
#include <gio++/common/hashing.h>
#include <span>
#include <string_view>
#include <tests/testing.h>
using namespace testing;

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/new", []
    {

      auto bytes = g_bytes_new_static ("123456789", 10);
      auto box = boxing::bytes (bytes);
    });

  g_test_add_action (TESTPATHROOT "/copy", []
    {

      auto bytes = g_bytes_new_static ("123456789", 10);

      auto box1 = boxing::bytes (bytes);
      auto box2 = boxing::bytes (box1);

      g_assert_cmpint ((guintptr) *box1, ==, (guintptr) *box2);
    });

  g_test_add_action (TESTPATHROOT "/move", []
    {

      auto bytes = g_bytes_new_static ("123456789", 10);

      auto box1 = boxing::bytes (bytes);
      auto box2 = boxing::bytes (std::move (box1));

      g_assert_cmpint (0, ==, (guintptr) *box1);
      g_assert_cmpint ((guintptr) bytes, ==, (guintptr) *box2);
    });

  g_test_add_action (TESTPATHROOT "/compare_null", []
    {

      auto bytes = g_bytes_new_static ("123456789", 10);

      auto box1 = boxing::bytes ();
      auto box2 = boxing::bytes (bytes);

      g_assert_true (nullptr == box1);
      g_assert_true (nullptr != box2);
      g_assert_true (box1 != box2);
    });

  g_test_add_action (TESTPATHROOT "/compare_data", []
    {

      auto [ data1, length1 ] = g_test_rand_data ();
      auto bytes1 = g_bytes_new_take (data1.steal (), length1);

      auto [ data2, length2 ] = g_test_rand_data ();
      auto bytes2 = g_bytes_new_take (data2.steal (), length2);

      auto box1 = boxing::bytes (bytes1);
      auto box2 = boxing::bytes (bytes2);
      auto box3 = boxing::bytes (box2);
      auto view = std::string_view ((char*) g_bytes_get_data (box2, NULL), length2);

      g_assert_true (box1 != box2);
      g_assert_true (box1 != box3);
      g_assert_true (box2 == box3);
      g_assert_true (view == box3);
    });

  g_test_add_action (TESTPATHROOT "/hash", []
    {

      auto [ data1, length1 ] = g_test_rand_data ();
      auto data2 = (guint8*) g_memdup2 (data1, length1);

      auto box1 = boxing::bytes (g_bytes_new_take (data1.steal (), length1));
      auto box2 = boxing::bytes (g_bytes_new_take (data2, length1));

      g_assert_true (box1 == box2);

      /* equal content yields equal hashes regardless of the instance */
      g_assert_cmpuint (std::hash<boxing::bytes> {} (box1), ==, std::hash<boxing::bytes> {} (box2));

      if (0 < length1)
        {

          /* a single flipped byte changes the hash */
          ((guint8*) g_bytes_get_data (*box2, NULL)) [0] ^= 0xFF;

          g_assert_false (box1 == box2);
          g_assert_cmpuint (std::hash<boxing::bytes> {} (box1), !=, std::hash<boxing::bytes> {} (box2));
        }

      /* the hash of a null box is well-defined (hash of the empty buffer) */
      g_assert_true (nullptr == boxing::bytes ());
      g_assert_cmpuint (std::hash<boxing::bytes> {} (boxing::bytes ()), ==, (hashing::fnv_1a<std::size_t, std::uint8_t> (std::span<const std::uint8_t> ())));
    });

  g_test_add_action (TESTPATHROOT "/string_view", []
    {

      auto [ data, length ] = g_test_rand_data ();
      auto view = std::string_view ((const char*) *data, length);
      auto box = boxing::bytes (g_bytes_new_take (data.steal (), length));

      g_assert_true (box == view);
      g_assert_false (box != view);
      g_assert_true (view == box);

      /* different content compares unequal */
      if (1 < length)
        {
          auto shorter = view.substr (0, length / 2);
          g_assert_false (box == shorter);
        }

      /* comparing with an empty view is well-defined */
      auto empty = boxing::bytes (g_bytes_new (NULL, 0));
      g_assert_true (empty == std::string_view ());
      g_assert_false (empty == view);
    });

  g_test_add_action (TESTPATHROOT "/data", []
    {

      auto [ data, length ] = g_test_rand_data ();
      auto raw_data = (gconstpointer) *data;
      auto box = boxing::bytes (g_bytes_new_take (data.steal (), length));

      auto [ ptr, size ] = box.data ();

      g_assert_cmpuint (size, ==, length);
      g_assert_cmpint (0, ==, std::memcmp (ptr, raw_data, length));

      auto nothing = boxing::bytes ();
      auto [ nptr, nsize ] = nothing.data ();
      g_assert_true (nptr == nullptr);
      g_assert_cmpuint (nsize, ==, 0);
    });

return g_test_run ();
}