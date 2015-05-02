CXX := $(CXX) -std=c++11
CC := $(CXX)

CPPFLAGS += -MD -MP -D_XOPEN_SOURCE=500
CXXFLAGS += -Wall -Wextra -Woverloaded-virtual -pedantic -O3 -g -rdynamic
LDFLAGS = -lm -lumfpack

#pkgs =
#CXXFLAGS += $(shell pkg-config --cflags $(pkgs))
#LDFLAGS += $(shell pkg-config --libs $(pkgs))

TNL_VERSION=0.1
TNL_INSTALL_DIR=${HOME}/local/lib
TNL_INCLUDE_DIR=${HOME}/local/include/tnl-${TNL_VERSION}

CXXFLAGS += -I$(TNL_INCLUDE_DIR)
LDFLAGS += -L$(TNL_INSTALL_DIR) -ltnl-0.1

SRC = main.cpp Array.cpp Vector.cpp Matrix.cpp DenseMatrix.cpp SparseMatrix.cpp SOR.cpp RectangularMesh.cpp Solver.cpp
DIST_TARBALL = bak-$(shell git describe --always | sed 's|-|.|g').tar.gz

export

all: main

main: $(SRC:%.cpp=%.o)
# override implicit linking rule and put LDFLAGS at the end
# (workaround for gcc 4.8.2 in ubuntu 14.04 on romanne)
	$(CXX) -o $@ $^ $(LDFLAGS)

tests: main force_look
	$(MAKE) $(MFLAGS) --directory=tests

clean:
	$(RM) *.[od] main
	$(MAKE) $(MFLAGS) --directory=tests clean

dist:
	$(RM) $(DIST_TARBALL) $(DIST_TARBALL).sig
	tar czf $(DIST_TARBALL) $(shell git ls-files)
	gpg --detach-sign $(DIST_TARBALL)

force_look:

-include $(SRC:%.cpp=%.d)
