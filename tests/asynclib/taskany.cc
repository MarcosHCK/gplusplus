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
#include <gplusplus/asynclib/asynclib.h>
#include <tests/testing.h>
#include <atomic>
#include <optional>
#include <stdexcept>
#include <thread>
#include <variant>
#include <vector>
using namespace testing;

namespace
{

  using nul_tag = gplusplus::asynclib::details::async_task_any_nul_tag;

  struct controlled_tasks
    {

      struct operation
        {

          GTask* task;
          GAsyncReadyCallback callback;
          gpointer user_data;
          bool returns_void;
          int value;

          operation (GTask* task_, GAsyncReadyCallback callback_, gpointer user_data_, bool returns_void_, int value_) noexcept:
              task (task_), callback (callback_), user_data (user_data_), returns_void (returns_void_), value (value_)
            { }

          operation (operation&& other) noexcept:
              task (std::exchange (other.task, nullptr)), callback (other.callback), user_data (other.user_data), returns_void (other.returns_void), value (other.value)
            { }

          operation (const operation&) = delete;
          operation& operator= (const operation&) = delete;
          operation& operator= (operation&&) = delete;

          ~operation ()
            { g_clear_object (&task); }
        };

      std::vector<operation> operations;

      void start (GAsyncReadyCallback callback, gpointer user_data, bool returns_void, int value)
        {

          auto child = g_task_new (NULL, NULL, NULL, NULL);
          try
            { operations.emplace_back (child, callback, user_data, returns_void, value); }
          catch (...)
            { g_object_unref (child); throw; }
        }

      void complete (unsigned index, bool fail = false)
        {

          auto& op = operations.at (index);
          g_assert_nonnull (op.task);
          auto child = std::exchange (op.task, nullptr);

          if (fail)
            g_task_return_new_error (child, G_IO_ERROR, G_IO_ERROR_FAILED, "controlled child failure");
          else if (op.returns_void)
            g_task_return_boolean (child, TRUE);
          else
            g_task_return_int (child, op.value);

          op.callback (NULL, G_ASYNC_RESULT (child), op.user_data);
          g_object_unref (child);
        }
    };

  template<typename Return,
           bool ThrowsOnStart = false,
           bool ThrowsOnFinish = false>
  struct controlled_task
    {

      struct begin_details { };
      struct end_details { using return_type = Return; };

      controlled_tasks* tasks;
      int value;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        {

          if constexpr (ThrowsOnStart)
            throw std::runtime_error ("controlled child start failure");
          else
            tasks->start (callback, user_data, std::same_as<void, Return>, value);
        }

      static Return finish (GObject*, GAsyncResult* async_result, GError** error) noexcept (!ThrowsOnFinish)
        {

          if constexpr (ThrowsOnFinish)
            throw std::runtime_error ("controlled child finish failure");
          else if constexpr (std::same_as<void, Return>)
            g_task_propagate_boolean (G_TASK (async_result), error);
          else
            return static_cast<Return> (g_task_propagate_int (G_TASK (async_result), error));
        }
    };

  struct synchronous_tasks
    {
      unsigned starts = 0;
    };

  struct synchronous_value_task
    {

      struct begin_details { };
      struct end_details { using return_type = int; };

      synchronous_tasks* tasks;
      int value;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        {

          ++tasks->starts;
          auto child = g_task_new (NULL, NULL, NULL, NULL);
          g_task_return_int (child, value);
          callback (NULL, G_ASYNC_RESULT (child), user_data);
          g_object_unref (child);
        }

      static int finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        { return g_task_propagate_int (G_TASK (async_result), error); }
    };

  struct tracked_result
    {

      inline static guint releases = 0;

      int value = 0;
      bool owns_release = false;

      tracked_result () noexcept = default;

      explicit tracked_result (int value_) noexcept:
          value (value_), owns_release (true)
        { }

      tracked_result (tracked_result&& other) noexcept:
          value (other.value), owns_release (std::exchange (other.owns_release, false))
        { }

      tracked_result& operator= (tracked_result&& other) noexcept
        {

          if (this != &other)
            {
              if (owns_release)
                ++releases;
              value = other.value;
              owns_release = std::exchange (other.owns_release, false);
            }
        return *this;
        }

      tracked_result (const tracked_result&) = delete;
      tracked_result& operator= (const tracked_result&) = delete;

      ~tracked_result ()
        {
          if (owns_release)
            ++releases;
        }
    };

  struct tracked_value_task
    {

      struct begin_details { };
      struct end_details { using return_type = tracked_result; };

      controlled_tasks* tasks;
      int value;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        { tasks->start (callback, user_data, false, value); }

      static tracked_result finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        {

          auto value = g_task_propagate_int (G_TASK (async_result), error);
          if (error && *error)
            return { };
        return tracked_result (value);
        }
    };

  struct potentially_throwing_move_result
    {

      int value = 0;

      potentially_throwing_move_result () noexcept = default;
      explicit potentially_throwing_move_result (int value_) noexcept: value (value_) { }
      potentially_throwing_move_result (potentially_throwing_move_result&& other) noexcept (false): value (other.value) { }
      potentially_throwing_move_result& operator= (potentially_throwing_move_result&&) noexcept (false) = default;
      potentially_throwing_move_result (const potentially_throwing_move_result&) = delete;
      potentially_throwing_move_result& operator= (const potentially_throwing_move_result&) = delete;
    };

  struct potentially_throwing_move_task
    {

      struct begin_details { };
      struct end_details { using return_type = potentially_throwing_move_result; };

      controlled_tasks* tasks;
      int value;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        { tasks->start (callback, user_data, false, value); }

      static potentially_throwing_move_result finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        {

          auto value = g_task_propagate_int (G_TASK (async_result), error);
          if (error && *error)
            return { };
        return potentially_throwing_move_result (value);
        }
    };

  struct throwing_move_result
    {

      int value = 0;

      throwing_move_result () noexcept = default;
      explicit throwing_move_result (int value_) noexcept: value (value_) { }
      throwing_move_result (throwing_move_result&& other) noexcept (false): value (other.value) { }
      throwing_move_result& operator= (throwing_move_result&&) = default;
      throwing_move_result (const throwing_move_result&) = delete;
      throwing_move_result& operator= (const throwing_move_result&) = delete;
    };

  struct throwing_move_task
    {

      struct begin_details { };
      struct end_details { using return_type = throwing_move_result; };

      controlled_tasks* tasks;
      int value;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        { tasks->start (callback, user_data, false, value); }

      static throwing_move_result finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        {

          auto value = g_task_propagate_int (G_TASK (async_result), error);
          if (error && *error)
            return { };
        return throwing_move_result (value);
        }
    };

  using value_result = gplusplus::asynclib::details::async_task_any_return<controlled_task<int>, controlled_task<long>>;
  using repeated_value_result = gplusplus::asynclib::details::async_task_any_return<controlled_task<int>, controlled_task<int>>;
  using void_result = gplusplus::asynclib::details::async_task_any_return<controlled_task<void>, controlled_task<void>>;
  using synchronous_result = gplusplus::asynclib::details::async_task_any_return<synchronous_value_task, synchronous_value_task>;
  using tracked_result_type = gplusplus::asynclib::details::async_task_any_return<tracked_value_task>;
  using potentially_throwing_move_result_type = gplusplus::asynclib::details::async_task_any_return<potentially_throwing_move_task>;
  using throwing_move_result_type = gplusplus::asynclib::details::async_task_any_return<throwing_move_task>;

  template<typename... Tasks>
  concept supported_any = requires { typename gplusplus::asynclib::details::async_task_any_return<Tasks...>; }
                       && requires { gplusplus::asynclib::any (std::declval<Tasks> () ...); };

  static_assert (std::same_as<value_result, std::variant<nul_tag, int, long>>);
  static_assert (std::same_as<repeated_value_result, std::variant<nul_tag, int>>);
  static_assert (std::same_as<void_result, bool>);
  static_assert (supported_any<controlled_task<int>, controlled_task<long>>);
  static_assert (supported_any<controlled_task<void>, controlled_task<void>>);
  static_assert (! supported_any<controlled_task<int>, controlled_task<void>>);
  static_assert (! supported_any<>);
  template<typename Result>
  struct completion
    {
      gint ready = 0;
      guint callbacks = 0;
      Result result { };
    };

  template<auto Function, typename Fixture>
  static auto start_task (Fixture& fixture, completion<typename decltype (Function (fixture))::end_details::return_type>& completed)
    {

      auto task = Function (fixture);
      using Result = typename decltype (task)::end_details::return_type;

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {

          auto& completed = *(completion<Result>*) user_data;
          GError* error = nullptr;
          completed.result = gplusplus::asynclib::task_function_finish<Function> (async_result, &error);
          g_assert_no_error (error);
          ++completed.callbacks;
          g_atomic_int_set (&completed.ready, 1);
        }, &completed);
    return task;
    }

  template<typename Result>
  static void wait_for_completion (completion<Result>& completed)
    {
      while (0 == g_atomic_int_get (&completed.ready))
        g_main_context_iteration (g_main_context_get_thread_default (), TRUE);
    }

  template<typename Result>
  static void assert_value (const Result& result, int expected)
    {
      g_assert_true (std::holds_alternative<int> (result));
      g_assert_cmpint (std::get<int> (result), ==, expected);
    }

  template<typename Result>
  static void assert_failure_sentinel (const Result& result)
    {
      g_assert_true (std::holds_alternative<nul_tag> (result));
    }

  static gplusplus::asynclib::task<value_result> first_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int> { &tasks, 17 }, controlled_task<long> { &tasks, 29 });
    }

  static gplusplus::asynclib::task<std::variant<nul_tag, int>> one_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int> { &tasks, 23 });
    }

  static gplusplus::asynclib::task<value_result> failure_then_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int> { &tasks, 17 }, controlled_task<long> { &tasks, 29 });
    }

  static gplusplus::asynclib::task<value_result> all_value_failures (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int> { &tasks, 17 }, controlled_task<long> { &tasks, 29 });
    }

  static gplusplus::asynclib::task<value_result> start_failure_then_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int, true> { &tasks, 17 }, controlled_task<long> { &tasks, 29 });
    }

  static gplusplus::asynclib::task<value_result> finish_failure_then_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<int, false, true> { &tasks, 17 }, controlled_task<long> { &tasks, 29 });
    }

  static gplusplus::asynclib::task<void_result> first_void (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<void> { &tasks, 0 }, controlled_task<void> { &tasks, 0 });
    }

  static gplusplus::asynclib::task<bool> one_void (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<void> { &tasks, 0 });
    }

  static gplusplus::asynclib::task<void_result> all_void_failures (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (controlled_task<void> { &tasks, 0 }, controlled_task<void> { &tasks, 0 });
    }

  static gplusplus::asynclib::task<synchronous_result> synchronous_values (synchronous_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (synchronous_value_task { &tasks, 5 }, synchronous_value_task { &tasks, 8 });
    }

  static gplusplus::asynclib::task<tracked_result_type> tracked_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (tracked_value_task { &tasks, 73 });
    }

  static gplusplus::asynclib::task<potentially_throwing_move_result_type> potentially_throwing_move_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (potentially_throwing_move_task { &tasks, 97 });
    }

  static gplusplus::asynclib::task<throwing_move_result_type> throwing_move_value (controlled_tasks& tasks) noexcept
    {
      co_return co_await gplusplus::asynclib::any (throwing_move_task { &tasks, 91 });
    }
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/any/single/value", []
    {

      controlled_tasks tasks;
      completion<std::variant<nul_tag, int>> completed;
      auto task = start_task<&one_value> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);
      assert_value (completed.result, 23);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/winner/void", []
    {

      controlled_tasks tasks;
      completion<bool> completed;
      auto task = start_task<&one_void> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);

      g_assert_true (completed.result);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/winner/value", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&first_value> (tasks, completed);

      tasks.complete (0);
      wait_for_completion (completed);
      assert_value (completed.result, 17);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 1);

      tasks.complete (1, true);
      while (g_main_context_pending (g_main_context_get_thread_default ()))
        g_main_context_iteration (g_main_context_get_thread_default (), FALSE);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/winner/concurrent", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&first_value> (tasks, completed);
      std::atomic<bool> start = false;

      auto complete_child = [&] (unsigned index)
        {
          while (! start.load (std::memory_order_acquire))
            std::this_thread::yield ();
          tasks.complete (index);
        };

      std::thread first (complete_child, 0);
      std::thread second (complete_child, 1);
      start.store (true, std::memory_order_release);
      first.join ();
      second.join ();
      wait_for_completion (completed);

      const auto& winner = completed.result;
      g_assert_true (std::holds_alternative<int> (winner) || std::holds_alternative<long> (winner));
      if (std::holds_alternative<int> (winner))
        g_assert_cmpint (std::get<int> (winner), ==, 17);
      else
        g_assert_cmpint (std::get<long> (winner), ==, 29);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/failure-then-value", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&failure_then_value> (tasks, completed);

      tasks.complete (0, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);
      g_assert_true (std::holds_alternative<long> (completed.result));
      g_assert_cmpint (std::get<long> (completed.result), ==, 29);
    });

  g_test_add_action (TESTPATHROOT "/any/value-order", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&first_value> (tasks, completed);

      tasks.complete (1);
      wait_for_completion (completed);
      g_assert_true (std::holds_alternative<long> (completed.result));
      g_assert_cmpint (std::get<long> (completed.result), ==, 29);
      tasks.complete (0, true);
      while (g_main_context_pending (g_main_context_get_thread_default ()))
        g_main_context_iteration (g_main_context_get_thread_default (), FALSE);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/all-value-failures", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&all_value_failures> (tasks, completed);

      tasks.complete (0, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1, true);
      wait_for_completion (completed);
      assert_failure_sentinel (completed.result);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/single/void", []
    {

      controlled_tasks tasks;
      completion<void_result> completed;
      auto task = start_task<&first_void> (tasks, completed);

      tasks.complete (1, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      wait_for_completion (completed);
      g_assert_true (completed.result);
    });

  g_test_add_action (TESTPATHROOT "/any/all-void-failures", []
    {

      controlled_tasks tasks;
      completion<void_result> completed;
      auto task = start_task<&all_void_failures> (tasks, completed);

      tasks.complete (0, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1, true);
      wait_for_completion (completed);
      g_assert_false (completed.result);
    });

  g_test_add_action (TESTPATHROOT "/any/start-exception", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&start_failure_then_value> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);
      g_assert_true (std::holds_alternative<long> (completed.result));
      g_assert_cmpint (std::get<long> (completed.result), ==, 29);
    });

  g_test_add_action (TESTPATHROOT "/any/finish-exception", []
    {

      controlled_tasks tasks;
      completion<value_result> completed;
      auto task = start_task<&finish_failure_then_value> (tasks, completed);

      tasks.complete (0);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);
      g_assert_true (std::holds_alternative<long> (completed.result));
      g_assert_cmpint (std::get<long> (completed.result), ==, 29);
    });

  g_test_add_action (TESTPATHROOT "/any/synchronous-completion", []
    {

      synchronous_tasks tasks;
      completion<synchronous_result> completed;
      auto task = start_task<&synchronous_values> (tasks, completed);

      g_assert_cmpuint (tasks.starts, ==, 2);
      wait_for_completion (completed);
      assert_value (completed.result, 5);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/lifetime/move-only-value", []
    {

      controlled_tasks tasks;
      completion<tracked_result_type> completed;
      tracked_result::releases = 0;

      {
        auto task = start_task<&tracked_value> (tasks, completed);
        tasks.complete (0);
        wait_for_completion (completed);

        g_assert_true (std::holds_alternative<tracked_result> (completed.result));
        g_assert_cmpint (std::get<tracked_result> (completed.result).value, ==, 73);
        completed.result = { };
      }

      g_assert_cmpuint (tracked_result::releases, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/lifetime/throwing-move-value", []
    {

      controlled_tasks tasks;
      completion<throwing_move_result_type> completed;
      auto task = start_task<&throwing_move_value> (tasks, completed);

      tasks.complete (0);
      wait_for_completion (completed);

      g_assert_true (std::holds_alternative<throwing_move_result> (completed.result));
      g_assert_cmpint (std::get<throwing_move_result> (completed.result).value, ==, 91);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  g_test_add_action (TESTPATHROOT "/any/lifetime/potentially-throwing-move", []
    {

      controlled_tasks tasks;
      completion<potentially_throwing_move_result_type> completed;
      auto task = start_task<&potentially_throwing_move_value> (tasks, completed);

      tasks.complete (0);
      wait_for_completion (completed);
      g_assert_true (std::holds_alternative<potentially_throwing_move_result> (completed.result));
      g_assert_cmpint (std::get<potentially_throwing_move_result> (completed.result).value, ==, 97);
      g_assert_cmpuint (completed.callbacks, ==, 1);
    });

  return g_test_run ();
}