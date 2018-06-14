
TARGET = libosd.a

CROSS = arm-none-linux-gnueabi-
CC  = $(CROSS)gcc
AR  = $(CROSS)ar

CFLAGS += -Wall -g -O -I include
VERBOSE = @
COMPILE = $(VERBOSE) $(CC) $(CFLAGS) -c
LINK = $(VERBOSE) $(CC)
LINKLIB = $(VERBOSE) $(AR) -r
SOURCES = $(wildcard *.c)
HEADERS = $(wildcard include/*.h)
OBJFILES = $(SOURCES:%.c=%.o)

.PHONY: clean

all:	$(TARGET)

$(TARGET):	$(OBJFILES)
	@echo
	@echo Linking $@ from $^ 
	$(LINKLIB) $@ $^

$(OBJFILES):	%.o: %.c $(HEADERS)
	@echo Compiling $@ from $<
	$(COMPILE) $< -o $@

clean:
	@echo Removing generated files..
	$(VERBOSE) -$(RM) -rf $(OBJFILES) $(TARGET) *~ *.d .dep

