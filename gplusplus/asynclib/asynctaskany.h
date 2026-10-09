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
#include <gplusplus/common/type_traits.h>
#include <gplusplus/common/soo.h>
#include <variant>

namespace gplusplus::asynclib::details
{

  template<typename Return>
  struct async_task_any_builder_base
    {

      struct state_base_t
        {

          unsigned fulfilled, pending;
          Return return_value;

          inline constexpr state_base_t (unsigned pending_) noexcept:
              fulfilled (0), pending (pending_), return_value { }
            { }
        };

      template<bool NothrowMove = std::is_nothrow_move_constructible_v<Return>>
      struct state_t: public state_base_t
        {

          inline constexpr state_t (unsigned pending) noexcept:
              state_base_t (pending)
            { }

          inline bool move_lock () noexcept
            { return g_atomic_int_compare_and_exchange (&this->fulfilled, 0, 1); }

          inline void move_unlock (bool done) noexcept
            {
              if (! done)
                g_atomic_int_set (&this->fulfilled, 0);
            }
        };

      template<>
      struct state_t<false>: public state_base_t
        {

          GMutex lock;

          inline constexpr state_t (unsigned pending) noexcept:
              state_base_t (pending), lock ({ 0 })
            { }

          inline bool move_lock () noexcept
            {

              bool allow; if (! (allow = (g_mutex_lock (&lock), 0 == this->fulfilled)))
                g_mutex_unlock (&lock);
            return allow;
            }

          inline void move_unlock (bool done) noexcept
            {
              if (done)
                this->fulfilled = 1;
            return g_mutex_unlock (&lock);
            }
        };

      using state = state_t<>;

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

          if (g_atomic_int_dec_and_test (&stat.pending) && stat.move_lock ())
            {
              stat.move_unlock (true);
              g_task_return_pointer ((GTask*) user_data, &stat, NULL);
            }

          g_object_unref ((GTask*) user_data);
        }
    };

  template<async_task_type Task,
           bool Noexcept_ = std::is_nothrow_invocable_v<decltype (Task::finish), GObject*, GAsyncResult*, GError**>>
  struct async_task_any_builder_finishable
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
  struct async_task_any_builder_finishable<Task, true>
    {

      static inline auto finish (GObject* source_object, GAsyncResult* async_result, GError** error) noexcept
        {
          return Task::finish (source_object, async_result, error);
        }
    };

  template<typename Return, bool AllVoid>
  struct async_task_any_builder_notifiable: public async_task_any_builder_base<Return>
    {

      using state = typename async_task_any_builder_base<Return>::state;

      template<async_task_type Task>
      static inline void notify (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = nullptr;
          auto& stat = *(state*) g_task_get_task_data ((GTask*) user_data);

          using finishable = async_task_any_builder_finishable<Task>;
          using return_type = typename Task::end_details::return_type;

          if (auto result = finishable::finish (source_object, async_result, &tmperr);
              G_UNLIKELY (NULL != tmperr))

            g_error_free (tmperr);
          else if (stat.move_lock ()) try

            { stat.return_value.template emplace<return_type> (std::move (result));
              stat.move_unlock (true);
              g_task_return_pointer ((GTask*) user_data, &stat, NULL); }
          catch (...)
            { stat.move_unlock (false); }

        return async_task_any_builder_base<Return>::notify (stat, user_data);
        }
    };

  template<typename Return>
  struct async_task_any_builder_notifiable<Return, true>: public async_task_any_builder_base<Return>
    {

      using state = typename async_task_any_builder_base<Return>::state;

      template<async_task_type Task>
      static inline void notify (GObject* source_object, GAsyncResult* async_result, gpointer user_data) noexcept
        {

          GError* tmperr = nullptr;
          auto& stat = *(state*) g_task_get_task_data ((GTask*) user_data);

          using finishable = async_task_any_builder_finishable<Task>;

          if (finishable::finish (source_object, async_result, &tmperr); G_UNLIKELY (NULL != tmperr))
            g_error_free (tmperr);

          else if (stat.move_lock ())
            {
              stat.move_unlock (stat.return_value = true);
              g_task_return_pointer ((GTask*) user_data, &stat, NULL);
            }

        return async_task_any_builder_base<Return>::notify (stat, user_data);
        }
    };

  struct async_task_any_nul_tag { };

  template<async_task_type... Tasks>
    requires ((std::same_as<void, typename Tasks::end_details::return_type> && ...)
           || ((! std::same_as<void, typename Tasks::end_details::return_type>) && ...))
  using async_task_any_return = std::conditional_t<(std::same_as<void, typename Tasks::end_details::return_type> && ...),
    bool, typename traits::type_list_to_container<std::variant, traits::type_list_of_unique_t<async_task_any_nul_tag,
                                                                                              typename Tasks::end_details::return_type ...>>::type>;

  template<async_task_type... Tasks>
    requires ((std::same_as<void, typename Tasks::end_details::return_type> && ...)
           || ((! std::same_as<void, typename Tasks::end_details::return_type>) && ...))
  struct async_task_any_builder: public async_task_any_builder_notifiable<async_task_any_return<Tasks ...>,
                                                                          (std::same_as<void, typename Tasks::end_details::return_type> && ...)>
    {

      using parent = async_task_any_builder_notifiable<async_task_any_return<Tasks ...>,
                                                       (std::same_as<void, typename Tasks::end_details::return_type> && ...)>;

      template<unsigned... Is>
        requires (sizeof... (Is) == sizeof... (Tasks))
      static inline constexpr void begin_all (std::tuple<Tasks...>& tasks, gpointer task, std::integer_sequence<unsigned, Is ...> const&)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {
          (begin_one (std::get<Is> (tasks), g_object_ref (task)), ...);
        }

      static inline constexpr void begin_mock (GAsyncReadyCallback callback, gpointer user_data)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {
        }

      template<async_task_type Task>
      static inline constexpr void begin_one (Task& task, gpointer g_task)
          noexcept ((std::is_nothrow_invocable_v<Tasks, GAsyncReadyCallback, gpointer> && ...))
        {

          try
            { task (parent::template notify<Task>, g_task); }
          catch (...)
            { auto e = error::to_glib_error (std::current_exception ());
              g_task_report_error (NULL, parent::template notify<Task>, g_task, NULL, e); }
        }

      static inline constexpr auto build (std::tuple<Tasks...> tasks) noexcept (std::is_nothrow_move_constructible_v<std::tuple<Tasks ...>>)
        {

          using Return = async_task_any_return<Tasks ...>;
          using State = typename async_task_any_builder_base<Return>::state;

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