#!/usr/bin/env python

import argparse
import elliptical_bee
import math
import matplotlib.pyplot as pyplot

if __name__ == "__main__":

  parser = argparse.ArgumentParser()

  parser.add_argument("--thorax-x", type=int, default=0, help="X position of bee's thorax")
  parser.add_argument("--thorax-y", type=int, default=0, help="Y position of bee's thorax")
  parser.add_argument("--bee-height", type=float, default=12.0, help="height of bee")
  parser.add_argument("--angle-degrees", type=int, default=0, help="angle (in degrees) of bee")
  parser.add_argument("--label-center", type=str, default="thorax", help="bee label location: head, thorax or abdomen")
  parser.add_argument("--output-file-name", type=str, default=None, help="optional output file name")

  args = parser.parse_args()

  ax = pyplot.subplot(aspect="equal")
  ax.set_xlim(-10, +10)
  ax.set_ylim(-10, +10)

  ellipses, label_coords = elliptical_bee.info(
    thorax_x=args.thorax_x,
    thorax_y=args.thorax_y,
    angle_radians=math.radians(args.angle_degrees),
    bee_height=args.bee_height,
    label_center=args.label_center
  )

  for ellipse in ellipses:
    print(ellipse)
    ax.add_artist(ellipse)

  if label_coords != None:
    print('label_coords='+str(label_coords))
    ax.text(
      x=label_coords[0],
      y=label_coords[1],
      s="label",
      horizontalalignment="center",
      verticalalignment="center"
    )

  if args.output_file_name != None:
    pyplot.savefig(args.output_file_name)
  else:
    pyplot.show()

