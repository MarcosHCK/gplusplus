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
#include <concepts>
#include <gio++/common/constexprstring.h>
#include <glib-object.h>
#include <type_traits>

namespace gioplusplus::object
{

  namespace details
    {

      class object_class_base;

      struct type_tag_base { };

      template<typename T>
      concept type_tag = std::is_base_of_v<type_tag_base, T>
                        && ! std::is_same_v<type_tag_base, T>;

      template<typename Fn, typename Ret = void, typename... Args>
      concept invocable_r = std::is_invocable_r_v<Ret, Fn, Args ...>;

      template<typename Fn, typename Ret, typename... Args>
      concept invocable_r_or_null = std::is_same_v<Fn, std::nullptr_t>
                                 || std::is_invocable_r_v<Ret, Fn, Args ...>;
    }

  class details::object_class_base
    {
    };

  template<typename ClassType = GObjectClass,
           typename InstanceType = GObject,
           details::invocable_r<GType> auto GetType = g_object_get_type>
  struct object_class_ancestor
    {

      typedef ClassType Class;
      typedef InstanceType Instance;

      static constexpr auto get_type = GetType;
    };

  template<typename IfaceType,
           details::invocable_r<GType> auto GetType,
           details::invocable_r<void, gpointer, gpointer> auto IfaceInit>
  struct object_class_interface
    {

      typedef IfaceType Iface;

      static constexpr auto get_type = GetType;
      static constexpr auto iface_init = IfaceInit;
    };

  namespace type_tag
    {

      struct abstract: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_ABSTRACT; };

      struct final: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_FINAL; };

      struct normal: details::type_tag_base
        { static inline constexpr GTypeFlags flags = G_TYPE_FLAG_NONE; };
    }

  namespace details
    {

      template<typename T,
               object_class_ancestor Ancestor = object_class_ancestor ()>
      class object_class_structs;
    }

  template<typename T,
           object_class_ancestor Ancestor>
  class details::object_class_structs
    {

    public:

      struct class_struct
        {
          typename decltype (Ancestor)::Class parent_class;
        };

      struct instance_struct
        {
          typename decltype (Ancestor)::Instance parent_instance;
          T self;
        };
    };

  template<typename ClassType,
           typename InstanceType,
           constexpr_string Name,
           details::type_tag TypeTag = type_tag::normal,
           details::invocable_r_or_null<void, gpointer> auto BaseInit = nullptr,
           details::invocable_r_or_null<void, gpointer> auto BaseFini = nullptr,
           details::invocable_r_or_null<void, gpointer, gpointer> auto ClassInit = nullptr,
           details::invocable_r_or_null<void, gpointer, gpointer> auto ClassFini = nullptr,
           details::invocable_r_or_null<void, GTypeInstance*, gpointer> auto InstanceInit = nullptr,
           object_class_ancestor Ancestor = object_class_ancestor (),
           object_class_interface... Implementations>
  class object_class: public details::object_class_base
    {

      template<object_class_interface Implementation_>
      static inline int implement_interface (GType derived_type) noexcept
        {

          static GInterfaceInfo info =
            {
              .interface_init = decltype (Implementation_)::iface_init,
              .interface_finalize = nullptr,
              .interface_data = nullptr,
            };

          auto interface_type = decltype (Implementation_)::get_type ();

        return g_type_add_interface_static (derived_type, interface_type, &info), 0;
        }

      template<object_class_interface... Implementations_>
      static inline void implement_interface (GType derived_type, int) noexcept
        {
          (void) (implement_interface<Implementations_> (derived_type), ...);
        }

    public:

      static inline GType get_type () noexcept G_GNUC_CONST
        {

          static gsize g_type_id = 0;

          if (g_once_init_enter (&g_type_id))
            {
              auto g_type = get_type_once ();
              g_once_init_leave (&g_type_id, g_type);
            }
        return (GType) g_type_id;
        }

    private:

      static inline GType get_type_once () noexcept
        {

        static GTypeInfo info =
          {

            .class_size = sizeof (ClassType),
            .base_init = BaseInit,
            .base_finalize = BaseFini,

            .class_init = [](gpointer g_class, gpointer class_data) noexcept -> void
              {
                if constexpr (! std::same_as<std::nullptr_t, decltype (ClassInit)>)
                  ClassInit (g_class, class_data);
              },

            .class_finalize = ClassFini,
            .class_data = nullptr,
            .instance_size = sizeof (InstanceType),
            .n_preallocs = 0,

            .instance_init = [](GTypeInstance* p_value, gpointer g_class) noexcept -> void
              {
                if constexpr (! std::same_as<std::nullptr_t, decltype (InstanceInit)>)
                  InstanceInit (p_value, g_class);
              },

            .value_table = nullptr,
          };

          auto ancestor_type = decltype (Ancestor)::get_type ();
          auto interned_name = g_intern_static_string (Name.c_str ());
          auto derived_type = g_type_register_static (ancestor_type, interned_name, &info, TypeTag::flags);

          implement_interface<Implementations ...> (derived_type, 0);
        return derived_type;
        }
    };
}