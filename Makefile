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

all: release-build dev-build

docker-ubuntu:
	docker build -t godot-jack-ubuntu ./platform/ubuntu

shell-ubuntu: docker-ubuntu
	docker run -it --rm -v ${CURDIR}:${CURDIR} --user ${UID}:${GID} -w ${CURDIR} godot-jack-ubuntu ${SHELL_COMMAND}

ubuntu:
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_release.sh'
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_debug.sh'

ubuntu-debug:
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_debug.sh'

ubuntu-release:
	$(MAKE) shell-ubuntu SHELL_COMMAND='./platform/ubuntu/build_release.sh'

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
