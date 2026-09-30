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
#include <gio++/object/objectclass.h>

namespace gioplusplus::object::details
{

  template<typename T>
  struct store_self { using Self = T; };

  /* { Ancestor } */

  template<typename Implementor>
  concept has_ancestor = requires () { Implementor::Ancestor; };

  template<typename Implementor>
  struct ancestor_or_default { static inline constexpr auto value = object_class_ancestor (); };

  template<has_ancestor Implementor>
  struct ancestor_or_default<Implementor> { static inline constexpr auto value = Implementor::Ancestor; };

  /* { VTable } */

  template<typename Implementor>
  concept has_vtable = requires () { typename Implementor::VTable; };

  template<typename Implementor>
  struct vtable_or_default { struct type { }; };

  template<has_vtable Implementor>
  struct vtable_or_default<Implementor> { struct type: public Implementor::VTable { }; };

  namespace build_class
    {

      template<typename Implementor>
      struct class_struct;

      template<typename Implementor>
      struct instance_struct;

      template<typename Implementor>
      struct object_class_;
    }

  template<typename Implementor>
  struct build_class::class_struct: public object_class_structs<typename Implementor::Type, ancestor_or_default<Implementor>::value>::class_struct,
                                    public vtable_or_default<Implementor>::type
    {
    };

  template<typename Implementor>
  struct build_class::instance_struct: public object_class_structs<typename Implementor::Type, ancestor_or_default<Implementor>::value>::instance_struct
    {
    };

  /* { base_class_init } */

  template<typename Implementor>
  concept has_base_init = requires (build_class::class_struct<Implementor>* c)
    { Implementor::base_class_init (c); };

  template<typename Implementor>
  struct base_init_callable { static inline constexpr auto value = nullptr; };

  template<has_base_init Implementor>
  struct base_init_callable<Implementor> { static inline constexpr auto value = [](gpointer c) noexcept -> void
    { using structs = object_class_structs<typename Implementor::Type, Implementor::Ancestor>;
      Implementor::base_class_init ((typename structs::class_struct*) c); }; };

  /* { base_class_fini } */

  template<typename Implementor>
  concept has_base_fini = requires (build_class::class_struct<Implementor>* c)
    { Implementor::base_class_fini (c); };

  template<typename Implementor>
  struct base_fini_callable { static inline constexpr auto value = nullptr; };

  template<has_base_fini Implementor>
  struct base_fini_callable<Implementor> { static inline constexpr auto value = [](gpointer c) noexcept -> void
    { using structs = object_class_structs<typename Implementor::Type, Implementor::Ancestor>;
      Implementor::base_class_fini ((typename structs::class_struct*) c); }; };

  /* { class_init } */

  template<typename Implementor>
  concept has_class_init = requires (build_class::class_struct<Implementor>* c, gpointer d)
    { Implementor::class_init (c, d); };

  template<typename Implementor>
  struct class_init_callable { static inline constexpr auto value = nullptr; };

  template<has_class_init Implementor>
  struct class_init_callable<Implementor> { static inline constexpr auto value = [](gpointer c, gpointer d) noexcept -> void
    { using structs = object_class_structs<typename Implementor::Type, Implementor::Ancestor>;
      Implementor::class_init ((typename structs::class_struct*) c, d); }; };

  /* { class_fini } */

  template<typename Implementor>
  concept has_class_fini = requires (build_class::class_struct<Implementor>* c, gpointer d)
    { Implementor::class_fini (c, d); };

  template<typename Implementor>
  struct class_fini_callable { static inline constexpr auto value = nullptr; };

  template<has_class_fini Implementor>
  struct class_fini_callable<Implementor> { static inline constexpr auto value = [](gpointer c, gpointer d) noexcept -> void
    { using structs = object_class_structs<typename Implementor::Type, Implementor::Ancestor>;
      Implementor::class_fini ((typename structs::class_struct*) c, d); }; };

  /* { instance_init } */

  template<typename Implementor>
  concept has_instance_init = requires (build_class::instance_struct<Implementor>* s,
                                        build_class::class_struct<Implementor>* c)
    { Implementor::instance_init (s, c); };

  template<typename Implementor>
  struct instance_init_callable { static inline constexpr auto value = nullptr; };

  template<has_instance_init Implementor>
  struct instance_init_callable<Implementor> { static inline constexpr auto value = [](GTypeInstance* i, gpointer c) noexcept -> void
    { using structs = object_class_structs<typename Implementor::Type, Implementor::Ancestor>;
      Implementor::instance_init ((typename structs::instance_struct*) i, (typename structs::class_struct*) c); }; };

  template<typename Implementor>
  struct build_class::object_class_
    {

      using Type = typename Implementor::Type;
      using TypeTag = typename Implementor::TypeTag;

      struct Implementor_: public Implementor
        {

          static inline constexpr auto Ancestor = ancestor_or_default<Implementor>::value;
          using VTable = typename vtable_or_default<Implementor>::type;
        };

      static inline constexpr auto Name = Implementor::ClassName;

      using type = object_class<typename build_class::class_struct<Implementor>,
                                typename build_class::instance_struct<Implementor>,
                                Name, TypeTag,
                                base_init_callable<Implementor_>::value, base_fini_callable<Implementor_>::value,
                                class_init_callable<Implementor_>::value, class_fini_callable<Implementor_>::value,
                                instance_init_callable<Implementor_>::value,
                                Implementor_::Ancestor>;
    };

  namespace build_class
    {
      template<typename Implementor>
      using object_class = typename object_class_<Implementor>::type;
    }
};

# define _GPP_IMPLEMENT(TypeName,type_name,tag,...) \
 ; \
  struct __gioplusplus_implementor_info_base_##TypeName \
    { \
 ; \
      using Type = TypeName; \
      using TypeTag = tag; \
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