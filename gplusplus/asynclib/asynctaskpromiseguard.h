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

namespace gplusplus::asynclib::details
{

  template<typename promise_type> struct async_task_promise_guard
    {

      std::coroutine_handle<promise_type> handle = nullptr;

      inline ~async_task_promise_guard ()
        { ((nullptr == handle) ? nullptr : (handle = (handle.destroy (), nullptr))); }

      inline async_task_promise_guard (async_task_promise_guard&& o) noexcept: handle (o.handle)
        { o.handle = nullptr; }

      async_task_promise_guard (const async_task_promise_guard&) = delete;

      inline async_task_promise_guard (std::coroutine_handle<promise_type> _handle) noexcept: handle (_handle)
        { }

      inline void resume () noexcept (std::is_nothrow_invocable_v<decltype (&std::coroutine_handle<promise_type>::resume)>)
        { ((nullptr == handle) ? nullptr : (handle = (handle.resume (), nullptr))); }
    };
}