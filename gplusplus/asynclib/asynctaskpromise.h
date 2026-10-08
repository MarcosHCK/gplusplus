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
#include <coroutine>
#include <gio/gio.h>
#include <gplusplus/asynclib/asynctask.h>
#include <gplusplus/asynclib/asynctaskpromiseactions.h>
#include <gplusplus/asynclib/error.h>
#include <gplusplus/common/boxing.h>
#include <gplusplus/common/soo.h>

namespace gplusplus::asynclib::details
{

  struct async_task_promise_base
    {

      inline constexpr std::suspend_never final_suspend () noexcept
        {
          return { };
        }

      inline constexpr std::suspend_always initial_suspend () noexcept
        {
          return { };
        }

      inline void unhandled_exception ()
        {
          g_task_return_error (_task, error::to_glib_error (std::current_exception ()));
        }

    protected:

      boxing::object<GTask> _task;

      static inline constexpr void begin_mock (GAsyncReadyCallback callback, gpointer user_data) noexcept
        { }
    };

  template<typename T>
  concept async_task_promise_target = async_function_end<T (*) (GAsyncResult*, GError**)>
                                   && (std::is_void_v<T> || std::is_default_constructible_v<T>);

  template<async_task_promise_target Return>
  struct async_task_promise_completable: public async_task_promise_base
    {

      template<typename U = Return>
        requires (std::is_constructible_v<Return, U>)
      inline void return_value (U&& value) noexcept (std::is_nothrow_constructible_v<Return, U>)
        {
          return g_task_return_object<Return, U> (_task, std::forward<U> (value));
        }

    protected:

      static Return fulfill (GAsyncResult* async_result, GError** error) noexcept (std::is_nothrow_move_constructible_v<Return>)
        {
          return g_task_propagate_object<Return> ((GTask*) async_result, error);
        }
    };

  template<>
  struct async_task_promise_completable<void>: public async_task_promise_base
    {

      inline void return_void () noexcept
        {
          g_task_return_pointer (this->_task, NULL, NULL);
        }

    protected:

      static void fulfill (GAsyncResult* async_result, GError** error) noexcept
        {
          g_task_propagate_pointer ((GTask*) async_result, error);
        }
    };

  template<async_task_promise_target Return,
           typename Functor,
           typename... Args>
  struct async_task_promise: public async_task_promise_completable<Return>
    {

      template<typename promise_type> struct __coroutine_handle_guard
        {

          std::coroutine_handle<promise_type> handle = nullptr;

          inline ~__coroutine_handle_guard ()
            { ((nullptr == handle) ? nullptr : (handle = (handle.destroy (), nullptr))); }

          inline __coroutine_handle_guard (__coroutine_handle_guard&& o) noexcept: handle (o.handle)
            { o.handle = nullptr; }

          __coroutine_handle_guard (const __coroutine_handle_guard&) = delete;

          inline __coroutine_handle_guard (std::coroutine_handle<promise_type> _handle) noexcept: handle (_handle)
            { }

          inline void resume () noexcept (std::is_nothrow_invocable_v<decltype (&std::coroutine_handle<promise_type>::resume)>)
            { ((nullptr == handle) ? nullptr : (handle = (handle.resume (), nullptr))); }
        };

      static constexpr auto _Begin = async_task_promise_base::begin_mock;
      static constexpr auto _End = async_task_promise_completable<Return>::fulfill;

      inline constexpr async_task<_Begin, _End, Functor> get_return_object () noexcept
        {

          using task_type = async_task<_Begin, _End, Functor>;
          using promise_type = typename std::coroutine_traits<task_type, Args ...>::promise_type;

          auto handle = std::coroutine_handle<promise_type>::from_promise ((promise_type&) *this);
          auto guard = __coroutine_handle_guard<promise_type> (handle);

          auto begin = [this, guard = std::move (guard)] (GAsyncReadyCallback callback, gpointer user_data) mutable -> void
            {
              this->_task = g_task_new (NULL, NULL, callback, user_data);
              guard.resume ();
            };
        return async_task<_Begin, _End, Functor> (std::move (begin));
        }
    };

  template<typename T,
           typename Return>
  concept async_task_promise_object = async_task_promise_target<Return>;
}