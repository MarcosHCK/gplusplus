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
#include <gio++/common/constexprregistry.h>
#include <gio++/object/objectclass.h>
#include <gio++/object/objectimplementorbase.h>
#include <gio++/object/objectimplementorcallables.h>
#include <gio++/object/objectimplementorclass.h>
#include <gio++/object/objectimplementorproperties.h>
#include <gio++/object/objectimplementorstructs.h>

# define _GPP_IMPLEMENT(TypeName,type_name,tag,...) \
 ; \
  struct __gioplusplus_implementor_info_base_##TypeName \
    { \
 ; \
      using Type = TypeName; \
      using TypeTag = tag; \
 ; \
      struct property_adl_tag { }; \
      struct property_first_adl_tag { }; \
 ; \
      static gpointer parent_class; \
      static inline constexpr auto ClassName = constexpr_string (#TypeName); \
    }; \
 ; \
    gpointer __gioplusplus_implementor_info_base_##TypeName::parent_class = nullptr; \
 ; \
  struct __gioplusplus_implementor_info_##TypeName: public __gioplusplus_implementor_info_base_##TypeName, \
                                                    public gioplusplus::object::details::store_self<__gioplusplus_implementor_info_##TypeName> \
    __VA_ARGS__; \
  using __gioplusplus_implementor_type_##TypeName = \
    gioplusplus::object::details::build_class::object_class<__gioplusplus_implementor_info_##TypeName>; \
 ; \
  GType type_name##_get_type () noexcept G_GNUC_CONST; \
  GType type_name##_get_type () noexcept { return __gioplusplus_implementor_type_##TypeName::get_type (); }

# define GPP_IMPLEMENT_ABSTRACT(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gioplusplus::object::type_tag::abstract, __VA_ARGS__)

# define GPP_IMPLEMENT_FINAL(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gioplusplus::object::type_tag::final, __VA_ARGS__)

# define GPP_IMPLEMENT(TypeName,type_name,...) _GPP_IMPLEMENT(TypeName, type_name, gioplusplus::object::type_tag::normal, __VA_ARGS__)

# define GPP_IMPLEMENT_ANCESTOR(TypeName,type_name) \
  static inline constexpr auto Ancestor = (gioplusplus::object::object_class_ancestor<TypeName, TypeName##Class, type_name##_get_type> ());

# define GPP_IMPLEMENT_CLASS_VTABLE(...) struct VTable __VA_ARGS__;

# define GPP_IMPLEMENT_BASE_CLASS_INIT(...) \
  static inline void base_class_init (gioplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_BASE_CLASS_FINI(...) \
  static inline void base_class_fini (gioplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_CLASS_INIT(...) \
  static inline void class_init (gioplusplus::object::details::build_class::class_struct<Self>* klass, gpointer class_data) noexcept \
    { \
      parent_class = g_type_class_peek_parent ((gpointer) klass); \
      ([](decltype (klass) klass) noexcept -> void __VA_ARGS__) (klass); \
    }

# define GPP_IMPLEMENT_CLASS_FINI(...) \
  static inline void class_fini (gioplusplus::object::details::build_class::class_struct<Self>* klass, \
      gpointer class_data) noexcept \
    __VA_ARGS__;

# define GPP_IMPLEMENT_INSTANCE_INIT(...) \
  static inline void instance_init (gioplusplus::object::details::build_class::instance_struct<Self>* p_self, \
                                    gioplusplus::object::details::build_class::class_struct<Self>* klass) noexcept \
    { \
      ([](decltype (p_self) p_self, Type& self, decltype (klass) klass) noexcept -> void __VA_ARGS__) (p_self, p_self->self, klass); \
    }

# define GPP_IMPLEMENT_DEFAULT_INSTANCE_INIT(...) GPP_IMPLEMENT_INSTANCE_INIT({ new (&p_self->self) Type (__VA_ARGS__); })

# define GPP_IMPLEMENT_CONSTRUCTED(...) \
  static inline void class_constructed (GObject* p_self) noexcept \
    { using instance_struct = gioplusplus::object::details::build_class::instance_struct<Self>; \
      G_OBJECT_CLASS (Self::parent_class)->constructed (p_self); \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
    }

# define GPP_IMPLEMENT_DISPOSE(...) \
  static inline void class_dispose (GObject* p_self) noexcept \
    { using instance_struct = gioplusplus::object::details::build_class::instance_struct<Self>; \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
      G_OBJECT_CLASS (Self::parent_class)->dispose (p_self); \
    }

# define GPP_IMPLEMENT_FINALIZE(...) \
  static inline void class_finalize (GObject* p_self) noexcept \
    { using instance_struct = gioplusplus::object::details::build_class::instance_struct<Self>; \
      ([](instance_struct* p_self, Type& self) noexcept -> void __VA_ARGS__) ((instance_struct*) p_self, ((instance_struct*) p_self)->self); \
      G_OBJECT_CLASS (Self::parent_class)->finalize (p_self); \
    }

# define GPP_IMPLEMENT_DEFAULT_FINALIZE(...) GPP_IMPLEMENT_FINALIZE ( \
  { \
    ([](decltype (p_self) p_self, Type& self) noexcept -> void __VA_ARGS__) (p_self, p_self->self); \
    (&p_self->self)->~Type (); \
  })

# define GPP_IMPLEMENT_PROPERTY(type,...) \
  static constexpr constexpr_registry::install_once<property_first_adl_tag, \
    1 + __COUNTER__, __COUNTER__> G_GNUC_UNUSED G_PASTE (_reg_id_, __COUNTER__) {}; \
  static constexpr constexpr_registry::register_<property_adl_tag, __COUNTER__ - 2, \
    gioplusplus::object::object_class_property_##type <gioplusplus::object::details::build_class::instance_struct<Self>, __VA_ARGS__> {}> G_GNUC_UNUSED G_PASTE (_reg_prop_, __COUNTER__) {};

# define GPP_IMPLEMENT_INSTALL_PROPERTIES \
  gioplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::install (klass);

# define GPP_IMPLEMENT_DEFAULT_GET_PROPERTY \
  gioplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::get_property;

# define GPP_IMPLEMENT_DEFAULT_SET_PROPERTY \
  gioplusplus::object::details::build_class::properties_installer<Self, __COUNTER__>::set_property;