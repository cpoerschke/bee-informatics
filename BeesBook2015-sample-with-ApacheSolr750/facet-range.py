#!/usr/bin/env python

import argparse
import json
import matplotlib.pyplot as plt
import urllib2

def compose_url(args):
  url = 'http://{0}:{1}/solr/{2}/select?facet=true'.format(args.host, args.port, args.collection)
  key = 'q'
  for val in args.q:
    url += '&{0}={1}'.format(key, val)
    key = 'fq'
  url += '&rows={0}'.format(args.rows)
  url += '&facet.mincount={0}'.format(args.facet_mincount)
  url += '&facet.range={0}'.format(args.facet_range)
  url += '&facet.range.gap={0}'.format(args.facet_range_gap)
  url += '&facet.range.start={0}'.format(args.facet_range_start)
  url += '&facet.range.end={0}'.format(args.facet_range_end)
  return url

def open_url(url):
  return urllib2.urlopen(url).read().decode('utf-8')

def counts_arr(response, fieldName):
  return response.get('facet_counts',{}).get('facet_ranges',{}).get(fieldName,{}).get('counts',[])

def plot_counts(fileName, url, counts, xtick=60):
  vals = []
  for ii in range(0,len(counts)):
    if ii%2 == 1:
      vals.append(counts[ii])

  x_lefts = []
  x_widths = []
  y_heights = []
  for ii in range(0,len(vals)):
    x_lefts.append(ii)
    x_widths.append(1)
    y_heights.append(1 if vals[ii] else 0)

  fig = plt.figure()

  ax = fig.add_axes((0, 0, 1.0, 1.0))
  ax.set_xticks(range(0, len(vals)+xtick, xtick))
  ax.set_yticks([0,1])
  ax.set_xlim((-len(vals)/10,11*len(vals)/10))
  ax.set_ylim(-0.2, 2.2)
  ax.bar(x_lefts, y_heights, x_widths, color="black")

  fig.text(1.0/12, 2.0/3, url.replace('&','\n&'))

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

  parser.add_argument('--facet-mincount', type=int, default=0, help='facet.mincount value')
  parser.add_argument('--facet-range', type=str, default='timestamp_dt', help='facet.range value')
  parser.add_argument('--facet-range-start', type=str, default='2015-09-01T00:00:00.000Z', help='facet.range.start value')
  parser.add_argument('--facet-range-end', type=str, default='2015-09-30T23:59:59.999Z', help='facet.range.end value')
  parser.add_argument('--facet-range-gap', type=str, default='%2B1MINUTE', help='facet.range.gap value')

  args = parser.parse_args()
  if args.verbose:
    print(args)

  url = compose_url(args)
  if args.verbose:
    print(url)

  response = open_url(url)
  if args.verbose:
    print(response)

  vals = counts_arr(json.loads(response), args.facet_range)
  if args.verbose:
    print(vals)

  plot_counts(args.png_output_file_name, url, vals)

