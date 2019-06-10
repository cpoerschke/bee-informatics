#!/usr/bin/env python

import argparse
import datetime
import elliptical_bee
import math
import matplotlib.animation as animation
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

def draw_one_bee(ax, x, y, height, angle_radians, bee_label, bee_color, bee_color_alpha):

  elements = []

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
    e.set_edgecolor(bee_color)
    e.set_facecolor(bee_color)
    elements.append(e)

  if bee_label != None:
    t = ax.text(
      x=label_coords[0],
      y=label_coords[1],
      s=bee_label,
      size="xx-small",
      horizontalalignment="center",
      verticalalignment="center"
    )
    elements.append(t)

  return elements

def animate_bees(args):

  cam_id_2_timestamp_2_lines_map = read_lines_map(args)

  bee_id_2_focal_bee_meeting_count = {}

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

  writer = animation.writers["ffmpeg"](fps=args.frames_per_second)
  with writer.saving(fig, args.output_file_name, dpi=args.dots_per_inch):

    axes = []

    if len(args.cam_ids) == 1:
      axes.append(pyplot.subplot(aspect="equal"))
    else:
      for ii in range(0,len(args.cam_ids)):
        ax = pyplot.subplot(args.subplot_nrows_ncols_index[ii], aspect="equal")
        ax.xaxis.set_label_position(args.x_label_position[ii])
        ax.yaxis.set_label_position(args.y_label_position[ii])
        ax.set_xticks([])
        ax.set_yticks([])
        axes.append(ax)

    for ax in axes:
      ax.set_xlim(args.x_min, args.x_max)
      ax.set_ylim(args.y_min, args.y_max)

    timestamp_counts = []
    for ii in range(0,len(axes)):
      cam_id = args.cam_ids[ii]
      timestamp_counts.append(len(cam_id_2_timestamp_2_lines_map[cam_id].keys()))

    # assumption: all cameras (of interest) recorded at the same rate and duration
    if min(timestamp_counts) != max(timestamp_counts):
      print("timestamp_counts = {0}".format(timestamp_counts))

    timestamp_count = min(timestamp_counts)

    for tt in range(0, timestamp_count):

      if args.progress_report_interval and tt % args.progress_report_interval == 0:
        print("{0}/{1}".format(tt+1, timestamp_count))

      elements = []

      for ii in range(0,len(axes)):
        cam_id = args.cam_ids[ii]
        ax = axes[ii]

        sorted_timestamps = sorted(cam_id_2_timestamp_2_lines_map[cam_id].keys())
        timestamp = sorted_timestamps[tt]

        if args.input_timestamp_format and args.output_timestamp_format:
          display_timestamp = datetime.datetime.strptime(timestamp, args.input_timestamp_format).strftime(args.output_timestamp_format)
        else:
          display_timestamp = timestamp

        if len(args.cam_ids) == 1:
          ax.set_title("cam_id={0} timestamp={1}".format(cam_id, display_timestamp))
        else:
          ax.set_xlabel(display_timestamp)
          ax.set_ylabel("cam_id "+str(cam_id))

        lines = cam_id_2_timestamp_2_lines_map[cam_id][timestamp]

        focal_bee_line = None
        if args.focal_bee_id != None:
          for line in lines:
            bee_id = int(line[args.column_index_bee_id])
            if bee_id == args.focal_bee_id:
              focal_bee_line = line
              bee_id_2_focal_bee_meeting_count[bee_id] = bee_id_2_focal_bee_meeting_count.get(bee_id, 0) + 1
              break

        for line in lines:
          bee_id = int(line[args.column_index_bee_id])

          if bee_id != args.focal_bee_id and args.focal_bee_meet_radius != None and focal_bee_line != None:

            x_delta = int(focal_bee_line[args.column_index_x_pos]) - int(line[args.column_index_x_pos])
            y_delta = int(focal_bee_line[args.column_index_y_pos]) - int(line[args.column_index_y_pos])

            if math.sqrt(pow(x_delta,2) + pow(y_delta,2)) <= args.focal_bee_meet_radius:
              bee_id_2_focal_bee_meeting_count[bee_id] = bee_id_2_focal_bee_meeting_count.get(bee_id, 0) + 1

          if bee_id == args.focal_bee_id:

            bee_label = None if args.label_no_bees else str(bee_id)
            bee_color = args.focal_bee_color
            bee_alpha = args.focal_bee_color_alpha

          elif bee_id in bee_id_2_focal_bee_meeting_count:

            bee_label = None if args.label_no_bees else str(bee_id)
            bee_color = args.meet_bee_color
            bee_alpha = args.default_bee_color_alpha + (args.focal_bee_color_alpha - args.default_bee_color_alpha) * float(bee_id_2_focal_bee_meeting_count[bee_id])/float(bee_id_2_focal_bee_meeting_count[args.focal_bee_id])

          else:

            bee_label = None # don't label bees that have not (yet) met the focal bee
            bee_color = args.default_bee_color
            bee_alpha = args.default_bee_color_alpha

          line_elements = draw_one_bee(
            ax,
            int(line[args.column_index_x_pos]),
            int(line[args.column_index_y_pos]),
            args.bee_height,
            float(line[args.column_index_orientation]) - args.bee_orientation_offset,
            bee_label,
            bee_color,
            bee_alpha
          )

          elements.extend(line_elements)

      if (args.frame_timestamp_min == None or args.frame_timestamp_min <= timestamp) and \
         (args.frame_timestamp_max == None or args.frame_timestamp_max >= timestamp):
        writer.grab_frame()

      for element in elements:
        element.remove()

  if args.progress_report_interval:
    print("{0}/{1}".format(tt+1, timestamp_count))

def create_parser():

  parser = argparse.ArgumentParser()

  parser.add_argument("--input-file-name", type=str, help="input file name")
  parser.add_argument("--output-file-name", type=str, help="output file name")

  parser.add_argument("--progress-report-interval", type=int, default=None)

  parser.add_argument("--dots-per-inch", type=int, default=100)
  parser.add_argument("--frames-per-second", type=int, default=3)

  parser.add_argument("--frame-timestamp-min", type=str, default=None)
  parser.add_argument("--frame-timestamp-max", type=str, default=None)

  parser.add_argument("--input-timestamp-format", type=str, default="%Y-%m-%d %H:%M:%S.%f+00", help="format of the timestamps in the input file")
  parser.add_argument("--output-timestamp-format", type=str, default="%Y-%m-%d %H:%M:%S.%f", help="format for the timestamps in the output animation")

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
  parser.add_argument("--focal-bee-color-alpha", type=float, default=0.875, help="relative lightness/darkness of the focal bee")

  parser.add_argument("--focal-bee-id", type=int, default=None, help="(optional) id of the focal bee")
  parser.add_argument("--focal-bee-meet-radius", type=int, default=None, help="(optional) distance between the focal bee and a bee that 'meets' the focal bee")

  parser.add_argument("--focal-bee-color", type=str, default="red")
  parser.add_argument("--meet-bee-color", type=str, default="blue")
  parser.add_argument("--default-bee-color", type=str, default="black")

  parser.add_argument("--cam-ids", type=int, nargs="*", default=[ 0, 1, 2, 3 ])
  parser.add_argument("--subplot-nrows-ncols-index", type=int, nargs="*", default=[ 221, 222, 224, 223 ])
  parser.add_argument("--x-label-position", type=str, nargs="*", default=[ "top", "top", "bottom", "bottom" ])
  parser.add_argument("--y-label-position", type=str, nargs="*", default=[ "left", "right", "right", "left" ])

  parser.add_argument("--label-no-bees", action="store_true", help="don't label bees with their id")

  parser.add_argument("--title", type=str, default=None)

  parser.add_argument("--annotation", type=str, default=None)
  parser.add_argument("--annotation-x", type=int, default=0.5)
  parser.add_argument("--annotation-y", type=int, default=0.02)

  return parser

if __name__ == "__main__":

  parser = create_parser()

  args = parser.parse_args()

  animate_bees(args)

