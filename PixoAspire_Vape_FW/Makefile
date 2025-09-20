TARGET = Project

OPT = -Og

BUILD_DIR = build

C_SOURCES =  \
Src/main.c \
Src/usbd_cdc_if.c \
Src/py32f4xx_it.c \
Src/py32f4xx_hal_msp.c \
Src/system_py32f4xx.c \
Drivers/CherryUSB/core/usbd_core.c \
Drivers/CherryUSB/port/usb_dc_py32.c \
Drivers/CherryUSB/class/cdc/usbd_cdc.c \
Drivers/BSP/PY32F403xx_Start_Kit/py32f403xx_Start_Kit.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_cortex.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_dma.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_gpio.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_rcc.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_rcc_ex.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_uart.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_pwr.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_pwr_ex.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_iwdg.c\
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_wwdg.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_spi.c \
Drivers/PY32F4xx_HAL_Driver/Src/py32f4xx_hal_spi_ex.c

ASM_SOURCES =  \
startup_py32f403xx.s

PREFIX = arm-none-eabi-
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
CPU = -mcpu=cortex-m4

FPU = -mfpu=fpv4-sp-d16

FLOAT-ABI = -mfloat-abi=hard

MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

AS_DEFS = 

C_DEFS =  \
-DUSE_HAL_DRIVER \
-DPY32F403xC

AS_INCLUDES = 

C_INCLUDES =  \
-ISrc \
-IDrivers/BSP/PY32F403xx_Start_Kit \
-IDrivers/PY32F4xx_HAL_Driver/Inc \
-IDrivers/CMSIS/Device/PUYA/PY32F4xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/CherryUSB/port \
-IDrivers/CherryUSB/core \
-IDrivers/CherryUSB/common \
-IDrivers/CherryUSB/class/cdc \

ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

LDSCRIPT = py32f403xd.ld

LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

clean:
	-rm -fR $(BUILD_DIR)
  
flash:
	C:\Program Files\SEGGER\JLink_V870\JLink -device PY32F403xC -if swd -nogui 1 -speed 4000 -JLinkScriptFile jlink/jlink-script -CommanderScript jlink/jlink-command

erase:
	C:\Program Files\SEGGER\JLink_V870\JLink -device PY32F403xC -if swd -nogui 1 -speed 4000 -JLinkScriptFile jlink/jlink-script -CommanderScript jlink/jlink-command_erase

-include $(wildcard $(BUILD_DIR)/*.d)
