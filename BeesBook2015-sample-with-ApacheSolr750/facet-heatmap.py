#!/usr/bin/env python

import argparse
import base64
import json
import urllib2

def compose_url(args):
  url = 'http://{0}:{1}/solr/{2}/select?facet=true'.format(args.host, args.port, args.collection)
  key = 'q'
  for val in args.q:
    url += '&{0}={1}'.format(key, val)
    key = 'fq'
  url += '&rows={0}'.format(args.rows)
  url += '&facet.heatmap={0}'.format(args.heatmap)
  url += '&facet.heatmap.format={0}'.format(args.heatmap_format)
  url += '&facet.heatmap.geom={0}'.format(args.heatmap_geom)
  url += '&facet.heatmap.gridLevel={0}'.format(args.heatmap_gridLevel)
  return url

def open_url(url):
  return urllib2.urlopen(url).read().decode('utf-8')

def counts_png(response, fieldName):
  fileContent = response.get('facet_counts',{}).get('facet_heatmaps',{}).get(fieldName,{}).get('counts_png',None)
  if not fileContent:
    print("The Solr response contained no 'counts_png' value.")
  return fileContent

def save_png(fileName, fileContent):
  if fileContent:
    with open(fileName, "wb") as file:
      file.write(base64.b64decode(fileContent.encode("utf-8")))

if __name__ == '__main__':

  parser = argparse.ArgumentParser()

  parser.add_argument('png_output_file_name', type=str, help='.png output file name')

  parser.add_argument('q', type=str, nargs='*')

  parser.add_argument('--verbose', action='store_true')

  parser.add_argument('--host', type=str, default='localhost')
  parser.add_argument('--port', type=int, default=8983)
  parser.add_argument('--collection', type=str, default='bee-hive')
  parser.add_argument('--rows', type=int, default=0)

  parser.add_argument('--heatmap', type=str, default='pos_srpt', help='facet.heatmap value')
  parser.add_argument('--heatmap-format', type=str, default='png', help='facet.heatmap.format value')
  parser.add_argument('--heatmap-geom', type=str, default='["0.0+-0.0"+TO+"0.4+0.3"]', help='facet.heatmap.geom value')
  parser.add_argument('--heatmap-gridLevel', type=int, default=6, help='facet.heatmap.gridLevel value')

  args = parser.parse_args()
  if args.verbose:
    print(args)

  url = compose_url(args)
  if args.verbose:
    print(url)

  response = open_url(url)
  if args.verbose:
    print(response)

  png_string = counts_png(json.loads(response), args.heatmap)
  if args.verbose:
    print(png_string)

  save_png(args.png_output_file_name, png_string)

