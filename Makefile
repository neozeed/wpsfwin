ARCH ?= x64
ifeq ($(ARCH),x86)
CROSS ?= i686-w64-mingw32
else
CROSS ?= x86_64-w64-mingw32
endif
CXX = $(CROSS)-g++
WINDRES = $(CROSS)-windres
OUT = bin/$(ARCH)
OBJ = build/$(ARCH)
CPPFLAGS = -Iinclude -Iport -include port/compat.hpp -DNDEBUG
CXXFLAGS = -std=gnu++98 -O2 -g -Wall -Wno-write-strings -Wno-unknown-pragmas -Wno-unused-parameter -Werror=return-type -Werror=format -Werror=class-memaccess -Werror=maybe-uninitialized -Werror=int-to-pointer-cast
SOURCES = $(filter-out src/wpclrpal.cpp,$(wildcard src/*.cpp)) $(wildcard port/*.cpp)
OBJECTS = $(patsubst %.cpp,$(OBJ)/%.o,$(SOURCES))
all: $(OUT)/wpshell.exe $(OUT)/WPS_ENUS.DLL
$(OBJ)/%.o: %.cpp $(wildcard include/*) port/compat.hpp
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(OBJ)/shell.res.o: resources/wpshell.rc $(shell find resources -type f) $(wildcard include/*.h)
	@mkdir -p $(OBJ)
	cd resources && $(WINDRES) -I../include wpshell.rc ../$@
$(OBJ)/enus.res.o: resources/enus/wps_enus.rc $(shell find resources/enus -type f) $(wildcard include/*.h)
	@mkdir -p $(OBJ)
	cd resources/enus && $(WINDRES) -I../../include -I.. wps_enus.rc ../../$@
$(OUT)/wpshell.exe: $(OBJECTS) $(OBJ)/shell.res.o
	@mkdir -p $(OUT)
	$(CXX) -mwindows -static -Wl,-Map,$(OUT)/wpshell.map -o $@ $^ -lcomdlg32 -lwinmm -lshell32 -lgdi32 -luser32 -ladvapi32 -lcomctl32
$(OUT)/WPS_ENUS.DLL: $(OBJ)/enus.res.o
	@mkdir -p $(OUT)
	$(CXX) -shared -nostdlib -Wl,--entry,0 -o $@ $<
clean:
	rm -rf build bin
.PHONY: all clean
