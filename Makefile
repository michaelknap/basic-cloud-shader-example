# Clouds Makefile

# Variables
CC = gcc
CFLAGS = -Wall -fstack-protector-strong
LDFLAGS = -Wl,-z,relro,-z,now
LIBS = -lglfw -lGLEW -lGL
SOURCES = src/main.c
OBJDIR = build
TARGET = $(OBJDIR)/clouds

# Build Rules
all: release

$(OBJDIR):
	mkdir -p $(OBJDIR)

release: $(OBJDIR) $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)
	strip $(TARGET)

debug: $(OBJDIR) $(SOURCES)
	$(CC) $(CFLAGS) -g $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

clean:
	rm -rf $(OBJDIR) $(TARGET)
