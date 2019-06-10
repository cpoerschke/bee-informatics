#!/usr/bin/env python

import argparse
import elliptical_bee
import math
import matplotlib.pyplot as pyplot

def read_lines_map(args):

  cam_id_2_timestamp_2_lines_map = {}

  seen_header_line = False

  with open(args.input_file_name, "r") as input_file:
    for line in input_file.readlines():

      if not seen_header_line:
        seen_header_line = True
        continue # skip header line

      columns = line.replace("\n", "").split(",")

      cam_id = int(columns[args.column_index_cam_id])
      timestamp = columns[args.column_index_timestamp]

      if cam_id not in cam_id_2_timestamp_2_lines_map:
        cam_id_2_timestamp_2_lines_map[cam_id] = {}

      if timestamp not in cam_id_2_timestamp_2_lines_map[cam_id]:
        cam_id_2_timestamp_2_lines_map[cam_id][timestamp] = []

      cam_id_2_timestamp_2_lines_map[cam_id][timestamp].append(columns)

  return cam_id_2_timestamp_2_lines_map

def draw_one_bee(ax, x, y, height, angle_radians, bee_label, bee_color_alpha):

  ellipses, label_coords = elliptical_bee.info(
    thorax_x=x,
    thorax_y=y,
    bee_height=height,
    angle_radians=angle_radians,
    label_center="abdomen"
  )

  for ellipse in ellipses:
    e = ax.add_artist(ellipse)
    e.set_alpha(bee_color_alpha)
    e.set_edgecolor("black")
    e.set_facecolor("black")

  if bee_label != None:
    t = ax.text(
      x=label_coords[0],
      y=label_coords[1],
      s=bee_label,
      size="xx-small",
      horizontalalignment="center",
      verticalalignment="center"
    )

def draw_bees(args):

  cam_id_2_timestamp_2_lines_map = read_lines_map(args)

  fig = pyplot.figure()

  if args.title != None:
    fig.suptitle(args.title)

  if args.annotation != None:
    fig.text(
      args.annotation_x,
      args.annotation_y,
      args.annotation,
      size="xx-small",
      horizontalalignment="center"
    )

  ax = pyplot.subplot(aspect="equal")
  ax.set_xlim(args.x_min, args.x_max)
  ax.set_ylim(args.y_min, args.y_max)

  for cam_id in cam_id_2_timestamp_2_lines_map:
    for timestamp in cam_id_2_timestamp_2_lines_map[cam_id]:

      ax.set_title("cam_id={0} timestamp={1}".format(cam_id, timestamp))

      for line in cam_id_2_timestamp_2_lines_map[cam_id][timestamp]:
        bee_id = int(line[args.column_index_bee_id])

        bee_label = str(bee_id) if args.label_all_bees else None
        bee_color_alpha = args.spotlight_bee_color_alpha if bee_id in args.spotlight_bee_ids else args.default_bee_color_alpha

        draw_one_bee(
          ax,
          int(line[args.column_index_x_pos]),
          int(line[args.column_index_y_pos]),
          args.bee_height,
          float(line[args.column_index_orientation]) - args.bee_orientation_offset,
          bee_label,
          bee_color_alpha
        )

      break # ignore any 2nd and subsequent timestamp values

    break # ignore any 2nd and subsequent cam id values

  if args.output_file_name != None:
    pyplot.savefig(args.output_file_name, dpi=args.dots_per_inch)
  else:
    pyplot.show()

def create_parser():

  parser = argparse.ArgumentParser()

  parser.add_argument("--input-file-name", type=str, help="input file name")
  parser.add_argument("--output-file-name", type=str, default=None, help="output file name (optional)")

  parser.add_argument("--dots-per-inch", type=int, default=100)

  parser.add_argument("--column-index-timestamp", type=int, default=0)
  parser.add_argument("--column-index-x-pos", type=int, default=3)
  parser.add_argument("--column-index-y-pos", type=int, default=4)
  parser.add_argument("--column-index-orientation", type=int, default=5)
  parser.add_argument("--column-index-bee-id", type=int, default=6)
  parser.add_argument("--column-index-cam-id", type=int, default=8)

  parser.add_argument("--x-min", type=int, default=0, help="X axis minimum")
  parser.add_argument("--x-max", type=int, default=4000, help="X axis maximum")
  parser.add_argument("--y-min", type=int, default=0, help="Y axis minimum")
  parser.add_argument("--y-max", type=int, default=3000, help="Y axis maximum")

  parser.add_argument("--bee-height", type=int, default=166, help="bee height")

  parser.add_argument("--bee-orientation-offset", type=float, default=math.pi/2,
                      help="orientation adjustment to match the inputs' 0 to the elliptical_bee library's 0")

  parser.add_argument("--default-bee-color-alpha", type=float, default=0.125, help="relative lightness/darkness of a bee")
  parser.add_argument("--spotlight-bee-color-alpha", type=float, default=0.500, help="relative lightness/darkness of a bee in the spotlight")

  parser.add_argument("--spotlight-bee-ids", type=int, nargs="*", default=[], help="optional ids of bees to put in the spotlight")

  parser.add_argument("--label-all-bees", action="store_true", help="label all bees with their id")

  parser.add_argument("--title", type=str, default=None)

  parser.add_argument("--annotation", type=str, default=None)
  parser.add_argument("--annotation-x", type=int, default=0.5)
  parser.add_argument("--annotation-y", type=int, default=0.02)

  return parser

if __name__ == "__main__":

  parser = create_parser()

  args = parser.parse_args()

  draw_bees(args)

