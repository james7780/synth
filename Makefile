# Makefile for Synth (Rapsberry Pi)

TARGET = synth
OBJS = Waveform.o Envelope.o Modulator.o Oscillator.o Patch.o \
	Voice.o Filter.o Reverb.o Mixer.o WaveSynth.o SynthEngine.o \
	Messages.o

CC = g++
RM = rm

INCDIR = 
#CFLAGS := -DRPI -O2 -Wall $(shell sdl-config --cflags)
CFLAGS := -DRPI -Wall $(shell sdl-config --cflags)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
#ASFLAGS = $(CFLAGS)

# note: -lrt for mqueue stuff
LIBDIR = 
LDFLAGS := $(shell sdl-config --libs) -lSDLmain -lSDL -lm -lstdc++ -lrt -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

clean:
	$(RM) -f $(TARGET)
	$(RM) -f $(OBJS)

