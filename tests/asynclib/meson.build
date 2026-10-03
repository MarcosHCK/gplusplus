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

c_args = [ '-DG_LOG_DOMAIN="Gpp.Tests.Asynclib"',
           '-DG_LOG_USE_STRUCTURED=1',
           '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
           '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()), ]

testpath = meson.current_source_dir ().replace (basepath + '/', '')

__test_env = [ 'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
               'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ]

test ((testpath + '/stress').replace ('/', '.'),
      executable ('stress',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'stress') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'stress') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: [ 'server.cc', 'server.h', 'stress.cc' ]),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ] )

test ((testpath + '/task').replace ('/', '.'),
      executable ('task',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'task') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'task') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'task.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ] )