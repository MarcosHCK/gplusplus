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
#include <gplusplus/asynclib/asynctaskpromise.h>

namespace std
{

  template<gplusplus::asynclib::details::async_function_begin auto _Begin,
           gplusplus::asynclib::details::async_function_end auto _End,
           typename Functor,
           typename... Args>
  struct coroutine_traits<gplusplus::asynclib::details::async_task<_Begin, _End, Functor>, Args ...>
    {
      struct promise_type;
    };

  template<gplusplus::asynclib::details::async_function_begin auto _Begin,
           gplusplus::asynclib::details::async_function_end auto _End,
           typename Functor,
           typename... Args>
  struct coroutine_traits<gplusplus::asynclib::details::async_task<_Begin, _End, Functor>, Args ...>::promise_type:
      public gplusplus::asynclib::details::async_task_promise<typename gplusplus::asynclib::details::async_function_end_details<decltype (_End)>::return_type, Functor, Args ...>
    {
    };
}