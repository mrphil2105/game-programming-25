.PHONY: all build configure clean

all: build

configure:
	cmake -S . -B out -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

build: configure
	cmake --build out

clean:
	rm -rf out
