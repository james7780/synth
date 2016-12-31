# Makefile for Synth and SynthGUI (Rapsberry Pi)

TARGET1 = synth
OBJS1 = Waveform.o Envelope.o Modulator.o Oscillator.o Patch.o \
	Voice.o Filter.o Reverb.o Mixer.o WaveSynth.o SynthEngine.o \
	Messages.o

TARGET2 = synthgui
OBJS2 = SynthGUI.o GUI.o fontengine.o \
	Patch.o Oscillator.o Modulator.o Envelope.o \
    Messages.o TouchScreen.o

CC = g++
RM = rm

INCDIR = 
#CFLAGS := -DRPI -O2 -Wall $(shell sdl-config --cflags)
CFLAGS := -DRPI -Wall $(shell sdl2-config --cflags)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti

# note: -lrt for mqueue stuff
LIBDIR = 
#LDFLAGS := $(shell sdl2-config --libs) -lSDL2main -lm -lstdc++ -lrt -lpthread
LDFLAGS1 = $(LDFLAGS) $(shell sdl2-config --libs) -lm -lstdc++ -lrt -lpthread
LDFLAGS2 = $(LDFLAGS) $(shell sdl2-config --libs) -lm -lstdc++ -lrt

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJS1)
	$(CC) $(LDFLAGS1) -o $(TARGET1) $(OBJS1)

$(TARGET2): $(OBJS2)
	$(CC) $(LDFLAGS2) -o $(TARGET2) $(OBJS2)

clean:
	$(RM) -f $(TARGET1)
	$(RM) -f $(OBJS1)
	$(RM) -f $(TARGET2)
	$(RM) -f $(OBJS2)

