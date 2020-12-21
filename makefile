src:=./src/*
bin:=./bin
outbinFile:=yamaRuntime.exe

all: init build

init:
	git submodule update --init
	mkdir -p $(bin)

build: compile

compile:
	gcc -o $(bin)/$(outbinFile) $(src)

clean:
	rm -rf $(bin)

