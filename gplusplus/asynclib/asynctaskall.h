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
#pragma once
#include <gio/gio.h>
#include <gplusplus/asynclib/asynctask.h>
#include <gplusplus/asynclib/asynctaskawaitable.h>
#include <gplusplus/asynclib/asynctaskconcept.h>
#include <gplusplus/asynclib/error.h>
#include <gplusplus/common/boxing.h>
#include <gplusplus/common/soo.h>
#include <optional>
#include <variant>

namespace gplusplus::asynclib::details
{

  template<typename Return>
  struct async_task_all_builder_base
    {

      struct state
        {

          Return return_value;
          unsigned pending;

          inline constexpr state (unsigned pending_) noexcept: pending (pending_)
            { }
        };

      static inline Return complete (GAsyncResult* async_result, GError** error) noexcept
        {

          auto& stat = *(state*) g_task_propagate_pointer ((GTask*) async_result, error);

          try
            { return Return (std::move (stat.return_value)); }
          catch (...)
            { g_propagate_error (error, error::to_glib_error (std::current_exception ())); }
        return Return { };
        }

      template<async_task_type Task>
      static inline auto finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {

          try
            { return Task::finish (source_object, async_result, error); }
          catch (...)
            { g_propagate_error (error, details::error::to_glib_error (std::current_exception ())); }
        }

      static inline void notify (state& stat, gpointer user_data) noexcept
        {

          if (g_atomic_int_dec_and_test (&stat.pending))
            g_task_return_pointer ((GTask*) user_data, &stat, NULL);

          g_object_unref ((GTask*) user_data);
        }
    };

  template<async_task_type Task,
           bool Noexcept_ = std::is_nothrow_invocable_v<decltype (Task::finish), GObject*, GAsyncResult*, GError**>>
  struct async_task_all_builder_finishable
    {

      static inline auto finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {

          try
            { return Task::finish (source_object, async_result, error); }
          catch (...)
            { g_propagate_error (error, details::error::to_glib_error (std::current_exception ())); }
        return typename Task::end_details::return_type { };
        }
    };

  template<async_task_type Task>
  struct async_task_all_builder_finishable<Task, true>
    {

      static inline auto finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {
          return Task::finish (source_object, async_result, error);
        }
    };

  template<typename Return, bool AllVoid>
  struct async_task_all_builder_notifiable: public async_task_all_builder_base<Return>
    {

      using state = typename async_task_all_builder_base<Return>::state;

      template<unsigned N, async_task_type Task>
      static inline void notify (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = nullptr;
          auto& stat = *(state*) g_task_get_task_data ((GTask*) user_data);

          using finishable = async_task_all_builder_finishable<Task>;
          using return_type = typename Task::end_details::return_type;

          if (auto result = finishable::finish (source_object, async_result, &tmperr); G_UNLIKELY (NULL != tmperr))

            { std::get<N> (stat.return_value).template emplace<error> (tmperr); }
          else try
            { std::get<N> (stat.return_value).template emplace<return_type> (std::move (result)); }
          catch (...)
            { std::get<N> (stat.return_value).template emplace<error> (error::to_glib_error (std::current_exception ())); }

        return async_task_all_builder_base<Return>::notify (stat, user_data);
        }
    };

  template<typename Return>
  struct async_task_all_builder_notifiable<Return, true>: public async_task_all_builder_base<Return>
    {

      using state = typename async_task_all_builder_base<Return>::state;

      template<unsigned N, async_task_type Task>
      static inline void notify (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = nullptr;
          auto& stat = *(state*) g_task_get_task_data ((GTask*) user_data);

          using finishable = async_task_all_builder_finishable<Task>;

          if (finishable::finish (source_object, async_result, &tmperr); G_LIKELY (NULL == tmperr))

            std::get<N> (stat.return_value).reset ();
          else
            std::get<N> (stat.return_value).emplace (tmperr);

        return async_task_all_builder_base<Return>::notify (stat, user_data);
        }
    };

  template<async_task_type... Tasks>
    requires ((std::same_as<void, typename Tasks::end_details::return_type> && ...)
           || ((! std::same_as<void, typename Tasks::end_details::return_type>) && ...))
  using async_task_all_return = std::conditional_t<(std::same_as<void, typename Tasks::end_details::return_type> && ...),
                                                   std::tuple<std::conditional_t<false, Tasks, std::optional<error>> ...>,
                                                   std::tuple<std::variant<error, typename Tasks::end_details::return_type> ...>>;

  template<async_task_type... Tasks>
    requires ((std::same_as<void, typename Tasks::end_details::return_type> && ...)
           || ((! std::same_as<void, typename Tasks::end_details::return_type>) && ...))
  struct async_task_all_builder: public async_task_all_builder_notifiable<async_task_all_return<Tasks ...>,
                                                                          (std::same_as<void, typename Tasks::end_details::return_type> && ...)>
    {

      using parent = async_task_all_builder_notifiable<async_task_all_return<Tasks ...>,
                                                       (std::same_as<void, typename Tasks::end_details::return_type> && ...)>;

      template<unsigned... Is>
        requires (sizeof... (Is) == sizeof... (Tasks))
      static inline constexpr void begin_all (std::tuple<Tasks...>& tasks, gpointer task, std::integer_sequence<unsigned, Is ...> const&)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {
          (begin_one<Is> (std::get<Is> (tasks), g_object_ref (task)), ...);
        }

      static inline constexpr void begin_mock (GAsyncReadyCallback callback, gpointer user_data)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {
        }

      template<unsigned N, async_task_type Task>
      static inline constexpr void begin_one (Task& task, gpointer g_task)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {

          try
            { task (parent::template notify<N, Task>, g_task); }
          catch (...)
            { auto e = error::to_glib_error (std::current_exception ());
              g_task_report_error (NULL, parent::template notify<N, Task>, g_task, NULL, e); }
        }

      static inline constexpr auto build (std::tuple<Tasks...> tasks) noexcept (std::is_nothrow_move_constructible_v<std::tuple<Tasks ...>>)
        {

          using Return = async_task_all_return<Tasks ...>;
          using State = typename async_task_all_builder_base<Return>::state;

          auto begin = [tasks = std::move (tasks)] (GAsyncReadyCallback callback, gpointer user_data)
               mutable noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...)) -> void
            {

              constexpr unsigned N = sizeof... (Tasks);

              boxing::object task = g_task_new (NULL, NULL, callback, user_data);
              gpointer task_data = nullptr;

              g_task_set_task_data (task, (soo_ptr::create<State> (&task_data, N), task_data), destroy);
              begin_all (tasks, (gpointer) *task, std::make_integer_sequence<unsigned, N> ());
            };
        return async_task<begin_mock, parent::complete, decltype (begin)> (std::move (begin));
        }

      static inline constexpr void destroy (gpointer data) noexcept
        {
          soo_ptr::destroy<typename parent::state> (&data);
        }
    };
}