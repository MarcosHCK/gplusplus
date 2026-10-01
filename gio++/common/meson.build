# Copyright (C) 2025-2026 MarcosHCK
# This file is part of gio++.
#
# gio++ is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# gio++ is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

glib_dep = dependency ('glib-2.0', required: true)
gobject_dep = dependency ('gobject-2.0', required: true)

__common_headers = [ ]
__common_inlines = [ ]
__common_sources = [ ]

foreach bit: [ 'bits', 'boxing', 'constexprstring', 'hashing', 'slice', 'soo', 'unalignedstore' ]
  __common_headers += [ '@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'boxing', 'freeable', 'object' ] 
  __common_inlines += [ '@0@.inl'.format (bit) ]
endforeach

foreach bit: [ 'bytes', 'error', 'variant', 'variantdict', 'variantiter' ]
  __common_inlines += [ '@0@.inl'.format (bit) ]
  __common_sources += [ '@0@.cc'.format (bit) ]
endforeach

install_headers (__common_headers + __common_inlines, subdir: 'gio++/common')

__common_lib = static_library ('gioplusplus-common',
  c_args: [ '-DG_LOG_DOMAIN="GIO++.Common"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="GIO++.Common"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, glib_dep, gobject_dep ],
  sources: [ __common_headers, __common_sources ])

__common_headers += __common_inlines

__common_dep = declare_dependency (dependencies: [ glib_dep, gobject_dep ],
  link_with: __common_lib)
