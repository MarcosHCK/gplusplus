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
#include <gio++/asynclib/asynclib.h>
#include <tests/testing.h>
using namespace testing;

static gioplusplus::asynclib::task<int> simple (int value) noexcept
{
co_return value;
}

gioplusplus::asynclib::async_function<g_file_read_async, g_file_read_finish> g_file_read_task;

static gioplusplus::asynclib::task<int> throws_simple (int value) noexcept
{

  boxing::object file = g_file_new_for_commandline_arg ("not_found");
  boxing::object stream = co_await g_file_read_task (file, G_PRIORITY_DEFAULT, NULL);
co_return value;
}

static gioplusplus::asynclib::task<int> throws_and_changes (int value, const gchar* message) noexcept
{

  try
    { boxing::object file = g_file_new_for_commandline_arg ("not_found");
      boxing::object stream = co_await g_file_read_task (file, G_PRIORITY_DEFAULT, NULL); }
  catch (boxing::error error)
    { throw boxing::error::literal (G_IO_ERROR, G_IO_ERROR_FAILED, message); }
co_return value;
}

static gioplusplus::asynclib::task<int> wrapped (int value) noexcept
{
co_return (co_await simple (value) ^ 1);
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/new/simple", []
    {

      auto rand = g_test_rand_int ();
      auto task = simple (rand);
      (void) task;
    });

  g_test_add_action (TESTPATHROOT "/new/wrapped", []
    {

      auto rand = g_test_rand_int ();
      auto task = wrapped (rand);
      (void) task;
    });

  g_test_add_action (TESTPATHROOT "/execute/simple", []
    {

      auto rand = g_test_rand_int ();
      auto task = simple (rand);

      struct D { guint ready; int result; }
        data = { .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gioplusplus::asynclib::task_function_finish<simple> (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (0, ==, rand ^ data.result);
    });

  g_test_add_action (TESTPATHROOT "/execute/throws/simple", []
    {

      auto rand = g_test_rand_int ();
      auto task = throws_simple (rand);

      struct D { guint ready; int result; }
        data = { .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gioplusplus::asynclib::task_function_finish<throws_simple> (async_result, &e);
          g_assert_error (e, G_IO_ERROR, G_IO_ERROR_NOT_FOUND);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);
    });

  g_test_add_action (TESTPATHROOT "/execute/throws/changes", []
    {

      auto rand = g_test_rand_int ();
      auto message = boxing::freeable (g_strdup_printf ("just testing: %i", rand));
      auto task = throws_and_changes (rand, message);

      struct D { boxing::freeable<gchar> message; guint ready; int result; }
        data = { .message = std::move (message), .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gioplusplus::asynclib::task_function_finish<throws_and_changes> (async_result, &e);
          g_assert_error (e, G_IO_ERROR, G_IO_ERROR_FAILED);
          g_assert_cmpstr (e->message, ==, p->message);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);
    });

  g_test_add_action (TESTPATHROOT "/execute/wrapped", []
    {

      auto rand = g_test_rand_int ();
      auto task = wrapped (rand);

      struct D { guint ready; int result; }
        data = { .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gioplusplus::asynclib::task_function_finish<wrapped> (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (1, ==, rand ^ data.result);
    });

return g_test_run ();
}