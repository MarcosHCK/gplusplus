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
#include <optional>
#include <stdexcept>
#include <tuple>
#include <variant>
#include <vector>
using namespace testing;

namespace
{

  struct controlled_tasks
    {

      struct operation
        {

          GTask* task;
          GAsyncReadyCallback callback;
          gpointer user_data;
          bool returns_void;
          int result;

          operation (GTask* task_, GAsyncReadyCallback callback_, gpointer user_data_, bool returns_void_, int result_) noexcept:
              task (task_), callback (callback_), user_data (user_data_), returns_void (returns_void_), result (result_)
            { }

          operation (operation&& other) noexcept:
              task (std::exchange (other.task, nullptr)), callback (other.callback), user_data (other.user_data), returns_void (other.returns_void), result (other.result)
            { }

          operation (const operation&) = delete;
          operation& operator= (const operation&) = delete;
          operation& operator= (operation&&) = delete;

          ~operation ()
            { g_clear_object (&task); }
        };

      std::vector<operation> operations;

      void start (GAsyncReadyCallback callback, gpointer user_data, bool returns_void, int result)
        {

          auto task = g_task_new (NULL, NULL, NULL, NULL);
          try
            { operations.emplace_back (task, callback, user_data, returns_void, result); }
          catch (...)
            { g_object_unref (task); throw; }
        }

      void complete (unsigned index, bool fail = false)
        {

          auto& op = operations.at (index);
          g_assert_nonnull (op.task);

          auto task = std::exchange (op.task, nullptr);

          if (fail)
            g_task_return_new_error (task, G_IO_ERROR, G_IO_ERROR_FAILED, "controlled child failure");
          else if (op.returns_void)
            g_task_return_boolean (task, TRUE);
          else
            g_task_return_int (task, op.result);

          op.callback (NULL, G_ASYNC_RESULT (task), op.user_data);
          g_object_unref (task);
        }
    };

  struct tracked_result
    {

      inline static guint releases = 0;

      int value = 0;
      bool owns_release = false;

      tracked_result () noexcept = default;

      tracked_result (int value_, bool owns_release_) noexcept:
          value (value_), owns_release (owns_release_)
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

  struct controlled_move_task
    {

      struct begin_details { };
      struct end_details { using return_type = tracked_result; };

      controlled_tasks* tasks;
      int result;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        { tasks->start (callback, user_data, false, result); }

      static tracked_result finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        {

          auto result = g_task_propagate_int (G_TASK (async_result), error);
          if (error && *error)
            return { };
        return tracked_result (result, true);
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
      int result;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        {

          ++tasks->starts;
          auto task = g_task_new (NULL, NULL, NULL, NULL);
          g_task_return_int (task, result);
          callback (NULL, G_ASYNC_RESULT (task), user_data);
          g_object_unref (task);
        }

      static int finish (GObject*, GAsyncResult* async_result, GError** error) noexcept
        { return g_task_propagate_int (G_TASK (async_result), error); }
    };

  template<typename Return,
           bool ThrowsOnStart = false,
           bool ThrowsOnFinish = false>
  struct controlled_task
    {

      struct begin_details { };
      struct end_details { using return_type = Return; };

      controlled_tasks* tasks;
      int result;

      void operator() (GAsyncReadyCallback callback, gpointer user_data) const
        {

          if constexpr (ThrowsOnStart)
            throw std::runtime_error ("controlled child start failure");
          else
            tasks->start (callback, user_data, std::same_as<void, Return>, result);
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

  using value_slot = std::variant<gplusplus::asynclib::details::error, int>;
  using value_result_1 = std::tuple<value_slot>;
  using value_result_2 = std::tuple<value_slot, value_slot>;
  using value_result_3 = std::tuple<value_slot, value_slot, value_slot>;
  using heterogeneous_value_result = std::tuple<value_slot,
                                                std::variant<gplusplus::asynclib::details::error, long>,
                                                std::variant<gplusplus::asynclib::details::error, double>>;
  using tracked_slot = std::variant<gplusplus::asynclib::details::error, tracked_result>;
  using tracked_result_1 = std::tuple<tracked_slot>;
  using void_slot = std::optional<gplusplus::asynclib::details::error>;
  using void_result_1 = std::tuple<void_slot>;
  using void_result_2 = std::tuple<void_slot, void_slot>;
  using void_result_3 = std::tuple<void_slot, void_slot, void_slot>;

  template<typename... Tasks>
  concept supported_task_all = (0 < sizeof... (Tasks))
                            && requires { typename gplusplus::asynclib::details::async_task_all_return<Tasks...>; };

  static_assert (supported_task_all<controlled_task<int>, controlled_task<int>>);
  static_assert (supported_task_all<controlled_task<void>, controlled_task<void>>);
  static_assert (supported_task_all<controlled_task<int>, controlled_task<long>>);
  static_assert (! supported_task_all<controlled_task<void>, controlled_task<int>>);
  static_assert (! supported_task_all<>);

  template<typename Result>
  struct completion
    {

      gint ready = 0;
      Result result { };
    };

  template<auto Function, typename Result>
  static void complete_task (GObject*, GAsyncResult* async_result, gpointer user_data)
    {

      auto& completed = *(completion<Result>*) user_data;
      GError* error = nullptr;

      completed.result = gplusplus::asynclib::task_function_finish<Function> (async_result, &error);
      g_assert_no_error (error);
      g_atomic_int_set (&completed.ready, 1);
    }

  template<auto Function, typename Fixture, typename Result>
  static auto start_task (Fixture& fixture, completion<Result>& completed)
    {

      auto task = Function (fixture);
      task.begin (&complete_task<Function, Result>, &completed);
    return task;
    }

  template<typename Result>
  static void wait_for_completion (completion<Result>& completed)
    {
      while (0 == g_atomic_int_get (&completed.ready))
        g_main_context_iteration (g_main_context_get_thread_default (), TRUE);
    }

  static void assert_value (const value_slot& value, int expected)
    {
      g_assert_true (std::holds_alternative<int> (value));
      g_assert_cmpint (std::get<int> (value), ==, expected);
    }

  static void assert_controlled_failure (const value_slot& value)
    {

      g_assert_true (std::holds_alternative<gplusplus::asynclib::details::error> (value));
      auto error = std::get<gplusplus::asynclib::details::error> (value).get ();
      g_assert_nonnull (error);
      g_assert_error (error, G_IO_ERROR, G_IO_ERROR_FAILED);
      g_assert_cmpstr (error->message, ==, "controlled child failure");
    }

  static void assert_exception_failure (const value_slot& value, const char* message)
    {

      g_assert_true (std::holds_alternative<gplusplus::asynclib::details::error> (value));
      auto error = std::get<gplusplus::asynclib::details::error> (value).get ();
      g_assert_nonnull (error);
      g_assert_cmpstr (error->message, ==, message);
    }

  static void assert_void_success (const void_slot& value)
    { g_assert_false (value.has_value ()); }

  static void assert_void_failure (const void_slot& value)
    {

      g_assert_true (value.has_value ());
      auto error = value->get ();
      g_assert_nonnull (error);
      g_assert_error (error, G_IO_ERROR, G_IO_ERROR_FAILED);
      g_assert_cmpstr (error->message, ==, "controlled child failure");
    }

  static void assert_exception_void_failure (const void_slot& value)
    {

      g_assert_true (value.has_value ());
      auto error = value->get ();
      g_assert_nonnull (error);
      g_assert_cmpstr (error->message, ==, "c++ exception thrown");
    }

  static gplusplus::asynclib::task<value_result_1> collect_one_value (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all (controlled_task<int> { &tasks, 41 });
    co_return result;
    }

  static gplusplus::asynclib::task<value_result_3> collect_synchronous_values (synchronous_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( synchronous_value_task { &tasks, 3 },
                                                        synchronous_value_task { &tasks, 5 },
                                                        synchronous_value_task { &tasks, 8 } );
    co_return result;
    }

  static gplusplus::asynclib::task<void_result_1> collect_one_void (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<void> { &tasks, 0 } );
    co_return result;
    }

  static gplusplus::asynclib::task<value_result_3> collect_values (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<int> { &tasks, 31 },
                                                        controlled_task<int> { &tasks, -7 },
                                                        controlled_task<int> { &tasks, 19 } );
    co_return result;
    }

  static gplusplus::asynclib::task<heterogeneous_value_result> collect_heterogeneous_values (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<int> { &tasks, 12 },
                                                        controlled_task<long> { &tasks, -9000 },
                                                        controlled_task<double> { &tasks, 27 } );
    co_return result;
    }

  static gplusplus::asynclib::task<void_result_3> collect_voids (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<void> { &tasks, 0 },
                                                        controlled_task<void> { &tasks, 0 },
                                                        controlled_task<void> { &tasks, 0 } );
    co_return result;
    }

  static gplusplus::asynclib::task<value_result_3> collect_value_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<int> { &tasks, 31 },
                                                        controlled_task<int> { &tasks, -7 },
                                                        controlled_task<int> { &tasks, 19 } );
    co_return result;
    }

  static gplusplus::asynclib::task<void_result_2> collect_void_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<void> { &tasks, 0 },
                                                        controlled_task<void> { &tasks, 0 } );
    co_return result;
    }

  static gplusplus::asynclib::task<value_result_2> collect_start_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<int, true> { &tasks, 13 },
                                                        controlled_task<int> { &tasks, 29 } );
    co_return result;
    }

  static gplusplus::asynclib::task<value_result_2> collect_finish_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<int, false, true> { &tasks, 13 },
                                                        controlled_task<int> { &tasks, 29 } );
    co_return result;
    }

  static gplusplus::asynclib::task<void_result_2> collect_void_start_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<void, true> { &tasks, 0 },
                                                        controlled_task<void> { &tasks, 0 } );
    co_return result;
    }

  static gplusplus::asynclib::task<void_result_2> collect_void_finish_failure (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_task<void, false, true> { &tasks, 0 },
                                                        controlled_task<void> { &tasks, 0 } );
    co_return result;
    }

  static gplusplus::asynclib::task<tracked_result_1> collect_tracked_value (controlled_tasks& tasks) noexcept
    {
      auto result = co_await gplusplus::asynclib::all ( controlled_move_task { &tasks, 73 } );
    co_return result;
    }
}

static gplusplus::asynclib::task<int> return_value (int value) noexcept
{
  co_return value;
}

static gplusplus::asynclib::task<void> return_void (int& result, int value) noexcept
{
  result = value;
co_return;
}

static gplusplus::asynclib::task<int> value_sum (int value_1, int value_2) noexcept
{

  auto [ a, b ] = co_await gplusplus::asynclib::all ( return_value (value_1), return_value (value_2) );
co_return std::get<int> (a) + std::get<int> (b);
}

static gplusplus::asynclib::task<int> void_sum (int value_1, int value_2) noexcept
{
  int x, y;
  auto [ a, b ] = co_await gplusplus::asynclib::all ( return_void (x, value_1), return_void (y, value_2) );
co_return x + y;
}

int main (int argc, char* argv[])
{

  g_test_init (&argc, &argv, NULL);

  g_test_add_action (TESTPATHROOT "/all/simple/value", []
    {

      auto rand_1 = g_test_rand_int_range (G_MININT16, G_MAXINT16);
      auto rand_2 = g_test_rand_int_range (G_MININT16, G_MAXINT16);
      auto task = value_sum (rand_1, rand_2);

      struct D { guint ready; int result; }
        data = { .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gplusplus::asynclib::task_function_finish<value_sum> (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (data.result, ==, rand_1 + rand_2);
    });

  g_test_add_action (TESTPATHROOT "/all/simple/void", []
    {

      auto rand_1 = g_test_rand_int_range (G_MININT16, G_MAXINT16);
      auto rand_2 = g_test_rand_int_range (G_MININT16, G_MAXINT16);
      auto task = void_sum (rand_1, rand_2);

      struct D { guint ready; int result; }
        data = { .ready = 0, .result = 0, };

      task.begin ([](GObject*, GAsyncResult* async_result, gpointer user_data)
        {
          auto p = (D*) user_data;
          auto e = (GError*) nullptr;

          p->result = gplusplus::asynclib::task_function_finish<void_sum> (async_result, &e);
          g_assert_no_error (e);
          g_atomic_int_set (&p->ready, 1);
        }, &data);

      for (auto main_context = g_main_context_get_thread_default (); 0 == g_atomic_int_get (&data.ready);)
        g_main_context_iteration (main_context, FALSE);

      g_assert_cmpint (data.result, ==, rand_1 + rand_2);
    });

  g_test_add_action (TESTPATHROOT "/all/single/value", []
    {

      controlled_tasks tasks;
      completion<value_result_1> completed;
      auto task = start_task<&collect_one_value> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_value (std::get<0> (completed.result), 41);
    });

  g_test_add_action (TESTPATHROOT "/all/single/void", []
    {

      controlled_tasks tasks;
      completion<void_result_1> completed;
      auto task = start_task<&collect_one_void> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_void_success (std::get<0> (completed.result));
    });

  g_test_add_action (TESTPATHROOT "/all/order/value", []
    {

      controlled_tasks tasks;
      completion<value_result_3> completed;
      auto task = start_task<&collect_values> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 3);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);

      tasks.complete (2);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);

      assert_value (std::get<0> (completed.result), 31);
      assert_value (std::get<1> (completed.result), -7);
      assert_value (std::get<2> (completed.result), 19);
    });

  g_test_add_action (TESTPATHROOT "/all/order/value-types", []
    {

      controlled_tasks tasks;
      completion<heterogeneous_value_result> completed;
      auto task = start_task<&collect_heterogeneous_values> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 3);
      tasks.complete (2);
      tasks.complete (0);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);

      assert_value (std::get<0> (completed.result), 12);
      g_assert_true (std::holds_alternative<long> (std::get<1> (completed.result)));
      g_assert_cmpint (std::get<long> (std::get<1> (completed.result)), ==, -9000);
      g_assert_true (std::holds_alternative<double> (std::get<2> (completed.result)));
      g_assert_cmpfloat (std::get<double> (std::get<2> (completed.result)), ==, 27.0);
    });

  g_test_add_action (TESTPATHROOT "/all/order/synchronous-completion", []
    {

      synchronous_tasks tasks;
      completion<value_result_3> completed;
      auto task = start_task<&collect_synchronous_values> (tasks, completed);

      g_assert_cmpuint (tasks.starts, ==, 3);
      wait_for_completion (completed);
      assert_value (std::get<0> (completed.result), 3);
      assert_value (std::get<1> (completed.result), 5);
      assert_value (std::get<2> (completed.result), 8);
    });

  g_test_add_action (TESTPATHROOT "/all/order/void", []
    {

      controlled_tasks tasks;
      completion<void_result_3> completed;
      auto task = start_task<&collect_voids> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 3);
      tasks.complete (1);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (2);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_void_success (std::get<0> (completed.result));
      assert_void_success (std::get<1> (completed.result));
      assert_void_success (std::get<2> (completed.result));
    });

  g_test_add_action (TESTPATHROOT "/all/failure/value", []
    {

      controlled_tasks tasks;
      completion<value_result_3> completed;
      auto task = start_task<&collect_value_failure> (tasks, completed);

      tasks.complete (2);
      tasks.complete (1, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_value (std::get<0> (completed.result), 31);
      assert_controlled_failure (std::get<1> (completed.result));
      assert_value (std::get<2> (completed.result), 19);
    });

  g_test_add_action (TESTPATHROOT "/all/failure/void", []
    {

      controlled_tasks tasks;
      completion<void_result_2> completed;
      auto task = start_task<&collect_void_failure> (tasks, completed);

      tasks.complete (1, true);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_void_success (std::get<0> (completed.result));
      assert_void_failure (std::get<1> (completed.result));
    });

  g_test_add_action (TESTPATHROOT "/all/failure/start", []
    {

      controlled_tasks tasks;
      completion<value_result_2> completed;
      auto task = start_task<&collect_start_failure> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_exception_failure (std::get<0> (completed.result), "c++ exception thrown");
      assert_value (std::get<1> (completed.result), 29);
    });

  g_test_add_action (TESTPATHROOT "/all/failure/finish", []
    {

      controlled_tasks tasks;
      completion<value_result_2> completed;
      auto task = start_task<&collect_finish_failure> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 2);
      tasks.complete (0);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);

      assert_exception_failure (std::get<0> (completed.result), "c++ exception thrown");
      assert_value (std::get<1> (completed.result), 29);
    });

  g_test_add_action (TESTPATHROOT "/all/failure/void-start", []
    {

      controlled_tasks tasks;
      completion<void_result_2> completed;
      auto task = start_task<&collect_void_start_failure> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 1);
      tasks.complete (0);
      wait_for_completion (completed);

      assert_exception_void_failure (std::get<0> (completed.result));
      assert_void_success (std::get<1> (completed.result));
    });

  g_test_add_action (TESTPATHROOT "/all/failure/void-finish", []
    {

      controlled_tasks tasks;
      completion<void_result_2> completed;
      auto task = start_task<&collect_void_finish_failure> (tasks, completed);

      g_assert_cmpuint (tasks.operations.size (), ==, 2);
      tasks.complete (0);
      g_assert_cmpint (g_atomic_int_get (&completed.ready), ==, 0);
      tasks.complete (1);
      wait_for_completion (completed);

      assert_exception_void_failure (std::get<0> (completed.result));
      assert_void_success (std::get<1> (completed.result));
    });

  g_test_add_action (TESTPATHROOT "/all/lifetime/move-only-value", []
    {

      controlled_tasks tasks;
      completion<tracked_result_1> completed;
      tracked_result::releases = 0;

      {
        auto task = start_task<&collect_tracked_value> (tasks, completed);
        tasks.complete (0);
        wait_for_completion (completed);

        g_assert_true (std::holds_alternative<tracked_result> (std::get<0> (completed.result)));
        g_assert_cmpint (std::get<tracked_result> (std::get<0> (completed.result)).value, ==, 73);
        completed.result = { };
      }

      g_assert_cmpuint (tracked_result::releases, ==, 1);
    });

return g_test_run ();
}