#
#       !!!! Do NOT edit this makefile with an editor which replace tabs by spaces !!!!    
#
##############################################################################################
# 
# On command line:
#
# make all = Create project
#
# make clean = Clean project files.
#
# To rebuild project do "make clean" and "make all".
#

##############################################################################################
# Start of default section
#

TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

MCU  = cortex-m3

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS = 

# List all default directories to look for include files here
DINCDIR = 

# List the default directory to look for the libraries here
DLIBDIR = 

# List all default libraries here
DLIBS = 

#
# End of default section
##############################################################################################

##############################################################################################
# Start of user section
#

# 
# Define project name and Ram/Flash mode here
PROJECT        = freeRTOS
RUN_FROM_FLASH = 1
BAREBONE	   = 1

#
# Define linker script file here
#
ifeq ($(RUN_FROM_FLASH), 0)
LDSCRIPT = ./prj/lpc1768_ram.ld
FULL_PRJ = $(PROJECT)_ram
else
LDSCRIPT = ./prj/lpc1768_flash.ld
FULL_PRJ = $(PROJECT)_rom
endif

# List all user C define here, like -D_DEBUG=1
UDEFS = 

# Define ASM defines here
UADEFS = 

# List C source files here
# Start with those that are not likely to change. 
# Adding only at the tail of the list the new C codes.
SRC	= ./cmsis/device/system_LPC17xx.c \
	  ./app/crt.c \
	  ./app/retarget.c \
	  ./app/vectors_lpc1768.c \
	  ./lib/gpio/src/gpio_lpc17xx.c \
	  ./lib/uart/src/uart_lpc17xx.c \
	  ./lib/clock/src/clk_lpc17xx.c \
	  ./lib/i2c/src/i2c_lpc17xx.c \
	  ./lib/spi/src/spi_lpc17xx.c \
	  ./lib/adc/src/adc_lpc17xx.c \
	  ./lib/dac/src/dac_lpc17xx.c \
	  ./lib/hal/src/stdirq.c \
	  ./utils/src/utils.c \
	  ./utils/src/fifo.c \
	  ./utils/src/uc_stdio.c

	  
ifeq ($(BAREBONE), 1)
	SRC += ./app/main_barebone.c
else
	SRC +=	./app/main.c \
		  	./common/Minimal/blocktim.c \
	  		./common/Minimal/countsem.c \
	  		./common/Minimal/dynamic.c \
	  		./common/Minimal/recmutex.c \
	  		./app/Partest.c \
			./freeRTOS/list.c \
			./freeRTOS/queue.c \
			./freeRTOS/tasks.c \
			./freeRTOS/timers.c \
			./freeRTOS/portable/ARM_CM3/port.c \
			./freeRTOS/portable/MemMang/heap_2.c \
	  		./app/main-blinky.c \
	  		./app/main-full.c 
endif


#SRC  = ./cmsis/core/core_cm3.c \
#      ./cmsis/device/system_LPC17xx.c \
#      ./src/crt.c \
#       ./src/vectors_lpc1768.c \
#       ./src/main.c

# List ASM source files here
ASRC =

# List all user directories here
UINCDIR  = ./freeRTOS/portable/ARM_CM3 \
		   ./freeRTOS/include \
		   ./common/include \
		   ./cmsis/device \
		   ./cmsis/core \
		   ./lib/hal/inc \
		   ./lib/configs/inc \
		   ./lib/uart/inc \
		   ./lib/i2c/inc \
		   ./lib/clock/inc \
		   ./lib/spi/inc \
		   ./lib/adc/inc \
		   ./lib/dac/inc \
		   ./app \
		   ./lib/gpio/inc \
		   ./utils/inc 

		   

#UINCDIR = ./inc \
#          ./cmsis/core \
#          ./cmsis/device

# List the user directory to look for the libraries here
ULIBDIR = 

# List all user libraries here
ULIBS = 

# Define optimisation level here
OPT = -O0

#
# End of user defines
##############################################################################################


INCDIR  = $(patsubst %,-I%,$(DINCDIR) $(UINCDIR))
LIBDIR  = $(patsubst %,-L%,$(DLIBDIR) $(ULIBDIR))

ifeq ($(RUN_FROM_FLASH), 0)
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=0 -D__RAM_MODE__
else
DEFS    = $(DDEFS) $(UDEFS) -DRUN_FROM_FLASH=1
endif

ADEFS   = $(DADEFS) $(UADEFS)
OBJS    = $(ASRC:.s=.o) $(SRC:.c=.o)
LIBS    = $(DLIBS) $(ULIBS)
MCFLAGS = -mcpu=$(MCU)

ASFLAGS = $(MCFLAGS) -g -gdwarf-2 -Wa,-amhls=$(<:.s=.lst) $(ADEFS)
CPFLAGS = $(MCFLAGS) $(OPT) -gdwarf-2 -mthumb -fomit-frame-pointer -Wall -Wstrict-prototypes -fverbose-asm -Wa,-ahlms=$(<:.c=.lst) $(DEFS)
LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -T$(LDSCRIPT) -Wl,-Map=$(FULL_PRJ).map,--cref,--no-warn-mismatch $(LIBDIR) -lm

# Generate dependency information
CPFLAGS += -MD -MP -MF .dep/$(@F).d

#
# makefile rules
#

ifeq ($(RUN_FROM_FLASH), 0)
all: $(OBJS) $(FULL_PRJ).elf $(FULL_PRJ).hex
else
all: $(OBJS) $(FULL_PRJ).elf $(FULL_PRJ).hex $(FULL_PRJ).bin
endif


%.o : %.c
	$(CC) -c $(CPFLAGS) -I . $(INCDIR) $< -o $@

%.o : %.s
	$(AS) -c $(ASFLAGS) $< -o $@

%elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
  
%hex: %elf
	$(HEX) $< $@

%bin: %elf
	$(BIN) $< $@

clean:
	-rm -f $(OBJS)
	-rm -f $(FULL_PRJ).elf
	-rm -f $(FULL_PRJ).map
	-rm -f $(FULL_PRJ).hex
	-rm -f $(FULL_PRJ).bin
	-rm -f $(SRC:.c=.c.bak)
	-rm -f $(SRC:.c=.lst)
	-rm -f $(ASRC:.s=.s.bak)
	-rm -f $(ASRC:.s=.lst)
	-rm -fR .dep

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***