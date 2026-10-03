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
from socket import socket, AF_INET, SO_REUSEADDR, SOCK_STREAM, SOL_SOCKET
from hashlib import sha256
import os, re, sys

def parse_size (size_str):

  units = {
    'B': 1, 'KB': 1000, 'MB': 1000**2, 'GB': 1000**3, 'TB': 1000**4,
    'KiB': 1024, 'MiB': 1024**2, 'GiB': 1024**3, 'TiB': 1024**4,
    'K': 1000, 'M': 1000**2, 'G': 1000**3  # shorthand
  }

  if not (match := re.match (r'([\d.]+)\s*([A-Za-iK-Z]+)?', size_str.strip ())):
    raise ValueError (f'Invalid size format: {size_str}')

  value, unit = match.groups ()

  if not (factor := units.get (unit or 'B', None)):
    raise ValueError (f'Invalid size format: {size_str}')

  return int (factor * float (value))

header = b'GET / HTTP/1.1\r\n' \
       + b'Connection: close\r\n' \
       + b'\r\n'

def serve (socket: socket, blob: bytes):

  while True:

    client, address = socket.accept ()

    print (f'[{address}] incoming', file = sys.stderr)

    try:

      if header != (got := client.recv (len (header))):
        raise Exception (f'bad header \'{got}\' ({len (got)} bytes)')

    except Exception as e:
      print (f'[{address}] read()!: {str (e)}', file = sys.stderr)
      client.close ()
      continue

    try:
      client.sendall (blob)
    except Exception as e:
      print (f'[{address}] write()!: {str (e)}', file = sys.stderr)
      client.close ()
      continue

    client.close ()

if __name__ == '__main__':

  parser = ArgumentParser ('http_server')

  parser.add_argument ('-b', '--bind', default= '', type = str)
  parser.add_argument ('-p', '--port', default = 8000, type = int)
  parser.add_argument ('-s', '--size', default = '1MiB', type = str)

  args = parser.parse_args ()
  blob = os.urandom (parse_size (args.size))
  hash = sha256 (blob, usedforsecurity = False)

  server_socket = socket (AF_INET, SOCK_STREAM)
  server_socket.setsockopt (SOL_SOCKET, SO_REUSEADDR, 1)
  server_socket.bind ((args.bind, args.port))
  server_socket.listen (5)

  print (f'{hash.hexdigest ()},{args.bind},{args.port}')
  sys.stdout.close ()

  try:
    serve (server_socket, blob)
  except KeyboardInterrupt:
    pass
  finally:
    server_socket.close ()