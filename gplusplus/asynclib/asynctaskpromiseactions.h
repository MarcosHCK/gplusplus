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
#include <gplusplus/common/boxing.h>
#include <gplusplus/common/slice.h>

namespace gplusplus::asynclib::details
{

  template<typename T> struct _task_return
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<T, Args ...>;

      template<typename... Args>
        requires (std::is_constructible_v<T, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto ptr = g_slice_new_<T> (std::forward<Args> (args) ...);
        return g_task_return_pointer (task, ptr, g_slice_free_<T>);
        }
    };

  template<typename T,
           boxing::details::_copy_func<T> auto _copy_func,
           boxing::details::_free_func<T> auto _free_func>
  struct _task_return<boxing::shared_ptr<T, _copy_func, _free_func>>
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<T, Args ...>;

      template<typename... Args>
        requires (std::is_constructible_v<T, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto ptr = boxing::shared_ptr<T, _copy_func, _free_func> (std::forward<Args> (args) ...);
        return g_task_return_pointer (task, ptr.copy (), _free_func);
        }
    };

  template<typename T,
           boxing::details::_free_func<T> auto _free_func>
  struct _task_return<boxing::unique_ptr<T, _free_func>>
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<T, Args ...>;

      template<typename... Args>
        requires (std::is_constructible_v<T, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto ptr = boxing::unique_ptr<T, _free_func> (std::forward<Args> (args) ...);
        return g_task_return_pointer (task, ptr.release (), _free_func);
        }
    };

  template<> struct _task_return<bool>
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<bool, Args ...>;

      template<typename... Args>
        requires (std::is_constructible_v<bool, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto val = bool (std::forward<Args> (args) ...);
        return g_task_return_boolean (task, val);
        }
    };

  template<> struct _task_return<int>
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<int, Args ...>;

      template<typename... Args>
        requires (std::is_constructible_v<int, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto val = int (std::forward<Args> (args) ...);
        return g_task_return_int (task, val);
        }
    };

  template<> struct _task_return<void*>
    {

      template<typename... Args>
      static inline constexpr bool noexcept_ = std::is_nothrow_constructible_v<void*, Args ...>;

      using pointer = void*;

      template<typename... Args>
        requires (std::is_constructible_v<void*, Args ...>)
      inline void operator() (GTask* task, Args&&... args) noexcept (noexcept_<Args ...>)
        {
          auto val = pointer (std::forward<Args> (args) ...);
        return g_task_return_pointer (task, val, NULL);
        }
    };

  template<typename T, typename... Args>
    requires (std::is_constructible_v<T, Args ...>)
  static inline void g_task_return_object (GTask* task, Args&&... args) noexcept (_task_return<T>::template noexcept_<Args ...>)
    {
      return (_task_return<T> {}) (task, std::forward<Args> (args) ...);
    }

  template<typename T> struct _task_propagate
    {

      using box = boxing::unique_ptr<T, g_slice_free_<T>>;

      static inline constexpr bool noexcept_ = std::is_nothrow_move_constructible_v<T>
                                            && std::is_nothrow_destructible_v<T>;

      inline T operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          GError* tmperr = NULL;

          if (box ptr = (T*) g_task_propagate_pointer (task, &tmperr); G_LIKELY (NULL == tmperr))

            return T (std::move (*ptr.get ()));
          else
            return g_propagate_error (error, tmperr), T ();
        }
    };

  template<typename T,
           boxing::details::_copy_func<T> auto _copy_func,
           boxing::details::_free_func<T> auto _free_func>
  struct _task_propagate<boxing::shared_ptr<T, _copy_func, _free_func>>
    {

      using box = boxing::shared_ptr<T, _copy_func, _free_func>;

      static inline constexpr bool noexcept_ = std::is_constructible_v<boxing::shared_ptr<T, _copy_func, _free_func>, T*>;

      inline box operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          return (T*) g_task_propagate_pointer (task, error);
        }
    };

  template<typename T,
           boxing::details::_free_func<T> auto _free_func>
  struct _task_propagate<boxing::unique_ptr<T, _free_func>>
    {

      using box = boxing::unique_ptr<T, _free_func>;

      static inline constexpr bool noexcept_ = std::is_constructible_v<boxing::unique_ptr<T, _free_func>, T*>;

      inline box operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          return (T*) g_task_propagate_pointer (task, error);
        }
    };

  template<> struct _task_propagate<bool>
    {

      static inline constexpr bool noexcept_ = true;

      inline bool operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          return g_task_propagate_boolean (task, error);
        }
    };

  template<> struct _task_propagate<int>
    {

      static inline constexpr bool noexcept_ = true;

      inline int operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          return g_task_propagate_int (task, error);
        }
    };

  template<> struct _task_propagate<void*>
    {

      static inline constexpr bool noexcept_ = true;

      using pointer = void*;

      inline pointer operator() (GTask* task, GError** error) noexcept (noexcept_)
        {
          return g_task_propagate_pointer (task, error);
        }
    };

  template<typename T>
    requires (std::is_default_constructible_v<T> && std::is_move_constructible_v<T>)
  static inline constexpr T g_task_propagate_object (GTask* task, GError** error) noexcept (_task_propagate<T>::noexcept_)
    {
      return (_task_propagate<T> {}) (task, error);
    }
}