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
#include <gio++/common/boxing.h>
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

return g_test_run ();
}