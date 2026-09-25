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

      boxing::variant variant = g_variant_new ("(ss)", "some value", "some other value");
      boxing::variant_iter iterator = boxing::variant_iter (variant);
    });

  g_test_add_action (TESTPATHROOT "/iteration", []
    {

      auto [ str1, l1 ] = g_test_rand_cstring ();
      auto [ str2, _ ] = g_test_rand_cstring ();

      boxing::variant variant = g_variant_new ("(ss)", *str1, *str2);
      boxing::variant variant1a (g_variant_new_string ((gchar*) *str1));
      boxing::variant variant1b (g_variant_new_string ((gchar*) *str2));
      boxing::variant_iter end, iterator = boxing::variant_iter (variant);

      g_assert_false (nullptr == *iterator);
      g_assert_false (end == iterator);
      g_variant_check_format_string (*iterator, "s", FALSE);
      g_assert_cmpvariant (*iterator, variant1a);

      ++iterator;
      g_assert_false (nullptr == *iterator);
      g_assert_false (end == iterator);
      g_variant_check_format_string (*iterator, "s", FALSE);
      g_assert_cmpvariant (*iterator, variant1b);

      ++iterator;
      g_assert_true (nullptr == *iterator);
      g_assert_true (end == iterator);
    });

return g_test_run ();
}