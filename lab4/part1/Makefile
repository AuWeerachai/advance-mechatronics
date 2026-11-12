# Makefile

# Toolchain definitions
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# Hardware architecture flags specific to the Nucleo-F446ZE
MCU_FLAGS = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# Compilation flags
# Look for headers in the root, application, and both hardware folders.
CFLAGS  = $(MCU_FLAGS) -O0 -g -Wall -ffreestanding -I. -Isrc -Ihardware -Ihardware-edited

# Linker flags
LDFLAGS = -T linker.ld -nostdlib

# Explicitly list all your C source files, including their subfolder paths
C_SOURCES = src/main.c \
			hardware/stm32f446xx_system_mort.c \
			hardware-edited/stm32f446xx_gpio_mort.c \
			hardware/stm32f446xx_rcc_mort.c \
			hardware-edited/stm32f446xx_timer_mort.c \
			hardware/stm32f446xx_usart3_mort.c \
			hardware-edited/stm32f446xx_interruptcontroller_mort.c \
			hardware/stm32f446xx_dma_controller_mort.c \
			hardware/stm32f446xx_adc_mort.c \
			src/nucleo_led.c

# Your assembly startup file
S_SOURCES = hardware/startup_stm32f4xx.s

# Convert source file names into object file names (.o)
# The "patsubst" magic ensures objects mirror the source paths cleanly
OBJECTS = $(addprefix build/,$(C_SOURCES:.c=.o) $(S_SOURCES:.s=.o))
HEADERS = $(wildcard *.h src/*.h hardware/*.h hardware-edited/*.h)

.PHONY: all clean

# Default target rule
all: build/main.bin

# Link all object files together into an ELF file
build/main.elf: $(OBJECTS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

# Strip debugging structures to generate the final flashable raw binary
build/main.bin: build/main.elf
	$(OBJCOPY) -O binary $< $@

# Compile C source files into object files (handles root and subfolders seamlessly)
build/%.o: %.c $(HEADERS) Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble your startup file
build/%.o: %.s Makefile
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Remove generated files, leaving source files untouched.
clean:
	rm -f build/*.o build/*.elf build/*.bin build/src/*.o build/hardware/*.o build/hardware-edited/*.o
