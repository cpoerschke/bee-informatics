#!/usr/bin/env python

import argparse
import json
import matplotlib.pyplot as plt
import urllib2

def compose_url(args):
  url = 'http://{0}:{1}/solr/{2}/select?rows={3}'.format(args.host, args.port, args.collection, args.rows)
  key = 'q'
  for val in args.q:
    url += '&{0}={1}'.format(key, val)
    key = 'fq'
  url += '&sort={0}'.format(args.sort)
  url += '&fl={0},{1},{2}'.format(args.x_pos, args.y_pos, args.orientation)
  return url

def open_url(url):
  return urllib2.urlopen(url).read().decode('utf-8')

def positions_arr(response):
  return response.get('response',{}).get('docs',[])

def plot_positions(fileName, positions, x_pos_key, y_pos_key, style, axis, orientation_key):

  x_vals = []
  y_vals = []
  for ii in range(0,len(positions)):
    if orientation_key:
      x_vals.append(ii+1)
      y_vals.append(positions[ii].get(orientation_key))
    else:
      x_vals.append(positions[ii].get(x_pos_key))
      y_vals.append(positions[ii].get(y_pos_key))

  plt.plot(x_vals, y_vals, style)
  if axis:
    plt.axis(axis)
  plt.savefig(fileName)

if __name__ == '__main__':

  parser = argparse.ArgumentParser()

  parser.add_argument('png_output_file_name', type=str, help='.png output file name')

  parser.add_argument('q', type=str, nargs='*')

  parser.add_argument('--verbose', action='store_true')

  parser.add_argument('--host', type=str, default='localhost')
  parser.add_argument('--port', type=int, default=8983)
  parser.add_argument('--collection', type=str, default='bee-hive')
  parser.add_argument('--rows', type=int, default=0)

  parser.add_argument('--sort', type=str, default='timestamp_dt+asc', help='sort value')
  parser.add_argument('--x-pos', type=str, default='x_pos_i')
  parser.add_argument('--y-pos', type=str, default='y_pos_i')
  parser.add_argument('--orientation', type=str, default=None)

  parser.add_argument('--style', type=str, default='k.-')
  parser.add_argument('--axis', type=int, nargs='*')

  args = parser.parse_args()
  if args.verbose:
    print(args)

  url = compose_url(args)
  if args.verbose:
    print(url)

  response = open_url(url)
  if args.verbose:
    print(response)

  vals = positions_arr(json.loads(response))
  if args.verbose:
    print(vals)

  plot_positions(args.png_output_file_name, vals, args.x_pos, args.y_pos, args.style, args.axis, args.orientation)

