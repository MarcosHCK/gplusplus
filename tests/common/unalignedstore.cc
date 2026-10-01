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
#include <cstddef>
#include <cstdint>
#include <gio++/common/unalignedstore.h>
#include <tests/testing.h>
using namespace testing;

namespace
{
  struct alignas (64) tracked_type
    {
      static inline int ctor_calls = 0;
      static inline int copy_calls = 0;
      static inline int move_calls = 0;
      static inline int dtor_calls = 0;

      int value;

      inline explicit tracked_type (int value_): value (value_)
        { ++ctor_calls; }

      inline tracked_type (const tracked_type& other): value (other.value)
        { ++copy_calls; }

      inline tracked_type (tracked_type&& other) noexcept: value (other.value)
        { ++move_calls; other.value = -1; }

      inline ~tracked_type ()
        { ++dtor_calls; }
    };

  static_assert (alignof (unaligned_store<tracked_type>) == 1);
  static_assert (alignof (tracked_type) == 64);

  static void reset_counts ()
    {
      tracked_type::ctor_calls = 0;
      tracked_type::copy_calls = 0;
      tracked_type::move_calls = 0;
      tracked_type::dtor_calls = 0;
    }
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/aligned_at_arbitrary_offsets", []
    {

      reset_counts ();

      alignas (64) std::byte buffer [64 * 2 + sizeof (unaligned_store<tracked_type>)];
      auto base = reinterpret_cast<std::uintptr_t> (buffer);

      for (std::size_t offset = 0; offset < 64; ++offset)
        {

          auto* storage = reinterpret_cast<unaligned_store<tracked_type>*> (buffer + offset);
          auto* value = std::construct_at (storage, static_cast<int> (offset));

          g_assert_cmpuint (reinterpret_cast<std::uintptr_t> (&**value) % alignof (tracked_type), ==, 0);
          g_assert_cmpint (static_cast<int> (offset), ==, (**value).value);
          (**value).value = static_cast<int> (offset + 1);
          g_assert_cmpint (static_cast<int> (offset + 1), ==, static_cast<tracked_type&> (**value).value);

          std::destroy_at (value);
          g_assert_cmpint (static_cast<int> (offset + 1), ==, tracked_type::ctor_calls + tracked_type::copy_calls + tracked_type::move_calls);
          g_assert_cmpint (static_cast<int> (offset + 1), ==, tracked_type::dtor_calls);
          g_assert_cmpuint (reinterpret_cast<std::uintptr_t> (buffer + offset) - base, ==, offset);
        }
    });

  g_test_add_action (TESTPATHROOT "/copy_and_move", []
    {

      reset_counts ();

      {
        unaligned_store<tracked_type> source (42);
        unaligned_store<tracked_type> copied (source);
        unaligned_store<tracked_type> moved (std::move (source));

        g_assert_cmpint (-1, ==, (*source).value);
        g_assert_cmpint (42, ==, (*copied).value);
        g_assert_cmpint (42, ==, (*moved).value);
        g_assert_cmpint (1, ==, tracked_type::ctor_calls);
        g_assert_cmpint (1, ==, tracked_type::copy_calls);
        g_assert_cmpint (1, ==, tracked_type::move_calls);

        g_assert_cmpuint (reinterpret_cast<std::uintptr_t> (&*source) % alignof (tracked_type), ==, 0);
        g_assert_cmpuint (reinterpret_cast<std::uintptr_t> (&*copied) % alignof (tracked_type), ==, 0);
        g_assert_cmpuint (reinterpret_cast<std::uintptr_t> (&*moved) % alignof (tracked_type), ==, 0);

        const auto& const_source = source;
        const tracked_type& const_value = const_source;
        g_assert_cmpint (-1, ==, const_value.value);
      }

      /* Each constructed object, including the moved-from source, is destroyed
       * exactly once. */
      g_assert_cmpint (3, ==, tracked_type::dtor_calls);
    });

  g_test_add_action (TESTPATHROOT "/destruction", []
    {

      reset_counts ();

      {
        unaligned_store<tracked_type> value (7);
        g_assert_cmpint (0, ==, tracked_type::dtor_calls);
      }

      g_assert_cmpint (1, ==, tracked_type::ctor_calls);
      g_assert_cmpint (1, ==, tracked_type::dtor_calls);
    });

  g_test_add_action (TESTPATHROOT "/ordinary_alignment", []
    {

      unaligned_store<int> value (17);
      g_assert_cmpint (17, ==, *value);

      *value = 29;
      g_assert_cmpint (29, ==, static_cast<const int&> (value));
    });

return g_test_run ();
}