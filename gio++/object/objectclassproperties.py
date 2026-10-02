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
from argparse import ArgumentParser
from dataclasses import dataclass
from itertools import chain
from pathlib import Path
from typing import Iterable, NamedTuple, TextIO

class StructForVariable:

  name: str
  expr: str
  type_: str

  def __init__ (self, name: str, type_: str, expr: str | None = None) -> None:

    self.name = name
    self.expr = name if not expr else expr
    self.type_ = type_

  def to_template_argument (self):
    return f'{self.type_} {self.name}'

@dataclass
class StructForDescriptor:

  name: str
  variables: list[StructForVariable]

def make_numeric_descriptor (name: str, type_: str | None = None):

  type_ = type_ if None != type_ else f'g{name}'

  variables = [ StructForVariable ('MinimumValue', str (type_)),
                StructForVariable ('MaximumValue', str (type_)),
                StructForVariable ('DefaultValue', str (type_)) ]

  return StructForDescriptor (name = name, variables = variables)

descriptors = [

  StructForDescriptor (name = 'boolean', variables = [ StructForVariable ('DefaultValue', 'gboolean') ]),
  StructForDescriptor (name = 'boxed', variables = [ StructForVariable ('GetType', 'details::invocable_r<GType>', 'GetType ()') ]),

  make_numeric_descriptor ('char'),
  make_numeric_descriptor ('double'),

  StructForDescriptor (name = 'enum', variables = [ StructForVariable ('GetType', 'details::invocable_r<GType>', 'GetType ()'),
                                                    StructForVariable ('DefaultValue', 'gint') ]),

  StructForDescriptor (name = 'flags', variables = [ StructForVariable ('GetType', 'details::invocable_r<GType>', 'GetType ()'),
                                                     StructForVariable ('DefaultValue', 'gint') ]),

  make_numeric_descriptor ('float'),

  StructForDescriptor (name = 'gtype', variables = [ StructForVariable ('IsAType', 'details::invocable_r<GType>', 'IsAType ()') ]),

  make_numeric_descriptor ('int'),
  make_numeric_descriptor ('int64'),

  make_numeric_descriptor ('long'),

  StructForDescriptor (name = 'object', variables = [ StructForVariable ('GetType', 'details::invocable_r<GType>', 'GetType ()') ]),
  StructForDescriptor (name = 'param', variables = [ StructForVariable ('GetType', 'details::invocable_r<GType>', 'GetType ()') ]),

  StructForDescriptor (name = 'pointer', variables = [ ]),
  StructForDescriptor (name = 'string', variables = [ StructForVariable ('DefaultValue', 'constexpr_string') ]),

  make_numeric_descriptor ('uchar'),

  make_numeric_descriptor ('uint'),
  make_numeric_descriptor ('uint64'),

  make_numeric_descriptor ('ulong'),
  make_numeric_descriptor ('unichar'),

  StructForDescriptor (name = 'variant', variables = [ StructForVariable ('GetType', 'details::invocable_r<const GVariantType*>', 'GetType ()'),
                                                       StructForVariable ('GetDefaultValue', 'details::invocable_r<GVariant*>', 'GetDefaultValue ()') ]),
]

def struct_for (desc: StructForDescriptor):

  creator_expr = ', '.join (chain (( var.expr for var in desc.variables)))
  descend_expr = ', '.join (( var.name for var in desc.variables))
  template_expr = ', '.join (( var.to_template_argument () for var in desc.variables))

  E = lambda s, p = None: "" if 0 == len (template_expr) else (p or ",")

  d = f'''\
  namespace details
   {{

      template<constexpr_string Name, constexpr_string Nick, constexpr_string Blurb{E (template_expr)}
               {template_expr}>
      static inline constexpr auto _create_{desc.name}_spec = [](GParamFlags flags) noexcept {{ return g_param_spec_{desc.name} (Name, Nick, Blurb, {creator_expr}{E (creator_expr, ", ")}flags); }};
    }}

  template<typename InstanceType,
           constexpr_string Name, constexpr_string Nick, constexpr_string Blurb,
           {template_expr}{E (template_expr)}
           details::property_tag PropertyTag = property_tag::normal,
           details::invocable_r_or_null<void, InstanceType*, GValue*, GParamSpec*> auto GetProperty = nullptr,
           details::invocable_r_or_null<void, InstanceType*, const GValue*, GParamSpec*> auto SetProperty = nullptr>
  using object_class_property_{desc.name} = object_class_property<InstanceType, details::_create_{desc.name}_spec<Name, Nick, Blurb{E (descend_expr)}{descend_expr}>, PropertyTag, GetProperty, SetProperty>;'''

  return ( f'{l}\r\n'  for l in d.splitlines () )

def read_till (stream: TextIO, stop: str, suffix: str = ''):

  at = -1

  while -1 == at:

    if 0 == len (line := stream.readline ()):
      raise Exception (f'missing stop token {stop}')

    if -1 == (at := line.find (stop)):

      yield line
    else:
      yield line [:at + len (stop)] + suffix

def read_preamble (file: Path):

  with file.open ('rt') as stream:

    if 0 == len (ch := stream.read (2)) or '/*' != ch:
      return

    return chain ([ '/*' ], list (read_till (stream, '*/', '\r\n')))

from sys import stdout

def write_output (output: str, iter: Iterable[str]):

  if ('-' == output):

    stdout.writelines (iter)
  else:

    with Path (output).open ('wt') as stream:
      stream.writelines (iter)

if __name__ == '__main__':

  parser = ArgumentParser ()

  parser.add_argument ('header', help = 'Basic header file', metavar = 'HEADER', type = str)
  parser.add_argument ('-b', '--basedir', default = '.', help = 'Base dir', metavar = 'DIR', type = str)
  parser.add_argument ('-o', '--output', default = '-', help = '')

  args = parser.parse_args ()

  basedir = Path (args.basedir)
  header = Path (args.header).resolve ()
  include = header.relative_to (basedir.resolve (), walk_up = True)

  preamble = p if (p := read_preamble (header)) else [ '' ]
  contents = chain (*(chain (struct_for (d), [ '\r\n' ]) for d in descriptors))
  contents = chain (( f'{s}\r\n' for s in ('#pragma once', f'#include <{include}>', '', 'namespace gioplusplus::object', '{\r\n') ), contents, '}\r\n')

  write_output (args.output, chain (preamble, contents))