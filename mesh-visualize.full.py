#! /usr/bin/env python3

import matplotlib.pyplot as plt
from matplotlib.widgets import Button
from mpl_toolkits.axes_grid1 import make_axes_locatable
import numpy as np
from scipy import interpolate
import glob

width = 100
height = 100
cols = 50
rows = 50
#filenames = sorted( glob.glob( "out/pressure-vect-gravity-%dx%d-*.dat" % (rows, cols) ) )
filenames = sorted( glob.glob( "out/pressure-vect-gravity-test-%dx%d-*.dat" % (rows, cols) ) )

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
    # TODO: velocity depends on gravity
#    grad_x, grad_y = get_gradients( values )
#    qv = ax.quiver( -grad_x, -grad_y, units="xy", scale=1.2, color="black", pivot="middle" )
    x, y, grad_x, grad_y = get_gradients_interpolated( values )
#    qv = ax.quiver( x, y, -grad_x, -grad_y, units="xy", scale=0.2, color="black", pivot="middle" )
    qv = None

#    # plot gradient streamlines
#    dx, dy = get_gradients( values )
#
#    # mesh grid
#    x = np.linspace( 1, cols-2, cols )
#    y = np.linspace( 1, rows-2, rows )
#    xx, yy = np.meshgrid( x, y )
#
#    sp = ax.streamplot( xx, yy, -dx, -dy, color="black", density=[2, 1], arrowsize=1 )
#    plt.xlim((0, cols-1))
#    plt.ylim((rows-1, 0))

    # plot colorbar
    divider = make_axes_locatable( ax )
    cax = divider.append_axes( "right", size="5%", pad=0.1, aspect=20*aspect )
    colorbar = fig.colorbar( img, cax=cax, format="%.0f" )

    # return reference to objects needed to update the image
    return img, colorbar, qv
#    return img, colorbar, sp

# return (grad_x, grad_y) tuple, where grad_x and grad_y are the x and y components
# of gradient vectors of mesh values
def get_gradients( mesh_values ):
    # FIXME: how to properly "invert" y-axis for plt.quiver?
#    mesh_values = mesh_values[::-1]

    # NOTE: np.gradient() returns x,y components transposed
    grad_y, grad_x = np.gradient( mesh_values, height, width )
    grad_norm = np.sqrt( grad_x ** 2 + grad_y ** 2 )

    return grad_x / grad_norm, grad_y / grad_norm
#    return grad_x, grad_y

def get_gradients_interpolated( mesh_values ):
    # FIXME: how to properly "invert" y-axis for plt.quiver?
    mesh_values = mesh_values[::-1]

    # full-size mesh grid for interpolation
    x = np.linspace( 0, cols-1, cols )
    y = np.linspace( 0, rows-1, rows )
    xx, yy = np.meshgrid( x, y )

    # interpolate pressure values
    # NOTE: bisplrep() has x,y components transposed
    interpolated = interpolate.bisplrep( yy, xx, mesh_values )
#    print( interpolated )

    # smaller mesh for gradient vectors
    x = np.linspace( 1, cols-2, 20 )
    y = np.linspace( 1, rows-2, 20 )

    # evaluate interpolated function's derivatives on the small mesh
    grad_x = interpolate.bisplev( x, y, interpolated, dy=1 )
    grad_y = interpolate.bisplev( x, y, interpolated, dx=1 )
    grad_norm = np.sqrt( grad_x ** 2 + grad_y ** 2 )

    return x, y, grad_x / grad_norm, grad_y / grad_norm

# update the plot to show data from num-th file
def update_plot( num ):
    fig.suptitle( filenames[ num ], fontsize=14 )
    values = load_values( filenames[ num ] )

    # update mesh values
    img.set_data( values )

    # set limits to update colorbar
    img.set_clim( [values.min(), values.max()] )

    # update gradient vectors
#    grad_x, grad_y = get_gradients( values )
    _, _, grad_x, grad_y = get_gradients_interpolated( values )
#    qv.set_UVC( -grad_x, -grad_y )

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
