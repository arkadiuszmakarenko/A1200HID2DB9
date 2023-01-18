TITLE_NAME	:= A1200HIDToDB9Keyboard

DEBUG:= 1

ifeq ($(DEBUG), 1)
TITLE_NAME	:= $(addsuffix Debug, $(TITLE_NAME))
endif

ifeq ($(DEBUG), 0)
TITLE_NAME	:= $(addsuffix Release, $(TITLE_NAME))
endif

CC = arm-none-eabi-gcc 
OBJCOPY  = arm-none-eabi-objcopy

GCC_FLAGS 	+= -std=gnu11
GCC_FLAGS 	+= -DUSE_HAL_DRIVER -DSTM32F401xC
GCC_FLAGS 	+= -Wall --specs=nano.specs
GCC_FLAGS 	+= -mlittle-endian -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
GCC_FLAGS 	+= -mcpu=cortex-m4 -DAT32F4=4 -DMCU=4
GCC_FLAGS 	+= -mslow-flash-data -fstack-usage -MMD -MP -fdata-sections -ffunction-sections 

ifeq ($(DEBUG), 1)
GCC_FLAGS   += -O0 -g -ggdb
endif

ifeq ($(DEBUG), 0)
GCC_FLAGS   += -Os
endif

SRCDIR      := src

ifeq ($(DEBUG), 1)
BUILDDIR    := obj/Debug
endif

ifeq ($(DEBUG), 0)
BUILDDIR    := obj/Release
endif

ifeq ($(DEBUG), 1)
TARGETDIR   := bin/Debug
endif

ifeq ($(DEBUG), 0)
TARGETDIR   := bin/Release
endif


RESDIR      := res

SOURCES     := $(shell find $(SRCDIR) -type f -name *.c)
ASSEMBLYS	:= $(shell find $(SRCDIR) -type f -name *.s)
SOURCES 	+= $(ASSEMBLYS:.s=.o)
HEADERH		:= $(shell find $(SRCDIR) -type f -name *.h)
INCLUDEH  	:= $(addprefix  -I$(CURDIR)/,$(sort $(dir $(HEADERH))))


OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
## SELECT ONLY FIRST FOUND LD SCRIPT
LDSCRIPT	:= $(word 1, $(addprefix  $(CURDIR)/,$(shell find $(SRCDIR) -type f -name *.ld))) 

LD_FLAGS	:= -mcpu=cortex-m4 -T$(LDSCRIPT) --specs=nosys.specs  \
				 -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 \
				 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group \
				 -O0 -g -ggdb 
#-Wl, -Map=$(TITLE_NAME).map

.PHONY: all clean read erase write

.SECONDARY: $(OBJECTS) $(TITLE_NAME).elf



debug: 
	@echo "Building Debug"
	@make build -j -s


release: 
	@echo "Building Release"
	@make build -j -s DEBUG=0

all:
	@make build -j -s
	@make build -j -s DEBUG=0




build: $(TARGETDIR)/$(TITLE_NAME).bin $(OBJECTS) $(HEADERH) $(TARGETDIR)/$(TITLE_NAME).elf
	@echo "Build completed."


$(TARGETDIR)/%.elf: $(OBJECTS)
	@echo "LD" $@ $(notdir $(LDSCRIPT) )
	@mkdir -p $(@D)
	@$(CC) -o $(TARGETDIR)/$(TITLE_NAME).elf $(OBJECTS) $(LD_FLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(HEADERH)
	@echo "CC" $(notdir $@)
	@mkdir -p $(@D)
	@$(CC) $(GCC_FLAGS) $(INCLUDEH) -c -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.s
	@echo "ASM" $(notdir $@)
	@mkdir -p $(@D)
	@$(CC) $(GCC_FLAGS) -c -x assembler-with-cpp  -o $@ $<

$(TARGETDIR)/%.bin: $(TARGETDIR)/%.elf
	@echo "OBJCOPY"  $@
	@mkdir -p $(@D)
	@$(OBJCOPY) -O binary $< $@
	@chmod a-x $@


read:
	st-flash --connect-under-reset read firmware.bin 0x8000000 256k

erase: 
	st-flash erase

write: all
	st-flash  write  $(TARGETDIR)/$(TITLE_NAME).bin 0x8000000

clean:
	@echo "Clean"
	 $(RM) -rf obj
	 $(RM) -rf bin
	 $(RM) -rf *.bin *.elf *.hex *.map


