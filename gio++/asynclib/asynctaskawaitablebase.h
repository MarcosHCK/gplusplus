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
#include <gio++/asynclib/asyncfunction.h>
#include <gio++/asynclib/error.h>
#include <gio++/common/boxing.h>
#include <optional>

namespace gioplusplus::asynclib::details
{

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename Functor,
           typename Return>
  struct async_task_awaitable_base
    {

      inline Return await_resume ()
        {

          if (G_UNLIKELY (nullptr != _error))
            {
              auto error = _error.release ();
              std::rethrow_exception (error::from_glib_error (error));
            }
        return std::move (_return).value ();
        }

      inline constexpr async_task_awaitable_base (async_task<_Begin, _End, Functor>&& task)
          noexcept (std::is_nothrow_move_constructible_v<decltype (task)>):
          _task (std::move (task))
        { }

    protected:

      boxing::error _error = nullptr;
      std::optional<Return> _return = std::nullopt;
      async_task<_Begin, _End, Functor> _task;

      inline void await_complete (GObject* source_object, GAsyncResult* async_result)
          noexcept (async_function_end_details<decltype (_End)>::noexcept_v && std::is_nothrow_move_constructible_v<Return>)
        {

          GError* error = NULL;

          if (Return result = this->_task.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))

            this->_error = error;
          else
            this->_return.emplace (std::move (result));
        }
    };

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename Functor>
  struct async_task_awaitable_base<_Begin, _End, Functor, void>
    {

      inline void await_resume ()
        {

          if (G_UNLIKELY (nullptr != _error))
            {
              auto error = _error.release ();
              std::rethrow_exception (error::from_glib_error (error));
            }
        }

      inline constexpr async_task_awaitable_base (async_task<_Begin, _End, Functor>&& task)
          noexcept (std::is_nothrow_move_constructible_v<decltype (task)>):
          _task (std::move (task))
        { }

    protected:

      boxing::error _error = nullptr;
      async_task<_Begin, _End, Functor> _task;

      inline void await_complete (GObject* source_object, GAsyncResult* async_result)
          noexcept (async_function_end_details<decltype (_End)>::noexcept_v)
        {

          GError* error = NULL;

          if (this->_task.finish (source_object, async_result, &error); G_UNLIKELY (NULL != error))
            this->_error = error;
        }
    };
}