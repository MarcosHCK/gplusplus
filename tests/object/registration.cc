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
#include <barrier>
#include <config.h>
#include <gplusplus/object/objectimplementor.h>
#include <tests/testing.h>
#include <thread>
using namespace testing;

namespace
{

  struct registration_probe { };
  struct abstract_probe { };
  struct final_probe { };
  struct concurrent_probe { };
}

GPP_IMPLEMENT (registration_probe, registration_probe, { });

GPP_IMPLEMENT_ABSTRACT (abstract_probe, abstract_probe, { });

GPP_IMPLEMENT_FINAL (final_probe, final_probe, { });

GPP_IMPLEMENT (concurrent_probe, concurrent_probe, { });

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/basic", []
    {

      auto type = registration_probe_get_type ();

      g_assert_cmpuint (type, ==, registration_probe_get_type ());
      g_assert_cmpuint (type, ==, g_type_from_name ("registration_probe"));
      g_assert_cmpstr (g_type_name (type), ==, "registration_probe");
      g_assert_cmpuint (g_type_parent (type), ==, G_TYPE_OBJECT);
      g_assert_true (G_TYPE_IS_DERIVED (type));
    });

  g_test_add_action (TESTPATHROOT "/type_tags", []
    {

      auto abstract_type = abstract_probe_get_type ();
      auto final_type = final_probe_get_type ();

      g_assert_true (G_TYPE_IS_ABSTRACT (abstract_type));
      g_assert_false (G_TYPE_IS_FINAL (abstract_type));
      g_assert_true (G_TYPE_IS_FINAL (final_type));
      g_assert_false (G_TYPE_IS_ABSTRACT (final_type));
      g_assert_cmpuint (g_type_parent (abstract_type), ==, G_TYPE_OBJECT);
      g_assert_cmpuint (g_type_parent (final_type), ==, G_TYPE_OBJECT);
    });

  g_test_add_action (TESTPATHROOT "/concurrent_get_type", []
    {

      constexpr auto n_threads = 8;
      std::array<GType, n_threads> types = { };
      std::array<std::thread, n_threads> threads;
      std::barrier start (n_threads);

      for (std::size_t i = 0; i < n_threads; ++i)
        threads [i] = std::thread ([&, i]
          {
            start.arrive_and_wait ();
            types [i] = concurrent_probe_get_type ();
          });

      for (auto& thread: threads)
        thread.join ();

      for (std::size_t i = 1; i < n_threads; ++i)
        g_assert_cmpuint (types [i], ==, types [0]);

      g_assert_cmpstr (g_type_name (types [0]), ==, "concurrent_probe");
    });

return g_test_run ();
}
