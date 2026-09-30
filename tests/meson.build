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

basepath = meson.current_source_dir ()
testpathroot = '/org/hck/gioplusplus/asynclib'

gio_dep = dependency ('gio-2.0', required: true)

__testing_lib = static_library ('testing',
  c_args: [ '-DG_LOG_DOMAIN="GIO++.Tests"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()),
            '-DTESTPATHROOT="@0@"'.format (testpathroot) ],
  cpp_args: [ '-DG_LOG_DOMAIN="GIO++.Tests"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()),
              '-DTESTPATHROOT="@0@"'.format (testpathroot) ],
  build_by_default: false,
  dependencies: [ config_dep, gio_dep ],
  sources: [ 'testing.cc', 'testing.h' ])

__testing_dep = declare_dependency (dependencies: [ config_dep, gio_dep ],
  link_with: __testing_lib)

__testing_env = [ ]

if get_option ('b_sanitize') == 'address,undefined' or get_option ('b_sanitize').contains ('address')

  # Work around an upstream GLib happy-eyeballs leak (see glib.supp) so the
  # sanitizer gate stays green without distorting the plain-GLib baseline.
  __testing_env += [ 'LSAN_OPTIONS=suppressions=@0@'.format (meson.current_source_dir () / 'glib.supp') ]
endif

subdir ('asynclib')
subdir ('common')
subdir ('object')
