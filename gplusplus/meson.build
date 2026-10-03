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

gio_dep = dependency ('gio-2.0', required: true)

# level 0
subdir ('common')

# level 1
subdir ('asynclib')
subdir ('object')

__gplusplus_ssl = [ ]

foreach bit: [ __asynclib_dep, __common_dep, __object_dep ]

  __gplusplus_ssl += [ bit.as_link_whole () ]
endforeach

__gplusplus_lib = library ('gplusplus',
  c_args: [ '-DG_LOG_DOMAIN="Gpp.Common"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Gpp.Common"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, __gplusplus_ssl ],
  install: true,
  soversion: '@0@.@1@'.format (git_version_major, git_version_minor),
  sources: 'dummy.cc',
  version: '@0@.@1@.@2@'.format (git_version_major, git_version_minor, git_version_micro))

__gplusplus_dep = declare_dependency (dependencies: [ gio_dep, glib_dep, gobject_dep,
                                                            config_dep.partial_dependency (compile_args: true) ],
  link_with: __gplusplus_lib)

import ('pkgconfig').generate (__gplusplus_lib,
  description: 'C++26 wrapper library around GLib/GIO with coroutine-based async support',
  filebase: 'gplusplus',
  name: 'gplusplus',
  requires: [ gio_dep, glib_dep, gobject_dep ])

meson.override_dependency ('gplusplus', __gplusplus_dep)