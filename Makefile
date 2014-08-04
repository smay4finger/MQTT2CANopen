TARGET=mqtt2canopen
OBJECTS=mqtt2canopen.o dictionary.o

CANFESTIVAL=../CanFestival-3
CANFESTIVAL_TARGET=unix
CANFESTIVAL_CAN_DRIVER=socket
CANFESTIVAL_TIMERS_DRIVER=unix
CANFESTIVAL_DYNAMIC_LOAD=false

##################################################

CANFESTIVAL_CONFIGURE = --target=$(CANFESTIVAL_TARGET) --can=$(CANFESTIVAL_CAN_DRIVER) --timers=$(CANFESTIVAL_TIMERS_DRIVER)

ifdef DEBUG
CANFESTIVAL_CONFIGURE += --debug=PDO,WAR,MSG
endif

CANFESTIVAL_LIB = $(CANFESTIVAL)/src/libcanfestival.a
OBJECTS += $(CANFESTIVAL_LIB)

CANFESTIVAL_TARGET_LIB = $(CANFESTIVAL)/drivers/$(CANFESTIVAL_TARGET)/libcanfestival_$(CANFESTIVAL_TARGET).a
OBJECTS += $(CANFESTIVAL_TARGET_LIB)

LDLIBS += -lpthread -lrt -lmosquitto

ifeq ($(CANFESTIVAL_DYNAMIC_LOAD),true)
  LDLIBS += -ldl
  CFLAGS += -DCANFESTIVAL_DYNAMIC_LOAD
else
  CANFESTIVAL_CONFIGURE += --disable-dll
  CANFESTIVAL_DRIVER_LIB = $(CANFESTIVAL)/drivers/can_$(CANFESTIVAL_CAN_DRIVER)/can_$(CANFESTIVAL_CAN_DRIVER).o
  OBJECTS += $(CANFESTIVAL_DRIVER_LIB)
endif

INCLUDES= \
	-I$(CANFESTIVAL)/include \
	-I$(CANFESTIVAL)/include/$(CANFESTIVAL_TARGET) \
	-I$(CANFESTIVAL)/include/can_$(CANFESTIVAL_CAN_DRIVER) \
	-I$(CANFESTIVAL)/include/timers_$(CANFESTIVAL_TIMERS_DRIVER)

CFLAGS += -g -W -Wall -Wstrict-prototypes -O2 $(INCLUDES)

all: $(TARGET)
$(TARGET): $(OBJECTS)
clean:
	$(RM) $(TARGET) $(OBJECTS)
	$(RM) dictionary.c dictionary.h
	$(MAKE) -C $(CANFESTIVAL) clean
.PHONY: all clean objedit


dictionary.c dictionary.h: $(TARGET:=.od)
	python $(CANFESTIVAL)/objdictgen/objdictgen.py $< $@

$(CANFESTIVAL_LIB) $(CANFESTIVAL_TARGET_LIB): canfestival

canfestival: $(CANFESTIVAL)
	( cd $(CANFESTIVAL) ; ./configure $(CANFESTIVAL_CONFIGURE) )
	$(MAKE) -C $(CANFESTIVAL) canfestival

objedit: $(TARGET:=.od)
	$(CANFESTIVAL)/objdictgen/objdictedit.py $<

mqtt2canopen.c: dictionary.h
