src:=./src/*
bin:=./bin
outbinFile:=yamaRuntime.exe

all: init build

init:
	git submodule update --init
	mkdir -p $(bin)

build: compile

compile:
	gcc -I/opt/usr/include -o $(bin)/$(outbinFile) $(src)

compileWin:
	gcc -D WINDOWS -o $(bin)/$(outbinFile) $(src)

clean:
	rm -rf $(bin)

