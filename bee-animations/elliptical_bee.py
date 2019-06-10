#!/usr/bin/env python

import math
import matplotlib.patches as patches

def info(thorax_x, thorax_y, bee_height, angle_radians, label_center=None):

  bee_width = float(bee_height) * 40/120

  head_width = float(bee_width) * 35/40
  head_height = float(bee_height) * 10/120

  thorax_width = float(bee_width)
  thorax_height = float(bee_height) * 40/120

  abdomen_width = float(bee_width)
  abdomen_height = float(bee_height) * 70/120

  head_to_thorax_height_half = (thorax_height + head_height) / 2
  head_x = thorax_x + math.sin(angle_radians) * -head_to_thorax_height_half
  head_y = thorax_y + math.cos(angle_radians) * +head_to_thorax_height_half

  thorax_to_abdomen_height_half = (thorax_height + abdomen_height) / 2
  abdomen_x = thorax_x + math.sin(angle_radians) * +thorax_to_abdomen_height_half
  abdomen_y = thorax_y + math.cos(angle_radians) * -thorax_to_abdomen_height_half

  def ellipse(xy, width, height):
    return patches.Ellipse(xy=xy, width=width, height=height, angle=math.degrees(angle_radians))

  ellipses = [
    ellipse([head_x, head_y], head_width, head_height),
    ellipse([thorax_x, thorax_y], thorax_width, thorax_height),
    ellipse([abdomen_x, abdomen_y], abdomen_width, abdomen_height)
  ]

  if label_center == "head":
    label_coords = [ head_x, head_y ]
  elif label_center == "thorax":
    label_coords = [ thorax_x, thorax_y ]
  elif label_center == "abdomen":
    label_coords = [ abdomen_x, abdomen_y ]
  else:
    label_coords = None

  return ellipses, label_coords

