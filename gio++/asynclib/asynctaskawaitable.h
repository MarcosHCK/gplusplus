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
#include <coroutine>
#include <gio++/asynclib/asynctaskawaitablebase.h>

namespace gioplusplus::asynclib::details
{

  template<async_function_begin auto _Begin,
           async_function_end auto _End,
           typename Functor>
  struct async_task_awaitable: public async_task_awaitable_base<_Begin, _End, Functor, typename async_function_end_details<decltype (_End)>::return_type>
    {

      using begin_details = async_function_begin_details<decltype (_Begin)>;
      using end_details = async_function_end_details<decltype (_End)>;

      inline constexpr async_task_awaitable (async_task<_Begin, _End, Functor>&& task)
          noexcept (std::is_nothrow_constructible_v<async_task_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>, decltype (task)&&>):
          async_task_awaitable_base<_Begin, _End, Functor, typename end_details::return_type> (std::move (task))
        { }

      inline bool await_ready () const noexcept 
        { return false; }

      inline void await_suspend (std::coroutine_handle<> handle)
          noexcept (std::is_nothrow_invocable_v<async_task<_Begin, _End, Functor>, GAsyncReadyCallback, gpointer>)
        {

          struct Data
            {

              std::coroutine_handle<> handle;
              async_task_awaitable<_Begin, _End, Functor>& self;

              inline Data (std::coroutine_handle<> _handle, async_task_awaitable<_Begin, _End, Functor>& _self) noexcept:
                  handle (_handle), self (_self)
                { }
            };

          auto data = new (g_slice_alloc0 (sizeof (Data))) Data (handle, *this);

          try { this->_task ([] (GObject* source_object, GAsyncResult* async_result, gpointer user_data)
            {
              auto handle = ((Data*) user_data)->handle;
              ((Data*) user_data)->self.await_complete (source_object, async_result);
              ((Data*) user_data)->~Data ();

              g_slice_free1 (sizeof (Data), user_data);
              handle.resume ();
            }, data); }
          catch (...)
            {

              this->_error = error::to_glib_error (std::current_exception ());
              ((Data*) data)->~Data ();
              g_slice_free1 (sizeof (Data), data);
              handle.resume ();
            }
        }

    private:

      inline void await_complete (GObject* source_object, GAsyncResult* async_result) noexcept
        {

          if constexpr (end_details::noexcept_v)

            { async_task_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>::await_complete (source_object, async_result); }
          else try
            { async_task_awaitable_base<_Begin, _End, Functor, typename end_details::return_type>::await_complete (source_object, async_result); }
          catch (...)
            { this->_error = error::to_glib_error (std::current_exception ()); }
        }
    };
}

template<gioplusplus::asynclib::details::async_function_begin auto _Begin,
         gioplusplus::asynclib::details::async_function_end auto _End,
         typename Functor>
static inline constexpr auto operator co_await (gioplusplus::asynclib::details::async_task<_Begin, _End, Functor>&& task)
  noexcept (std::is_nothrow_constructible_v<gioplusplus::asynclib::details::async_task_awaitable<_Begin, _End, Functor>, decltype (task)&&>)
{
  return gioplusplus::asynclib::details::async_task_awaitable<_Begin, _End, Functor> (std::move (task));
}