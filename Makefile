RACK_DIR ?= ../..
FLAGS +=
SOURCES += $(wildcard src/*.cpp) $(wildcard src/dependancies/utility/*.cpp) $(wildcard src/dependancies/filt/biquad.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res
# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
