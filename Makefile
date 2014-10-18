CXX := $(CXX) -std=c++11
CC := $(CXX)

base_CXXFLAGS = -Wall -Wextra -pedantic -O2 -g
base_LIBS = -lm -lumfpack

#pkgs =
#pkgs_CXXFLAGS = $(shell pkg-config --cflags $(pkgs))
#pkgs_LIBS = $(shell pkg-config --libs $(pkgs))

CPPFLAGS += -MD -MP -D_XOPEN_SOURCE=500
CXXFLAGS := $(base_CXXFLAGS) $(pkgs_CXXFLAGS) $(CXXFLAGS)
#LDFLAGS += -v
LDLIBS := $(base_LIBS) $(pkgs_LIBS)

SRC = main.cpp Matrix.cpp DenseMatrix.cpp SparseMatrix.cpp Vector.cpp SOR.cpp RectangularMesh.cpp Solver.cpp
DIST_TARBALL = bak-$(shell git describe --always | sed 's|-|.|g').tar.gz

all: main

main: $(SRC:%.cpp=%.o)

clean:
	$(RM) *.[od] main

dist: clean
	$(RM) $(DIST_TARBALL) $(DIST_TARBALL).sig
	tar czf $(DIST_TARBALL) $(shell git ls-files)
	gpg --detach-sign $(DIST_TARBALL)

-include $(SRC:%.cpp=%.d)
