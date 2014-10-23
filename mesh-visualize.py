#! /usr/bin/env python3

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

filename = "pressure-vect.dat"
#rows = 27
#cols = 27
rows = 100
cols = 100

# load vector, reshape to matrix form
zvals = np.loadtxt( filename, usecols=(1,) )
zvals = np.reshape( zvals, (rows, cols) )

# colormap
cmap = plt.get_cmap( "rainbow" )

img = plt.imshow( zvals,
                  interpolation="nearest",
                  cmap=cmap,
                  origin="lower" )

plt.colorbar( img, cmap=cmap )

# y-axis should point down
plt.gca().invert_yaxis()

#plt.savefig( "pressure-mesh-%dx%d.png" % (rows, cols) )
plt.show()
