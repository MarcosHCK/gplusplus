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
from argparse import ArgumentParser
from pathlib import Path
from typing import Iterable
from matplotlib import pyplot as plt
import json

def load_data (file: Path) -> tuple[str, list[float]]:

  with file.open ('r') as stream:

    if not isinstance (obj := json.load (fp = stream), dict):
      raise TypeError ('invalid root type on input (should be dict)')

    if not (arn := obj.get ('data', None)):
      raise TypeError ('missing data field on input')

    name = obj.get ('name', file.name)

    if not isinstance (arn, list) or any ((not isinstance (n, int) and not isinstance (n, float) for n in arn)):
      raise TypeError ('invalid data field on input (should be list[float])')

    return name, arn

def plot_data (iterable: Iterable[tuple[str, list[float]]]):

  plt.figure (figsize = (10, 6))

  for name, data in iterable:

    x = list (range (0, len (data)))
    plt.plot (x, data, label = name, marker = 'o', linewidth = 2, markersize = 2)

  plt.xlabel ('Tries')
  plt.ylabel ('Time (seconds)')

  plt.title ('I/O work times')
  plt.legend ()
  plt.grid (True, alpha = 0.3)

  plt.tight_layout ()
  plt.show ()

if __name__ == '__main__':

  parser = ArgumentParser ('plotter')

  parser.add_argument ('inputs', nargs = '+', metavar = '<filename>.json', type = str)

  args = parser.parse_args ()

  plot_data ((load_data (n) for n in map (Path, args.inputs)))