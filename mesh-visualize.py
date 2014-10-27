#! /usr/bin/env python3

import matplotlib.pyplot as plt
from matplotlib.widgets import Button
from mpl_toolkits.axes_grid1 import make_axes_locatable
import numpy as np
from scipy import interpolate
import glob

width = 2.0
height = 1.0
cols = 100
rows = 100
filenames = sorted( glob.glob( "pressure-vect-%dx%d-*.dat" % (rows, cols) ) )

def load_values( fname ):
    # load vector, reshape to matrix form
    values = np.loadtxt( fname, usecols=(1,) )
    values = np.reshape( values, (rows, cols) )
    return values

def plot_mesh( fig, ax, fname, suffix="" ):
    values = load_values( fname )
    aspect = (height/rows)/(width/cols)
    fig.suptitle( fname, fontsize=14 )

    # plot mesh values
    img = ax.imshow( values,
                      interpolation="nearest",
                      cmap=plt.get_cmap( "rainbow" ),
                      origin="upper",
                      aspect=aspect )

    # plot gradient vectors
    grad_x, grad_y = get_gradients( values )
    qv = ax.quiver( -grad_x, -grad_y, units="xy", scale=1.5, color="black", pivot="middle" )

    # plot colorbar
    divider = make_axes_locatable( ax )
    cax = divider.append_axes( "right", size="5%", pad=0.1, aspect=20*aspect )
    colorbar = fig.colorbar( img, cax=cax )

    # return reference to objects needed to update the image
    return img, colorbar, qv

# return (grad_x, grad_y) tuple, where grad_x and grad_y are the x and y components
# of gradient vectors of mesh values
def get_gradients( mesh_values ):
    # FIXME: how to properly "invert" y-axis for plt.quiver?
    mesh_values = mesh_values[::-1]

    # NOTE: np.gradient() returns x,y components in opposite order
    grad_y, grad_x = np.gradient( mesh_values, height, width )
    grad_norm = np.sqrt( grad_x ** 2 + grad_y ** 2 )

    return grad_x / grad_norm, grad_y / grad_norm

## interpolate pressure values
#
#    # mesh
#    x = np.linspace( 0, cols-1, cols )
#    y = np.linspace( 0, rows-1, rows )
#    xx, yy = np.meshgrid( x, y )
#
#interpolated = interpolate.bisplrep( xx, yy, values )
#print( interpolated )
#gradient_x = interpolate.bisplev( x, y, interpolated, dx=1 )
#gradient_y = interpolate.bisplev( x, y, interpolated, dy=1 )
#print( gradient_x )
#print( gradient_y )
#gradient_norm = np.sqrt( gradient_x ** 2 + gradient_y ** 2 )
#print( gradient_norm )
#
#plt.quiver( xx, yy, gradient_x / gradient_norm, gradient_y / gradient_norm,
#            units="xy", scale=3.0, color="black" )


# update the plot to show data from num-th file
def update_plot( num ):
    fig.suptitle( filenames[ num ], fontsize=14 )
    values = load_values( filenames[ num ] )

    # update mesh values
    img.set_data( values )

    # set limits to update colorbar
    img.set_clim( [values.min(), values.max()] )

    # update gradient vectors
    grad_x, grad_y = get_gradients( values )
    qv.set_UVC( -grad_x, -grad_y )

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


fig, ax = plt.subplots( 1, 1 )
img, colorbar, qv = plot_mesh( fig, ax, filenames[ 0 ] )
#fig.savefig( "pressure-mesh-%03dx%03d-%s.png" % (rows, cols, suffix) )

callback = Index( len( filenames ), update_plot )
axprev = plt.axes( [0.40, 0.05, 0.1, 0.075] )
axnext = plt.axes( [0.55, 0.05, 0.1, 0.075] )
bnext = Button( axnext, "Next" )
bnext.on_clicked( callback.next )
bprev = Button( axprev, "Previous" )
bprev.on_clicked( callback.prev )

plt.show()
