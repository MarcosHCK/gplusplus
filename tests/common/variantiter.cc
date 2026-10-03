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
#include <gplusplus/common/boxing.h>
#include <tests/testing.h>
using namespace testing;

/* NOTE: g_assert_cmpvariant() captures its operands as raw GVariant* at the
 * start of a statement, so a boxing::variant passed as a temporary (e.g. `*i`
 * or `boxing::variant (g_variant_new_string (...))`) is destroyed at the end
 * of that capture statement and leaves the macro comparing through a dangling
 * pointer. Always bind the values to named locals first. */

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

      auto first = *iterator;
      ++iterator;
      auto second = *iterator;
      ++iterator;

      g_assert_false (nullptr == first);
      g_assert_cmpvariant (first, variant1a);
      g_assert_false (nullptr == second);
      g_assert_cmpvariant (second, variant1b);
      g_assert_true (nullptr == *iterator);
      g_assert_true (end == iterator);
    });

  g_test_add_action (TESTPATHROOT "/copy_parallel", []
    {

      /* a copied iterator advances independently of the source */
      boxing::variant variant = g_variant_new ("(ss)", "alpha", "beta");
      boxing::variant_iter i1 = boxing::variant_iter (variant);
      boxing::variant_iter i2 (i1);

      boxing::variant a1, a2, b1, b2, b2_again;
      a1 = *i1;
      a2 = *i2;
      ++i1;
      ++i2;
      b1 = *i1;
      b2 = *i2;
      ++i1;
      b2_again = *i2;

      boxing::variant alpha (g_variant_new_string ("alpha"));
      boxing::variant beta (g_variant_new_string ("beta"));

      g_assert_cmpvariant (a1, alpha);
      g_assert_cmpvariant (a2, alpha);
      g_assert_cmpvariant (b1, beta);
      g_assert_cmpvariant (b2, beta);
      g_assert_true (nullptr == *i1);
      g_assert_cmpvariant (b2_again, beta);

      ++i2;
      g_assert_true (nullptr == *i2);
    });

  g_test_add_action (TESTPATHROOT "/assign", []
    {

      /* Regression: operator= used to copy the internal iteration state but
       * drop the cached current value, so the assigned iterator dereferenced
       * to the stale/default (null) value. */
      boxing::variant variant = g_variant_new ("(ss)", "first", "second");
      boxing::variant_iter end, iterator = boxing::variant_iter (variant);
      boxing::variant_iter assigned;

      boxing::variant first (g_variant_new_string ("first"));
      boxing::variant second (g_variant_new_string ("second"));

      auto got1 = *iterator;
      g_assert_cmpvariant (got1, first);

      assigned = iterator;
      auto got2 = *assigned;
      g_assert_cmpvariant (got2, first);

      ++assigned;
      auto got3 = *assigned;
      g_assert_cmpvariant (got3, second);

      /* the source is unaffected */
      auto got4 = *iterator;
      g_assert_cmpvariant (got4, first);

      ++iterator;
      ++iterator;
      g_assert_true (end == iterator);
      ++assigned;
      g_assert_true (end == assigned);
    });

  g_test_add_action (TESTPATHROOT "/array", []
    {

      const gchar* strings [] = { "one", "two", "three" };
      boxing::variant variant = g_variant_new_strv (strings, G_N_ELEMENTS (strings));

      boxing::variant_iter end, iterator = boxing::variant_iter (variant);
      gsize count = 0;

      for (; iterator != end; ++iterator)
        {
          auto entry = *iterator;
          g_assert_true (g_variant_is_of_type (entry, G_VARIANT_TYPE_STRING));
          ++count;
        }

      g_assert_cmpuint (count, ==, G_N_ELEMENTS (strings));
    });

  g_test_add_action (TESTPATHROOT "/dict", []
    {

      GVariantBuilder builder = G_VARIANT_BUILDER_INIT (G_VARIANT_TYPE ("a{sv}"));
      g_variant_builder_add (&builder, "{sv}", "key1", g_variant_new_string ("value1"));
      g_variant_builder_add (&builder, "{sv}", "key2", g_variant_new_int32 (7));
      boxing::variant variant = g_variant_builder_end (&builder);

      boxing::variant_iter end, iterator = boxing::variant_iter (variant);
      gsize count = 0;

      for (; iterator != end; ++iterator)
        {
          auto entry = *iterator;
          g_assert_true (g_variant_is_of_type (entry, G_VARIANT_TYPE ("{sv}")));
          ++count;
        }

      g_assert_cmpuint (count, ==, 2);
    });

  g_test_add_action (TESTPATHROOT "/empty", []
    {

      /* an empty tuple yields an end (==) iterator from the start */
      boxing::variant tuple = g_variant_new ("()");
      boxing::variant_iter end, iterator = boxing::variant_iter (tuple);

      g_assert_true (end == iterator);
      g_assert_true (nullptr == *iterator);

      /* so does an empty array */
      boxing::variant array = g_variant_new_strv (NULL, 0);
      boxing::variant_iter aend, aiterator = boxing::variant_iter (array);

      g_assert_true (aend == aiterator);
    });

  g_test_add_action (TESTPATHROOT "/variant_type", []
    {

      /* a "v" (variant) type is a container: it iterates over its single
       * contained value */
      boxing::variant wrapped = g_variant_new_variant (g_variant_new_int32 (42));

      boxing::variant_iter end, iterator = boxing::variant_iter (wrapped);

      g_assert_false (end == iterator);
      g_assert_cmpint (42, ==, g_variant_get_int32 (*iterator));

      ++iterator;
      g_assert_true (end == iterator);
    });

  g_test_add_action (TESTPATHROOT "/post_increment", []
    {

      boxing::variant variant = g_variant_new ("(ss)", "first", "second");
      boxing::variant_iter iterator = boxing::variant_iter (variant);

      auto previous = iterator++;
      boxing::variant first (g_variant_new_string ("first"));
      boxing::variant second (g_variant_new_string ("second"));

      auto got1 = *previous;
      auto got2 = *iterator;
      g_assert_cmpvariant (got1, first);
      g_assert_cmpvariant (got2, second);
    });

  g_test_add_action (TESTPATHROOT "/non_container", []
    {

      /* a plain string is not iterable */
      boxing::variant string_variant = g_variant_new_string ("hello");
      g_assert_throws (std::runtime_error, ({ boxing::variant_iter iterator (string_variant); }));

      /* nor is the null pointer */
      g_assert_throws (std::runtime_error, ({ boxing::variant_iter iterator ((GVariant*) nullptr); }));
    });

return g_test_run ();
}
