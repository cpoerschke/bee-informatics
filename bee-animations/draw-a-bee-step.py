#!/usr/bin/env python

import matplotlib.patches as patches
import matplotlib.pyplot as pyplot
import sys

step = None
if 1 < len(sys.argv):
  step = int(sys.argv[1])

fig, ax = pyplot.subplots()
ax.set_aspect('equal')
ax.set_xlim(-15, +15)
ax.set_ylim(-15, +15)

ax.set_xticks([])
ax.set_yticks([])

shapes = []

ax.set_title('Step ' + str(step))

if 1 <= step:
  shapes.append(patches.Circle(xy=[0, 0], radius=3))

if 2 == step:
  shapes.append(patches.Ellipse(xy=[0, 0], width=6, height=12, linestyle='dotted'))

if 3 <= step:
  shapes.append(patches.Ellipse(xy=[0,-9], width=6, height=12))

if 4 <= step:
  shapes.append(patches.Ellipse(xy=[0, 4], width=6, height=2))

if 5 == step:
  ax.text(x=0, y=4, s='head', horizontalalignment='center', verticalalignment='center')
  ax.text(x=0, y=0, s='thorax', horizontalalignment='center', verticalalignment='center')
  ax.text(x=0, y=-9, s='abdomen', horizontalalignment='center', verticalalignment='center')

for shape in shapes:
  if step < 5:
   shape.set_fill(False)
  else:
    shape.set_edgecolor('black')
    shape.set_facecolor('black')
    shape.set_alpha(0.25)
  ax.add_artist(shape)

if step != None:
  pyplot.savefig('draw-a-bee-step-' + str(step) + '.png')

