#!/usr/bin/env python

import argparse
import json
import matplotlib.pyplot as plt
import numpy as np
import urllib2

timestamp_range_facet_key = "timestamp_range_facet"
bee_terms_facet_key = "bee_terms_facet_key"
pos_delta_percentile_key_format = "p{0}"
pos_delta_sum_key = "sum"

def compose_pos_delta_facets(args):
  pos_delta_facets = {}
  for pp in args.pos_delta_percentile:
    key = pos_delta_percentile_key_format.format(pp)
    val = "percentile({0},{1})".format(args.pos_delta_key, pp)
    pos_delta_facets[key] = val
  if len(pos_delta_facets) == 0:
    pos_delta_facets[pos_delta_sum_key] = "sum({0})".format(args.pos_delta_key)

  if args.multi_bee_percentile != None:
    return {
        bee_terms_facet_key : {
          "field" : args.bee_id_key,
          "type" : "terms",
          "limit" : 1234567890,
          "facet" : pos_delta_facets
        }
    }
  else:
    return pos_delta_facets

def compose_json_facet(args):
  json_facet = {
    timestamp_range_facet_key : {
      "field" : args.timestamp_key,
      "type" : "range",
      "start" : args.timestamp_range_start,
      "end"   : args.timestamp_range_end,
      "gap" : args.timestamp_range_gap,
      "include" : "edge",
      "facet" : compose_pos_delta_facets(args)

    }
  }
  return json.dumps( json_facet, separators=(',',':') )

def compose_url(args):
  url = 'http://{0}:{1}/solr/{2}/select?rows=0'.format(args.host, args.port, args.collection)
  key = 'q'
  for val in args.q:
    url += '&{0}={1}'.format(key, val)
    key = 'fq'
  url += '&json.facet={0}'.format(compose_json_facet(args))
  if args.verbose:
    url += '&indent=on'
  return url

def metric_values_map(buckets, metrics):

  vals_map = {}

  def get_val(bucket, key):
    if key in bucket:
      return bucket[key]
    else:
      return None

  def add_bucket_vals(key):
    vals_map[key] = { "x" : [], "y" : [] }
    for ii in range(0,len(buckets)): # time slices
      val = get_val(buckets[ii], key)
      if val != None:
        vals_map[key]["x"].append(ii)
        vals_map[key]["y"].append(val)

  # for each metric get two lists:
  # --> x list is the time slots
  # --> y list is the metric value for each time slot

  for metric in metrics:
    add_bucket_vals(metric)

  return vals_map

def metric_percentiles_map(buckets, metrics, multi_bee_percentiles):

  val_lists_map = {}

  def get_vals(bucket_desc, sub_buckets, key):
    vals = []
    for sub_bucket in sub_buckets: # bee terms
      if key in sub_bucket:
        vals.append(sub_bucket[key])
    if 0 < len(vals):
      return vals
    else:
      print("Found no '{0}' data for '{1}' time slice.".format(key, bucket_desc))
      return None

  def add_bucket_val_lists(key):
    val_lists_map[key] = { "x" : [], "y" : [] }
    for ii in range(0,len(buckets)): # time slices
      bucket_desc = buckets[ii].get("val")
      sub_buckets = buckets[ii].get(bee_terms_facet_key,{}).get("buckets",[])
      vals = get_vals(bucket_desc, sub_buckets, key)
      if vals != None:
        val_lists_map[key]["x"].append(ii)
        val_lists_map[key]["y"].append(vals)

  # for each metric collate two lists:
  # --> x list is the time slots
  # --> y list is the 'metric values list' for each time slot
  #     the 'metric values list' contains the metrics (for that time slot) from multiple individual bees

  for metric in metrics:
    add_bucket_val_lists(metric)

  vals_map = {}

  # for each metric and percentile calculate two lists:
  # --> x list is the time slots
  # --> y list is the 'metric percentile' value for each time slot
  #     the 'metric percentile' is a percentile of the 'metric values list'

  for metric in val_lists_map:
    for pp in multi_bee_percentiles:
      pp_metric = 'p{0}({1})'.format(pp,metric)
      vals_map[pp_metric] = {
        "x" : val_lists_map[metric]["x"],
        "y" : [ np.percentile(val_list,pp) for val_list in val_lists_map[metric]["y"] ]
      }

  return vals_map

def plot_vals_map(args, vals_map):

  if args.plot_title != None:
    plt.title(args.plot_title)

  num_plot_lines = 0

  for key in vals_map:
    x_vals = vals_map[key]["x"]
    y_vals = vals_map[key]["y"]
    if args.plot_y_multiplier != None:
      y_vals = [ y_val * args.plot_y_multiplier for y_val in y_vals ]
    if 0 < len(x_vals) * len(y_vals):
      plt.plot(x_vals, y_vals, args.plot_fmt, linestyle=args.plot_linestyle, marker=args.plot_marker)
      num_plot_lines += 1

  if args.plot_axis != None:
    plt.axis(args.plot_axis)

  if 0 < num_plot_lines:
    plt.savefig(args.png_output_file_name)
    return args.png_output_file_name
  else:
    print("Found no {0} data.".format(sorted(vals_map.keys())))
    return None

if __name__ == '__main__':

  parser = argparse.ArgumentParser()

  parser.add_argument('png_output_file_name', type=str, help='.png output file name')

  parser.add_argument('q', type=str, nargs='*')

  parser.add_argument('--verbose', action='store_true')

  parser.add_argument('--multi-bee-percentile', type=int, nargs='*', default=None)

  parser.add_argument('--host', type=str, default='localhost')
  parser.add_argument('--port', type=int, default=8984)
  parser.add_argument('--collection', type=str, default='bee-steps')

  parser.add_argument('--bee-id-key', type=str, default='bee_id_i')
  parser.add_argument('--timestamp-key', type=str, default='timestamp_dt')
  parser.add_argument('--pos-delta-key', type=str, default='pos_delta_f')

  parser.add_argument('--pos-delta-percentile', type=int, nargs='*', default=[])

  parser.add_argument('--timestamp-range-start', type=str, default='2015-09-01T00:00:00.000Z')
  parser.add_argument('--timestamp-range-end', type=str, default='2015-09-30T23:59:59.999Z')
  parser.add_argument('--timestamp-range-gap', type=str, default='%2B1HOUR')

  parser.add_argument('--plot-axis', type=int, nargs='*', default=None)
  parser.add_argument('--plot-fmt', type=str, default='k.-')
  parser.add_argument('--plot-linestyle', type=str, default='-')
  parser.add_argument('--plot-marker', type=str, default='.')
  parser.add_argument('--plot-title', type=str, default=None)

  parser.add_argument('--plot-y-multiplier', type=float, default=None)

  args = parser.parse_args()
  if args.verbose:
    print(args)

  url = compose_url(args)
  if args.verbose:
    print(url)

  response = urllib2.urlopen(url).read().decode('utf-8')
  if args.verbose:
    print(response)

  buckets = json.loads(response).get("facets", {}).get(timestamp_range_facet_key,{}).get("buckets",[])
  if args.verbose:
    print(buckets)

  metrics = [ pos_delta_sum_key ]
  for pp in args.pos_delta_percentile:
    metrics.append(pos_delta_percentile_key_format.format(pp))
  if args.verbose:
    print(metrics)

  if args.multi_bee_percentile != None:
    vals_map = metric_percentiles_map(buckets, metrics, args.multi_bee_percentile)
  else:
    vals_map = metric_values_map(buckets, metrics)

  if args.verbose:
    print(vals_map)
  png_file = plot_vals_map(args, vals_map)
  if args.verbose:
    print(png_file)

