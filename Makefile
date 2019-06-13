TARGET_EXEC ?= main.elf

DEVICE     ?= atmega2560
CLOCK      = 16000000L
PROGRAMMER ?= -c avrisp2 -P usb
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -B 10 -F

FUSES      = -U hfuse:w:0xd2:m -U lfuse:w:0xff:m
BUILD_DIR ?= ./build
SRC_DIRS ?= ./grbl

CC=avr-g++
CXX=avr-g++

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS ?= $(INC_FLAGS) -Wall -std=gnu++11 -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -ffunction-sections -flto -MMD -MP
CPPFLAGS ?= $(INC_FLAGS) -Wall -std=gnu++11 -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) -ffunction-sections -flto -MMD -MP

grbl.hex: $(BUILD_DIR)/$(TARGET_EXEC)
	rm -f grbl.hex
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIR)/main.elf grbl.hex
	avr-size --format=berkeley $(BUILD_DIR)/main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@  $(CPPFLAGS) -lm -Wl,--gc-sections

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# symbolic targets:
all:	grbl.hex

.PHONY: clean

flash:	all
	$(AVRDUDE) -U flash:w:grbl.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID grbl.hex

clean:
	rm -rf grbl.hex 
	rm -r $(BUILD_DIR)


-include $(DEPS)

MKDIR_P ?= mkdir -p