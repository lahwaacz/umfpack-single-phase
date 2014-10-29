#include ../Makefile

pkgs = cppunit
pkgs_CXXFLAGS = $(shell pkg-config --cflags $(pkgs))
pkgs_LIBS = $(shell pkg-config --libs $(pkgs))

CPPFLAGS += -I..
CXXFLAGS += $(pkgs_CXXFLAGS)
LDLIBS += $(pkgs_LIBS)

SRC = $(wildcard *.cpp)
PROJECT_OBJ = $(wildcard ../*.o)
PROJECT_OBJ := $(filter-out ../main.o,$(PROJECT_OBJ))

all: run_tests

run_tests: test_runner
	@echo "==> Running tests:"
	./test_runner

test_runner: $(SRC:%.cpp=%.o) $(PROJECT_OBJ)

clean:
	$(RM) *.[od] test_runner

-include $(SRC:%.cpp=%.d)
