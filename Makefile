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

# Architecture specific stuff - linker script and architecture
LDSCRIPT = STM32F303CCTx_FLASH.ld
MCU  = cortex-m4

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes -Wshadow
# Define extra C flags here
CFLAGS += --specs=nosys.specs --specs=nano.specs
CFLAGS =  -mthumb -mthumb-interwork -mcpu=$(MCU) -Os -mlittle-endian 
CFLAGS += -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -march=armv7e-m
CFLAGS += -D ARM_MATH_CM4 -D STM32F303xC -D USE_HAL_DRIVER -D ARMGCC
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -lc -lm -lgcc -std=c99 
CFLAGS += $(CWARN) -g -Wa,-alms=$(LSTDIR)/$(notdir $(<:.c=.lst)) 
LDFLAGS = $(CFLAGS) 
LDFLAGS += -Wl,--script=$(LDSCRIPT),-lc,-lm,-lgcc,-lnosys,-Map=${BUILDDIR}/${PROJECT}.map,--cref,-u,ResetHandler
ASFLAGS  = $(CFLAGS) -Wa,-alms=$(LSTDIR)/$(notdir $(<:.s=.lst)) 

ASMSRC = $(STARTUPASM)

CSRC := Main/main.c \
		$(HAL_SRC) \
		$(DEVICE_SRC) \
		$(LIB_SRC) \
		$(MOD_SRC) \
		$(HWDRIVER_SRC) \
		$(SWDRIVER_SRC) \

INCDIR = Main \
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
		   $(BUILDDIR)/$(PROJECT).s \

## Makefile rules

all: $(BUILDDIR) $(OBJDIR) $(LSTDIR) $(OBJS) $(OUTFILES)

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
	$(BIN) $< $@

$(BUILDDIR)/$(PROJECT).s : $(BUILDDIR)/$(PROJECT).elf
	@echo Creating $@
	$(OD) -S -d $< > $@

clean:
	rm -rf $(BUILDDIR) 
	
rebuild: clean all

upload: $(BUILDDIR)/$(PROJECT).bin
	@echo Uploading $(BUILDDIR)/$(PROJECT).bin ...
	openocd -f board/st_nucleo_f3.cfg -c "program $(BUILDDIR)/$(PROJECT).bin verify reset exit 0x08000000"

debug: 
	openocd -f board/st_nucleo_f3.cfg