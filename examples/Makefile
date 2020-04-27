SOURCES = wav2mp3.ml
RESULT = wav2mp3
LIBS = unix lame
INCDIRS = ../src

all: nc

test: all
	if [ ! -e test.wav ] ; then ffmpeg -f lavfi -i "sine=frequency=440:duration=5" -ac 2 test.wav ; fi
	./wav2mp3 test.wav test.mp3
	./wav2mp3 --bigarray test.wav test.mp3

-include OCamlMakefile
