#!/usr/bin/env python

import matplotlib.patches as patches
import matplotlib.pyplot as pyplot

ax = pyplot.subplot(aspect="equal")
ax.set_xlim(-16, +16)
ax.set_ylim(-16, +16)

ax.add_artist(patches.Circle(xy=[0, 0], radius=3))
ax.text(x=0, y=0, s='thorax', horizontalalignment='center', verticalalignment='center')

ax.add_artist(patches.Ellipse(xy=[0,-9], width=6, height=12))
ax.text(x=0, y=-9, s='abdomen', horizontalalignment='center', verticalalignment='center')

ax.add_artist(patches.Ellipse(xy=[0, 4], width=6, height=2))
ax.text(x=0, y=4, s='head', horizontalalignment='center', verticalalignment='center')

pyplot.savefig('draw-a-bee.png')
pyplot.show()

