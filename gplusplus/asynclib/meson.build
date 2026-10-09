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

__asynclib_headers = [ 'asynclib.h' ]
__asynclib_sources = [ ]

foreach bit: [ 'asyncfunction', 'asynctask', 'error' ]
  __asynclib_headers += [ '@0@.h'.format (bit) ]
  __asynclib_sources += [ '@0@.cc'.format (bit) ]
endforeach

foreach bit: [ 'asyncoffload' ]
  __asynclib_headers += [ '@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'begin', 'end' ]
  __asynclib_headers += [ 'asyncfunction@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'all', 'any', 'concept' ]
  __asynclib_headers += [ 'asynctask@0@.h'.format (bit) ]
endforeach

foreach bit: [ 'awaitable', 'awaitablebase', 'coroutine', 'function', 'promise', 'promiseactions' ]
  __asynclib_headers += [ 'asynctask@0@.h'.format (bit) ]
endforeach

install_headers (__asynclib_headers, subdir: 'gplusplus/asynclib')

__asynclib_lib = static_library ('gplusplus-asynclib',
  c_args: [ '-DG_LOG_DOMAIN="Gpp.Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
            '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
            '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  cpp_args: [ '-DG_LOG_DOMAIN="Gpp.Asynclib"', '-DG_LOG_USE_STRUCTURED=1',
              '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
              '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()) ],
  dependencies: [ config_dep, gio_dep ],
  sources: [ __asynclib_headers, __asynclib_sources ])

__asynclib_dep = declare_dependency (dependencies: [ gio_dep, __common_dep ],
  link_with: __asynclib_lib)