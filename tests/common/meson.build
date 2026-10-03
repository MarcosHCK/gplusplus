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

c_args = [ '-DG_LOG_DOMAIN="Gpp.Tests.Common"',
           '-DG_LOG_USE_STRUCTURED=1',
           '-DBUILD_DIR="@0@"'.format (meson.current_build_dir ()),
           '-DSOURCE_DIR="@0@"'.format (meson.current_source_dir ()), ]

testpath = meson.current_source_dir ().replace (basepath + '/', '')

test ((testpath + '/bytes').replace ('/', '.'),
      executable ('bytes',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'bytes') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'bytes') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'bytes.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()), ] )

test ((testpath + '/variantdict').replace ('/', '.'),
      executable ('variantdict',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'variantdict') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'variantdict') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'variantdict.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/variantiter').replace ('/', '.'),
      executable ('variantiter',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'variantiter') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'variantiter') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'variantiter.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/bits').replace ('/', '.'),
      executable ('bits',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'bits') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'bits') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'bits.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/hashing').replace ('/', '.'),
      executable ('hashing',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'hashing') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'hashing') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'hashing.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/soo').replace ('/', '.'),
      executable ('soo',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'soo') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'soo') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'soo.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )

test ((testpath + '/error').replace ('/', '.'),
      executable ('error',
            c_args: c_args +
                    [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'error') ],
            cpp_args: c_args +
                      [ '-DTESTPATHROOT="@0@"'.format (testpathroot / testpath / 'error') ],
            build_by_default: false,
            dependencies: [ config_dep, __asynclib_dep, __testing_dep ],
            sources: 'error.cc'),
      env: [ __testing_env,
             'G_TEST_BUILDDIR=@0@'.format (meson.current_build_dir ()),
             'G_TEST_SRCDIR=@0@'.format (meson.current_source_dir ()) ], )
