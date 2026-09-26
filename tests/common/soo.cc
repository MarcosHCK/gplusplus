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
#include <cstring>
#include <gio++/common/slice.h>
#include <gio++/common/soo.h>
#include <stdexcept>
#include <string>
#include <tests/testing.h>
using namespace testing;

namespace
{
  struct small_type
    {
      int value;
      inline explicit small_type (int value_): value (value_) { }
    };

  /* fits inside a pointer slot: no allocation involved */
  static_assert (sizeof (small_type) <= sizeof (void*));
  static_assert (alignof (small_type) <= alignof (void*));

  struct large_type
    {
      guchar payload [32];
      int value;
      inline explicit large_type (guchar tag): value (tag)
        { std::memset (payload, tag, sizeof (payload)); }
    };

  static_assert (sizeof (large_type) > sizeof (void*));

  static int g_alloc_calls = 0;
  static int g_free_calls = 0;

  static void* counting_alloc (size_t bytes) noexcept
    { ++g_alloc_calls; return g_slice_alloc (bytes); }

  static void counting_free (size_t bytes, void* location) noexcept
    { ++g_free_calls; g_slice_free1 (bytes, location); }

  struct throwing_type
    {
      static inline bool should_throw = false;
      static inline int dtor_calls = 0;
      guchar padding [32];
      int value;

      inline explicit throwing_type (int value_): value (value_)
        {
          if (G_UNLIKELY (should_throw))
            throw std::runtime_error ("throwing_type constructor");
        }

      inline ~throwing_type ()
        { ++dtor_calls; }
    };

  static_assert (sizeof (throwing_type) > sizeof (void*));

  struct sliced_type
    {
      static inline int dtor_calls = 0;
      static inline bool should_throw = false;
      std::string text;

      inline explicit sliced_type (std::string text_): text (std::move (text_))
        {
          if (G_UNLIKELY (should_throw))
            throw std::runtime_error ("sliced_type constructor");
        }

      inline ~sliced_type ()
        { ++dtor_calls; }
    };
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/small", []
    {

      void* location = nullptr;
      auto* ptr = soo_ptr::create<small_type> (&location, 42);

      g_assert_cmpint (42, ==, ptr->value);
      g_assert_cmpuint ((guintptr) ptr, ==, (guintptr) &location);

      /* in-place mutation through cast<> */
      ((small_type*) soo_ptr::cast<small_type> (&location))->value = 24;
      g_assert_cmpint (24, ==, ptr->value);

      soo_ptr::destroy<small_type> (&location);
    });

  g_test_add_action (TESTPATHROOT "/large", []
    {

      g_alloc_calls = g_free_calls = 0;

      void* location = nullptr;
      auto* ptr = soo_ptr::create<large_type, counting_alloc, counting_free> (&location, 7);

      g_assert_cmpint (1, ==, g_alloc_calls);
      g_assert_cmpint (0, ==, g_free_calls);
      g_assert_cmpint (7, ==, ptr->value);
      for (guint i = 0; i < sizeof (large_type::payload); ++i)
        g_assert_cmpuint (ptr->payload [i], ==, 7);

      /* heap-backed: the slot holds the allocation, not the object */
      g_assert_cmpuint ((guintptr) location, !=, (guintptr) &location);
      g_assert_true (ptr == soo_ptr::cast<large_type> (&location));

      soo_ptr::destroy<large_type, counting_free> (&location);
      g_assert_cmpint (1, ==, g_free_calls);
    });

  g_test_add_action (TESTPATHROOT "/throwing_constructor", []
    {

      /* a throwing constructor must release the just-allocated block rather
       * than leak it, and must not leave a dangling pointer in the slot */
      throwing_type::dtor_calls = 0;
      g_alloc_calls = g_free_calls = 0;

      void* location = nullptr;
      auto* ptr = soo_ptr::create<throwing_type, counting_alloc, counting_free> (&location, 5);
      g_assert_cmpint (5, ==, ptr->value);
      g_assert_cmpint (1, ==, g_alloc_calls);
      g_assert_cmpint (0, ==, g_free_calls);

      /* clean up the successful object first */
      soo_ptr::destroy<throwing_type, counting_free> (&location);
      g_assert_cmpint (1, ==, g_free_calls);
      g_assert_cmpint (1, ==, throwing_type::dtor_calls);

      /* now make the constructor throw: the fresh block must be freed and the
       * slot must be reset, not left pointing at the freed block */
      throwing_type::should_throw = true;
      g_assert_throws (std::runtime_error, ({ soo_ptr::create<throwing_type, counting_alloc, counting_free> (&location, 6); }));
      throwing_type::should_throw = false;

      g_assert_cmpint (2, ==, g_alloc_calls);
      g_assert_cmpint (2, ==, g_free_calls);
      g_assert_cmpint (1, ==, throwing_type::dtor_calls);
      g_assert_true (nullptr == location);
    });

  g_test_add_action (TESTPATHROOT "/slice", []
    {

      sliced_type::dtor_calls = 0;

      auto* obj = g_slice_new_<sliced_type> ("hello from the slice allocator");
      g_assert_cmpstr (obj->text.c_str (), ==, "hello from the slice allocator");

      g_slice_free_<sliced_type> (obj);
      g_assert_cmpint (1, ==, sliced_type::dtor_calls);

      /* a throwing constructor must not leave the slice block behind */
      sliced_type::should_throw = true;
      g_assert_throws (std::runtime_error, ({ g_slice_new_<sliced_type> (std::string ("explode")); }));
      sliced_type::should_throw = false;
    });

return g_test_run ();
}