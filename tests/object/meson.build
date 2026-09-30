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

c_args = [ '-DG_LOG_DOMAIN="GIO++.Tests.Object"',
           '-DG_LOG_USE_STRUCTURED=1',
           '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
           '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()), ]

testpath = meson.current_source_dir ().replace (basepath + '/', '')
testpathroot = '/org/hck/gioplusplus'

test ((testpath + '/registration').replace ('/', '.'),
      executable ('registration',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'registration') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'registration') ],
            build_by_default: false,
            dependencies: [ config_dep, __object_dep, __testing_dep ],
            sources: 'registration.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ])

test ((testpath + '/layout').replace ('/', '.'),
      executable ('layout',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'layout') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'layout') ],
            build_by_default: false,
            dependencies: [ config_dep, __object_dep, __testing_dep ],
            sources: 'layout.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ])

test ((testpath + '/lifecycle').replace ('/', '.'),
      executable ('lifecycle',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'lifecycle') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'lifecycle') ],
            build_by_default: false,
            dependencies: [ config_dep, __object_dep, __testing_dep ],
            sources: 'lifecycle.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ])

test ((testpath + '/chain').replace ('/', '.'),
      executable ('chain',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'chain') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'chain') ],
            build_by_default: false,
            dependencies: [ config_dep, __object_dep, __testing_dep ],
            sources: 'chain.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ])

test ((testpath + '/vtable').replace ('/', '.'),
      executable ('vtable',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'vtable') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'vtable') ],
            build_by_default: false,
            dependencies: [ config_dep, __object_dep, __testing_dep ],
            sources: 'vtable.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ])
