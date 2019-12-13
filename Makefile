# Subdirectory make stuff
VPATH = Main 

include Libraries/lib.mk
include Modules/modules.mk 
include Device/device.mk
include Drivers/drivers.mk

PROJECT = DieBieBMS-firmware

# Compiler stuff
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes -Wshadow
# Define extra C flags here
CFLAGS = -mthumb-interwork -mcpu=$(MCU) -D STM32F303xC -D USE_HAL_DRIVER -D ARMGCC $(CWARN)
LDFLAGS = $(CFLAGS) -T $(LDSCRIPT) --specs=nosys.specs
ASFLAGS  = $(CFLAGS) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.s=.lst)) 

# Architecture specific stuff - linker script and architecture
LDSCRIPT= CubeMX/STM32F303CC_FLASH.ld
MCU  = cortex-m4

ASMSRC = $(STARTUPASM)

CSRC := Main/main.c \
		$(HAL_SRC) \
		$(DEVICE_SRC) \
		$(LIB_SRC) \
		$(MOD_SRC) \
		$(HWDRIVER_SRC) \
		$(SWDRIVER_SRC) \

INCDIR = Main \
		$(DEVICE_INC) \
		$(LIB_INC) \
		$(MOD_INC) \
		$(HWDRIVER_INC) \
		$(SWDRIVER_INC) \
		$(HAL_INC)

IINCDIR   = $(patsubst %,-I%,$(INCDIR))

ASMOBJS   = $(addprefix $(OBJDIR)/, $(notdir $(ASMSRC:.s=.o)))
COBJS    = $(addprefix $(OBJDIR)/, $(notdir $(CSRC:.c=.o)))
OBJS	 = $(ASMOBJS) $(COBJS)

BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
LSTDIR = $(BUILDDIR)/lst

OUTFILES = $(BUILDDIR)/$(PROJECT).elf \
           $(BUILDDIR)/$(PROJECT).hex \
           $(BUILDDIR)/$(PROJECT).bin \
           $(BUILDDIR)/$(PROJECT).dmp \
           $(BUILDDIR)/$(PROJECT).list		

## Makefile rules

all: $(BUILDDIR) $(OBJDIR) $(LSTDIR) $(DEPDIR) $(OBJS) $(OUTFILES)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(LSTDIR):
	mkdir -p $(LSTDIR)

$(COBJS) : $(OBJDIR)/%.o : %.c Makefile
	@echo Compiling $(<F)
	$(CC) -c $(CFLAGS) $(OPT) -I. $(IINCDIR) $< -o $@

$(ASMOBJS) : $(OBJDIR)/%.o : %.s Makefile
	@echo Compiling $(<F)
	$(AS) -c $(ASFLAGS) $(IINCDIR) $< -o $@

$(OBJS): | $(BUILDDIR) $(OBJDIR) $(LSTDIR) 

%.elf: $(OBJS) $(LDSCRIPT)
	@echo Linking $@
	$(LD) $(OBJS) $(LDFLAGS) -o $@

%.hex: %.elf $(LDSCRIPT)
	@echo Creating $@
	$(HEX) $< $@

%.bin: %.elf $(LDSCRIPT)
	@echo Creating $@
	$(BIN) $< $@ --gap-fill 0xFF

clean:
	rm -rf $(BUILDDIR) 

#DieBieBMS-firmware.bin: DieBieBMS-firmware.elf
#	$(BIN) DieBieBMS-firmware.elf DieBieBMS-firmware.bin 
#DieBieBMS-firmware.elf: $(CSRC)
#	$(CC) $(CFLAGS) $(ASMSRC) $(CSRC) $(IINCDIR) --specs=nosys.specs -o $@