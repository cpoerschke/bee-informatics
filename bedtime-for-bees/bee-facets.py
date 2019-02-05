#!/usr/bin/env python

import json
import urllib2

def compose_url():

  night_and_day_q = "timestamp_dt:[2015-09-08T00:00:00.000Z+TO+2015-09-08T23:59:59.999Z]"
  day_q = "timestamp_dt:[2015-09-08T06:00:00.000Z+TO+2015-09-08T17:59:59.999Z]"
  night_q = "-("+day_q+")"

  json_facet = {
    "bees" : {
      "type" : "terms",
      "field" : "bee_id_i",
      "sort" : "index",
      "limit" : 1234567890,
      "facet" : {
        "p25_pos_delta_f" : "percentile(pos_delta_f,25)",
        "p50_pos_delta_f" : "percentile(pos_delta_f,50)",
        "p75_pos_delta_f" : "percentile(pos_delta_f,75)",
        "day" : {
          "type" : "query",
          "q" : day_q,
          "facet" : {
            "p25_pos_delta_f" : "percentile(pos_delta_f,25)",
            "p50_pos_delta_f" : "percentile(pos_delta_f,50)",
            "p75_pos_delta_f" : "percentile(pos_delta_f,75)"
          }
        },
        "night" : {
          "type" : "query",
          "q" : night_q,
          "facet" : {
            "p25_pos_delta_f" : "percentile(pos_delta_f,25)",
            "p50_pos_delta_f" : "percentile(pos_delta_f,50)",
            "p75_pos_delta_f" : "percentile(pos_delta_f,75)"
          }
        }
      }
    }
  }

  url  = 'http://localhost:8984/solr/bee-steps/select?rows=0'
  url += '&q='+night_and_day_q
  url += '&fq=bee_id_i:[*+TO+*]'
  url += '&fq=pos_delta_f:[0+TO+*]'
  url += '&json.facet={0}'.format(json.dumps( json_facet, separators=(',',':') ))
  return url

def flatten_facets(facets, info, prefix=None):
  for key in facets:
    val = facets[key]
    if isinstance(val, dict):
      flatten_facets(val, info, prefix=key)
    else:
      info[prefix+"."+key if prefix else key] = val
  return info

if __name__ == '__main__':

  # get facet buckets
  url = compose_url()
  response = urllib2.urlopen(url).read().decode('utf-8')
  buckets = json.loads(response).get("facets", {}).get("bees",{}).get("buckets", [])

  # extract info
  infos = []
  for bucket in buckets:
    infos.append(flatten_facets(bucket, {}, prefix="bee"))

  # calculate ratios
  for suffix in ["count", "p25_pos_delta_f", "p50_pos_delta_f", "p75_pos_delta_f"]:
    day_key = "day."+suffix
    night_key = "night."+suffix
    for info in infos:
      if day_key in info and night_key in info:
        if info[day_key]:
          info[night_key+"/"+day_key] = info[night_key] / info[day_key]
        if info[night_key]:
          info[day_key+"/"+night_key] = info[day_key] / info[night_key]

  # compute union of all headers
  headers = set()
  for info in infos:
    headers.update(info.keys())

  # sort headers
  headers.remove("bee.count")
  headers.remove("bee.val")
  headers = sorted(list(headers))
  headers.insert(0, "bee.val")
  headers.insert(0, "bee.count")

  # compose lines
  lines = [ ','.join(headers) ]
  for info in infos:
    vals = []
    for key in headers:
      vals.append(str(info[key]) if key in info else '')
    lines.append( ','.join(vals) )

  # save lines
  with open("bee-facets.csv", 'w') as file:
    for line in lines:
      file.write(line+"\n")

