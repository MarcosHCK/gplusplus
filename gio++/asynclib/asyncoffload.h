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
#pragma once
#include <gio/gio.h>
#include <gio++/asynclib/asynctask.h>
#include <gio++/asynclib/asynctaskpromise.h>
#include <gio++/asynclib/asynctaskpromiseactions.h>
#include <gio++/asynclib/error.h>
#include <gio++/common/boxing.h>
#include <gio++/common/slice.h>

namespace gioplusplus::asynclib::details
{

  template<typename Action,
           typename Return>
    requires (std::is_void_v<Return> || std::is_default_constructible_v<Return>)
  struct offload_task;
}

template<typename Action,
         typename Return>
  requires (std::is_void_v<Return> || std::is_default_constructible_v<Return>)
struct gioplusplus::asynclib::details::offload_task
{
public:

  static inline constexpr Return finish (GAsyncResult* async_task, GError** error) noexcept
    {
      return g_task_propagate_object<Return> ((GTask*) async_task, error);
    }

  static inline constexpr void task (GTask* task, void*, void* _p_action, GCancellable*) noexcept
    {

      try
        { g_task_return_object<Return> (task, (*(Action*) _p_action) ()); }
      catch (...)
        { g_task_return_error (task, error::to_glib_error (std::current_exception ())); }
    }
};

template<typename Action>
struct gioplusplus::asynclib::details::offload_task<Action, void>
{
public:

  static inline constexpr void finish (GAsyncResult* async_task, GError** error) noexcept
    {
      g_task_propagate_boolean ((GTask*) async_task, error);
    }

  static inline constexpr void task (GTask* task, void*, void* _p_action, GCancellable*) noexcept
    {

      try
        { (*(Action*) _p_action) ();
          g_task_return_boolean (task, TRUE); }
      catch (...)
        { g_task_return_error (task, error::to_glib_error (std::current_exception ())); }
    }
};

namespace gioplusplus::asynclib
{

  template<typename Action>
    requires (std::is_invocable_v<Action>)
  static inline constexpr auto offload (Action&& action) noexcept (std::is_nothrow_move_constructible_v<Action>)
    {

      using return_type = std::invoke_result_t<Action>;

      auto begin = [action = std::move (action)] (GAsyncReadyCallback callback, gpointer user_data)
          mutable noexcept (std::is_nothrow_move_constructible_v<Action>) -> void
        {

          boxing::object task = g_task_new (NULL, NULL, callback, user_data);
          Action* task_data = g_slice_new_<Action> (std::move (action));

          g_task_set_task_data (task, task_data, g_slice_free_<Action>);

        return g_task_run_in_thread (task, details::offload_task<Action, return_type>::task);
        };

      constexpr auto _Begin = details::async_task_promise<return_type, decltype (begin)>::_Begin;

    return details::async_task<_Begin, details::offload_task<Action, return_type>::finish, decltype (begin)> (std::move (begin));
    }
}