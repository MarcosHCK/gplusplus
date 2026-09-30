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

__object_headers = [ ]
__object_sources = [ ]

foreach bit: [ 'objectclass' ] 
  __object_headers += [ '@0@.h'.format (bit) ]
  __object_sources += [ '@0@.cc'.format (bit) ]
endforeach

foreach bit: [ 'object', 'objectimplementor' ]
  __object_headers += [ '@0@.h'.format (bit) ]
endforeach

install_headers (__object_headers, subdir: 'gio++/object')

__object_lib = static_library ('gioplusplus-object',
  c_args: [ '-DG_LOG_DOMAIN="GIO++.Object"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="GIO++.Object"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, gobject_dep, __common_dep ],
  sources: [ __object_headers, __object_sources ])

__object_dep = declare_dependency (dependencies: [ gobject_dep, __common_dep ],
  link_with: __object_lib)