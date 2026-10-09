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
#include <chrono>
#include <limits>
#include <utility>
#include <gplusplus/asynclib/asynclib.h>
#include <tests/testing.h>
using namespace testing;

using namespace std::chrono_literals;

struct delay_result
{
  GMainLoop* loop;
  GMainContext* context;
  guint callbacks;
  bool value;
  bool callback_context_owned;
  bool callback_was_inline;
  bool timed_out;
  GQuark error_domain;
  int error_code;
  int priority;
};

static void delay_ready (GObject*, GAsyncResult* async_result, gpointer user_data)
{

  auto result = (delay_result*) user_data;
  GError* error = nullptr;

  result->value = gpp_asynclib_async_delay_finish (async_result, &error);
  result->callbacks++;
  result->callback_context_owned = g_main_context_is_owner (result->context);
  result->priority = g_task_get_priority ((GTask*) async_result);

  if (nullptr != error)
    {
      result->error_domain = error->domain;
      result->error_code = error->code;
      g_error_free (error);
    }

  g_main_loop_quit (result->loop);
}

static gboolean watchdog_ready (gpointer user_data)
{

  auto result = (delay_result*) user_data;

  result->timed_out = true;
  g_main_loop_quit (result->loop);
return G_SOURCE_REMOVE;
}

static gboolean cancel_ready (gpointer user_data)
{
  g_cancellable_cancel ((GCancellable*) user_data);
return G_SOURCE_REMOVE;
}

template<gplusplus::asynclib::details::time_like T>
static delay_result wait_delay (T interval,
                                GCancellable* cancellable = nullptr,
                                GMainContext* expected_context = nullptr,
                                guint cancel_after_ms = 0,
                                guint watchdog_ms = 2000,
                                int priority = G_PRIORITY_DEFAULT)
{

  GMainContext* context = expected_context;

  if (nullptr == context)
    {
      context = g_main_context_get_thread_default ();

      if (nullptr == context)
        context = g_main_context_default ();
    }

  delay_result result =
    {
      .loop = g_main_loop_new (context, FALSE),
      .context = context,
      .callbacks = 0,
      .value = false,
      .callback_context_owned = false,
      .callback_was_inline = false,
      .timed_out = false,
      .error_domain = 0,
      .error_code = 0,
      .priority = G_PRIORITY_DEFAULT,
    };

  GSource* watchdog = g_timeout_source_new (watchdog_ms);
  g_source_set_callback (watchdog, watchdog_ready, &result, nullptr);
  g_source_attach (watchdog, context);

  auto task = gplusplus::asynclib::delay (interval, priority, cancellable);
  task.begin (delay_ready, &result);

  result.callback_was_inline = (0 != result.callbacks);
  GSource* cancel_source = nullptr;

  if (0 != cancel_after_ms)
    {
      cancel_source = g_timeout_source_new (cancel_after_ms);
      g_source_set_callback (cancel_source, cancel_ready, cancellable, nullptr);
      g_source_attach (cancel_source, context);
    }

  if (0 == result.callbacks)
    g_main_loop_run (result.loop);

  if (nullptr != cancel_source)
    {
      g_source_destroy (cancel_source);
      g_source_unref (cancel_source);
    }

  g_source_destroy (watchdog);
  g_source_unref (watchdog);
  g_main_loop_unref (result.loop);

return result;
}

static void test_basic_delay ()
{

  const auto started = std::chrono::steady_clock::now ();
  const auto result = wait_delay (20ms);
  const auto elapsed = std::chrono::steady_clock::now () - started;

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_true (result.value);
  g_assert_cmpuint (result.error_domain, ==, 0);
  g_assert_true (result.callback_context_owned);
  g_assert_cmpint (result.priority, ==, G_PRIORITY_DEFAULT);
  /* Assert only a tolerant lower bound; GLib may dispatch late under load. */
  g_assert_cmpint (std::chrono::duration_cast<std::chrono::milliseconds> (elapsed).count (), >=, 10);
}

static void test_zero_negative_and_fractional ()
{

  auto zero = gplusplus::asynclib::details::normalize_delay (0ms);
  auto negative = gplusplus::asynclib::details::normalize_delay (-1ms);
  auto fractional = gplusplus::asynclib::details::normalize_delay (std::chrono::duration<double, std::milli> (0.25));
  auto fractional_over = gplusplus::asynclib::details::normalize_delay (std::chrono::duration<double, std::milli> (1.25));

  g_assert_false (zero.first);
  g_assert_cmpuint (zero.second, ==, 0);
  g_assert_false (negative.first);
  g_assert_cmpuint (negative.second, ==, 0);
  g_assert_true (fractional.first);
  g_assert_cmpuint (fractional.second, ==, 1);
  g_assert_true (fractional_over.first);
  g_assert_cmpuint (fractional_over.second, ==, 2);

  const auto fractional_result = wait_delay (std::chrono::duration<double, std::milli> (0.25));

  for (const auto* result: { &fractional_result })
    {
      g_assert_false (result->timed_out);
      g_assert_false (result->callback_was_inline);
      g_assert_cmpuint (result->callbacks, ==, 1);
      g_assert_true (result->value);
      g_assert_cmpuint (result->error_domain, ==, 0);
    }
}

static void test_priority ()
{

  const auto result = wait_delay (2ms, nullptr, nullptr, 0, 2000, G_PRIORITY_LOW);

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_true (result.value);
  g_assert_cmpint (result.priority, ==, G_PRIORITY_LOW);
}

static void test_duration_bounds ()
{

  using wide_duration = std::chrono::duration<long double, std::milli>;
  const long double maximum = static_cast<long double> (std::numeric_limits<guint>::max ());

  auto at_limit = gplusplus::asynclib::details::normalize_delay (wide_duration (maximum));
  auto over_limit = gplusplus::asynclib::details::normalize_delay (wide_duration (maximum + 1.0L));
  auto integer_over_limit = gplusplus::asynclib::details::normalize_delay (
    std::chrono::duration<unsigned long long> (std::numeric_limits<unsigned long long>::max ()));
  auto nan = gplusplus::asynclib::details::normalize_delay (wide_duration (std::numeric_limits<long double>::quiet_NaN ()));
  auto infinity = gplusplus::asynclib::details::normalize_delay (wide_duration (std::numeric_limits<long double>::infinity ()));

  g_assert_true (at_limit.first);
  g_assert_cmpuint (at_limit.second, ==, std::numeric_limits<guint>::max ());
  g_assert_false (over_limit.first);
  g_assert_false (integer_over_limit.first);
  g_assert_false (nan.first);
  g_assert_false (infinity.first);

}

static void assert_invalid_delay (std::chrono::duration<long double, std::milli> interval)
{

  const auto result = wait_delay (interval);

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_false (result.value);
  g_assert_cmpuint (result.error_domain, ==, G_IO_ERROR);
  g_assert_cmpint (result.error_code, ==, G_IO_ERROR_INVALID_ARGUMENT);
}

static gplusplus::asynclib::task<int> coroutine_waits_for_delay ()
{
  co_await gplusplus::asynclib::delay (2ms);
co_return 42;
}

static void test_invalid_delays ()
{

  using wide_duration = std::chrono::duration<long double, std::milli>;
  const long double maximum = static_cast<long double> (std::numeric_limits<guint>::max ());

  assert_invalid_delay (wide_duration (maximum + 1.0L));
  assert_invalid_delay (wide_duration (std::numeric_limits<long double>::quiet_NaN ()));
  assert_invalid_delay (wide_duration (std::numeric_limits<long double>::infinity ()));
}

static void test_cancellation ()
{

  GCancellable* cancellable = g_cancellable_new ();
  const auto result = wait_delay (5s, cancellable, nullptr, 10, 1500);
  g_object_unref (cancellable);

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_false (result.value);
  g_assert_cmpuint (result.error_domain, ==, G_IO_ERROR);
  g_assert_cmpint (result.error_code, ==, G_IO_ERROR_CANCELLED);
}

static void test_timeout_cancel_race ()
{

  for (unsigned i = 0; i < 20; ++i)
    {
      GCancellable* cancellable = g_cancellable_new ();
      const auto result = wait_delay (1ms, cancellable, nullptr, 1, 1000);
      g_object_unref (cancellable);

      g_assert_false (result.timed_out);
      g_assert_cmpuint (result.callbacks, ==, 1);
      g_assert_true ((result.value && 0 == result.error_domain)
                  || (! result.value && G_IO_ERROR == result.error_domain
                                   && G_IO_ERROR_CANCELLED == result.error_code));
    }
}

static void test_already_cancelled ()
{

  GCancellable* cancellable = g_cancellable_new ();
  g_cancellable_cancel (cancellable);
  const auto result = wait_delay (5s, cancellable, nullptr, 0, 1500);
  g_object_unref (cancellable);

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_false (result.value);
  g_assert_cmpuint (result.error_domain, ==, G_IO_ERROR);
  g_assert_cmpint (result.error_code, ==, G_IO_ERROR_CANCELLED);
}

static void test_custom_context ()
{

  GMainContext* context = g_main_context_new ();
  g_main_context_push_thread_default (context);
  const auto result = wait_delay (2ms, nullptr, context);
  g_main_context_pop_thread_default (context);
  g_main_context_unref (context);

  g_assert_false (result.timed_out);
  g_assert_false (result.callback_was_inline);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_true (result.value);
  g_assert_true (result.callback_context_owned);
}

static void test_coroutine_await ()
{

  GMainContext* context = g_main_context_get_thread_default ();

  if (nullptr == context)
    context = g_main_context_default ();

  GMainLoop* loop = g_main_loop_new (context, FALSE);
  struct result_type
    {
      GMainLoop* loop;
      guint callbacks = 0;
      bool timed_out = false;
      int value = 0;
      GQuark error_domain = 0;
      int error_code = 0;
    } result = { .loop = loop };

  GSource* watchdog = g_timeout_source_new (1500);
  g_source_set_callback (watchdog, [] (gpointer user_data) -> gboolean
    {
      auto values = *(std::pair<GMainLoop*, result_type*>*) user_data;
      values.second->timed_out = true;
      g_main_loop_quit (values.first);
    return G_SOURCE_REMOVE;
    }, new std::pair<GMainLoop*, result_type*> (loop, &result), [] (gpointer data)
      { delete (std::pair<GMainLoop*, result_type*>*) data; });
  g_source_attach (watchdog, context);

  auto task = coroutine_waits_for_delay ();
  task.begin ([] (GObject*, GAsyncResult* async_result, gpointer user_data)
    {
      auto result = (result_type*) user_data;
      GError* error = nullptr;
      result->value = gplusplus::asynclib::task_function_finish<coroutine_waits_for_delay> (async_result, &error);
      result->callbacks++;

      if (nullptr != error)
        {
          result->error_domain = error->domain;
          result->error_code = error->code;
          g_error_free (error);
        }

      g_main_loop_quit (result->loop);
    }, &result);

  g_main_loop_run (loop);
  g_source_destroy (watchdog);
  g_source_unref (watchdog);
  g_main_loop_unref (loop);

  g_assert_false (result.timed_out);
  g_assert_cmpuint (result.callbacks, ==, 1);
  g_assert_cmpint (result.value, ==, 42);
  g_assert_cmpuint (result.error_domain, ==, 0);
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, nullptr);

  g_test_add_action (TESTPATHROOT "/basic", [] { test_basic_delay (); });
  g_test_add_action (TESTPATHROOT "/priority", [] { test_priority (); });
  g_test_add_action (TESTPATHROOT "/duration/zero-negative-fractional", [] { test_zero_negative_and_fractional (); });
  g_test_add_action (TESTPATHROOT "/duration/bounds", [] { test_duration_bounds (); });
  g_test_add_action (TESTPATHROOT "/duration/invalid", [] { test_invalid_delays (); });
  g_test_add_action (TESTPATHROOT "/cancellation/pending", [] { test_cancellation (); });
  g_test_add_action (TESTPATHROOT "/cancellation/timeout-race", [] { test_timeout_cancel_race (); });
  g_test_add_action (TESTPATHROOT "/cancellation/already-cancelled", [] { test_already_cancelled (); });
  g_test_add_action (TESTPATHROOT "/context/custom", [] { test_custom_context (); });
  g_test_add_action (TESTPATHROOT "/coroutine/await", [] { test_coroutine_await (); });

return g_test_run ();
}