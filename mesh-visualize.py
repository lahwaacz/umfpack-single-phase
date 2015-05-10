#! /usr/bin/env python3

import os.path
import glob

import matplotlib.pyplot as plt
from matplotlib.widgets import Button
from mpl_toolkits.axes_grid1 import make_axes_locatable
import numpy as np
from scipy import interpolate

width = 10
height = 10
cols = 100
rows = 100
filenames = sorted( glob.glob( "out/pressure-%dx%d-*.dat" % (rows, cols) ) )

def load_values( fname ):
    # load vector, reshape to matrix form
    values = np.loadtxt( fname, usecols=(1,) )
    values = np.reshape( values, (rows, cols) )
    return values

def plot_mesh( fig, ax, fname ):
    values = load_values( fname )
    aspect = (height/rows)/(width/cols)
    fig.suptitle( fname, fontsize=14 )

    # plot mesh values
    img = ax.imshow( values,
                      interpolation="nearest",
#                      cmap=plt.get_cmap( "rainbow" ),
                      origin="lower",
                      aspect=aspect )

    # NOTE: fixed colorbar limits so that all figures use the same scale
    img.set_clim( vmin=1e5, vmax=1e5+1e3 )

    # plot colorbar
    divider = make_axes_locatable( ax )
    cax = divider.append_axes( "right", size="5%", pad=0.1, aspect=20*aspect )
    colorbar = fig.colorbar( img, cax=cax, format="%.0f" )

    # return reference to objects needed to update the image
    return img, colorbar

# update the plot to show data from num-th file
def update_plot( num ):
    fig.suptitle( filenames[ num ], fontsize=14 )
    values = load_values( filenames[ num ] )

    # update mesh values
    img.set_data( values )

    # set limits to update colorbar
#    img.set_clim( [values.min(), values.max()] )

    plt.draw()

class Index:
    ind = 0

    def __init__( self, length, updater ):
        self.length = length
        self.updater = updater

    def next(self, event):
        self.ind += 1
        i = self.ind % self.length
        self.updater( i )

    def prev(self, event):
        self.ind -= 1
        i = self.ind % self.length
        self.updater( i )


# save plots as png files
for filename in filenames:
    out = os.path.splitext(filename)[0] + ".png"
    print( "Plotting %s..." % out )
    fig, ax = plt.subplots( 1, 1 )
    plot_mesh( fig, ax, filename )
    fig.savefig( out )
    plt.close()

# "interactive" output
#fig, ax = plt.subplots( 1, 1 )
#img, colorbar = plot_mesh( fig, ax, filenames[ 0 ] )

#callback = Index( len( filenames ), update_plot )
#axprev = plt.axes( [0.40, 0.05, 0.1, 0.075] )
#axnext = plt.axes( [0.55, 0.05, 0.1, 0.075] )
#bnext = Button( axnext, "Next" )
#bnext.on_clicked( callback.next )
#bprev = Button( axprev, "Previous" )
#bprev.on_clicked( callback.prev )

#plt.show()
