
INSTALL_DIR=/usr/bin/
SOURCES=msteg.c
HEADERS=argp.h
TARGET=msteg
CC=cc

all: msteg

msteg: $(SOURCES) $(HEADERS)
	$(CC) $(SOURCES) -o $(TARGET)

clean:
	rm -rf $(TARGET)

install:
	cp $(TARGET) $(INSTALL_DIR)

uninstall:
	rm -f $(INSTALL_DIR)$(TARGET)

