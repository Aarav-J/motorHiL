# Bare-metal STM32F446RE (Nucleo-F446RE) build.
#
# Targets:
#   make            build/firmware.elf, .bin, .hex, .lst + size report
#   make flash      flash build/firmware.elf via OpenOCD + ST-LINK
#   make erase      mass-erase the chip
#   make debug      start OpenOCD (background) + arm-none-eabi-gdb attached
#   make clean

TARGET   = firmware
BUILDDIR = build

PREFIX   = arm-none-eabi-
CC       = $(PREFIX)gcc
OBJCOPY  = $(PREFIX)objcopy
OBJDUMP  = $(PREFIX)objdump
SIZE     = $(PREFIX)size
GDB      = $(PREFIX)gdb

OPENOCD_CFG = openocd.cfg

SRCS = \
  startup/startup_stm32f446xx.s \
  src/system_stm32f446xx.c \
  src/main.c \
  drivers/gpio.c \
  drivers/uart.c \
  drivers/pwm.c

INCLUDES = -Iinc -Idrivers

MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

CFLAGS  = $(MCU_FLAGS) $(INCLUDES) -std=c11 -Wall -Wextra -Os -g3
CFLAGS += -ffunction-sections -fdata-sections -fno-common

ASFLAGS = $(MCU_FLAGS) -x assembler-with-cpp

LDSCRIPT = linker/STM32F446RE_FLASH.ld
LDFLAGS  = $(MCU_FLAGS) -T$(LDSCRIPT) \
           -Wl,--gc-sections -Wl,-Map=$(BUILDDIR)/$(TARGET).map \
           --specs=nosys.specs

OBJS = $(addprefix $(BUILDDIR)/,$(notdir $(SRCS:.c=.o)))
OBJS := $(OBJS:.s=.o)
vpath %.c src drivers
vpath %.s startup

ELF = $(BUILDDIR)/$(TARGET).elf
BIN = $(BUILDDIR)/$(TARGET).bin
HEX = $(BUILDDIR)/$(TARGET).hex
LST = $(BUILDDIR)/$(TARGET).lst

.PHONY: all flash erase debug clean size

all: $(BIN) $(HEX) $(LST) size

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.s | $(BUILDDIR)
	$(CC) $(ASFLAGS) -c $< -o $@

$(ELF): $(OBJS) $(LDSCRIPT) | $(BUILDDIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

$(LST): $(ELF)
	$(OBJDUMP) -h -S $< > $@

size: $(ELF)
	$(SIZE) $(ELF)

flash: $(ELF)
	openocd -f $(OPENOCD_CFG) -c "program $(ELF) verify reset exit"

erase:
	openocd -f $(OPENOCD_CFG) -c "init" -c "reset halt" -c "stm32f4x mass_erase 0" -c "reset" -c "exit"

debug: $(ELF)
	-@pkill -f "openocd -f $(OPENOCD_CFG)" 2>/dev/null; sleep 0.3
	openocd -f $(OPENOCD_CFG) &
	$(GDB) -ex "target extended-remote :3333" -ex "monitor reset halt" $(ELF)
	-@pkill -f "openocd -f $(OPENOCD_CFG)" 2>/dev/null

clean:
	rm -rf $(BUILDDIR)
