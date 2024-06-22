UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
PLATFORM=linux
else ifeq ($(UNAME), Darwin)
PLATFORM=osx
else
PLATFORM=windows
endif

release-build:
	scons platform=$(PLATFORM) target=template_release

dev-build:
	scons platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes

godot-cpp:
	(cd godot-cpp && scons platform=$(PLATFORM) bits=64 generate_bindings=yes)

format:
	clang-format -i src/*.cpp src/*.h
	gdformat $(shell find -name '*.gd' ! -path './godot-cpp/*')

clean:
	rm -f src/*.os

compiledb: clean
	scons -c platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes | tee build-log.txt
	scons platform=$(PLATFORM) target=template_debug dev_build=yes debug_symbols=yes | tee build-log.txt
	compiledb --parse build-log.txt

UNAME := $(shell uname)
ifeq ($(UNAME), Windows)
    UID=1000
    GID=1000
else
    UID=`id -u`
    GID=`id -g`
endif

cgdb:
	cgdb --args $GODOT4 --editor
