# Copyright (C) 2025-2026 MarcosHCK
# This file is part of gplusplus.
#
# gplusplus is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# gplusplus is distributed in the hope that it will be useful,
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

foreach bit: [ 'bits', 'constexprregistry', 'constexprstring', 'hashing', 'slice', 'soo' ]
  __common_headers += [ '@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'base', 'concepts', 'freeable', 'object', 'variant' ]
  __common_headers += [ 'boxing@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'bytes', 'error', 'variantdict', 'variantiter' ]
  __common_headers += [ 'boxing@0@.h'.format (bit) ]
  __common_sources += [ 'boxing@0@.cc'.format (bit) ]
endforeach

install_headers (__common_headers + __common_inlines, subdir: 'gplusplus/common')

__common_lib = static_library ('gplusplus-common',
  c_args: [ '-DG_LOG_DOMAIN="Gpp.Common"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Gpp.Common"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, glib_dep, gobject_dep ],
  sources: [ __common_headers, __common_sources ])

__common_headers += __common_inlines

__common_dep = declare_dependency (dependencies: [ glib_dep, gobject_dep ],
  link_with: __common_lib)
