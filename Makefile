TARGET=mqtt2canopen
OBJECTS=mqtt2canopen.o
DRV=socketcan

ifeq ($(DRV), socketcan)
LIBEXT=s
else 
LIBEXT=c
endif

INCLUDES=-Icolib/include -I.
LDLIBS=-Lcolib/lib -lCANopenlib$(LIBEXT)_$(shell $(CC) -dumpmachine) -lrt -lmosquitto 


CFLAGS += -W -Wall -Wstrict-prototypes -O2 $(INCLUDES) -g

all: $(TARGET)
$(TARGET): $(OBJECTS)
clean:
	$(RM) $(TARGET) $(OBJECTS)

.PHONY: all clean
