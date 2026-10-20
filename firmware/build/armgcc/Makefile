# Makefile for Nordic nRF52840 / nRF52833 Ultra-Low-Power BLE Firmware

PROJECT_NAME     := ulp_ble
TARGETS          := $(PROJECT_NAME)
OUTPUT_DIRECTORY := _build

# Board selection (default ulp_board_52840)
BOARD ?= ulp_board_52840
LOG   ?= 0

ifeq ($(BOARD),ulp_board_52840)
  DEVICE := NRF52840_XXAA
  BOARD_DEF := BOARD_ULP_52840
  LDSCRIPT := ../../config/ulp_ble_nrf52840.ld
else ifeq ($(BOARD),ulp_board_52833)
  DEVICE := NRF52833_XXAA
  BOARD_DEF := BOARD_ULP_52833
  LDSCRIPT := ../../config/ulp_ble_nrf52833.ld
else ifeq ($(BOARD),pca10056)
  DEVICE := NRF52840_XXAA
  BOARD_DEF := BOARD_PCA10056
  LDSCRIPT := ../../config/ulp_ble_nrf52840.ld
else ifeq ($(BOARD),pca10100)
  DEVICE := NRF52833_XXAA
  BOARD_DEF := BOARD_PCA10100
  LDSCRIPT := ../../config/ulp_ble_nrf52833.ld
else
  $(error Unknown BOARD $(BOARD))
endif

# Toolchain configuration
GNU_PREFIX ?= arm-none-eabi-
CC         := $(GNU_PREFIX)gcc
AS         := $(GNU_PREFIX)as
AR         := $(GNU_PREFIX)ar -r
LD         := $(GNU_PREFIX)ld
NM         := $(GNU_PREFIX)nm
OBJDUMP    := $(GNU_PREFIX)objdump
OBJCOPY    := $(GNU_PREFIX)objcopy
SIZE       := $(GNU_PREFIX)size

MK := mkdir -p
RM := rm -rf

# Source files
SRC_FILES += \
  ../../app/src/app_main.c \
  ../../app/src/app_fsm.c \
  ../../app/src/app_cfg.c \
  ../../app/src/power_mgr.c \
  ../../app/src/wake_mgr.c \
  ../../app/src/retention.c \
  ../../app/src/ble_ulp_svc.c \
  ../../app/src/protocol.c \
  ../../app/src/telemetry.c \
  ../../app/src/diag.c \
  ../../drivers/drv_tmp117.c \
  ../../drivers/drv_lis2dw12.c \
  ../../drivers/drv_vbat.c \
  ../../hal/hal_gpio_lp.c \
  ../../hal/hal_twi.c \
  ../../hal/hal_saadc.c \
  ../../hal/hal_wdt.c

# Include directories
INC_FOLDERS += \
  ../../app/inc \
  ../../drivers \
  ../../hal \
  ../../boards \
  ../../config \
  ../../sdk_compat

# Preprocessor defines
CFLAGS += -D$(DEVICE)
CFLAGS += -D$(BOARD_DEF)
CFLAGS += -DLOG=$(LOG)
CFLAGS += -DNRF_SDH_BLE_ENABLED=1
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror -Wno-unused-function
CFLAGS += -O3 -g3
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums

# Linker flags
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mthumb -mabi=aapcs
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).map

INC_PATHS := $(addprefix -I,$(INC_FOLDERS))
OBJECT_DIRECTORY = $(OUTPUT_DIRECTORY)/objs
OBJS = $(patsubst %.c, $(OBJECT_DIRECTORY)/%.o, $(notdir $(SRC_FILES)))
VPATH = $(dir $(SRC_FILES))

.PHONY: all clean flash flash_softdevice erase

all: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).bin
	@echo "Build successful: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex"
	@$(SIZE) $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).elf

$(OUTPUT_DIRECTORY):
	@$(MK) $(OUTPUT_DIRECTORY)
	@$(MK) $(OBJECT_DIRECTORY)

$(OBJECT_DIRECTORY)/%.o: %.c | $(OUTPUT_DIRECTORY)
	@echo "Compiling: $<"
	@$(CC) $(CFLAGS) $(INC_PATHS) -c $< -o $@

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).elf: $(OBJS)
	@echo "Linking target: $@"
	@$(CC) $(LDFLAGS) $(OBJS) -o $@

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).elf
	@echo "Generating HEX file: $@"
	@$(OBJCOPY) -O ihex $< $@

$(OUTPUT_DIRECTORY)/$(PROJECT_NAME).bin: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).elf
	@echo "Generating BIN file: $@"
	@$(OBJCOPY) -O binary $< $@

clean:
	@$(RM) $(OUTPUT_DIRECTORY)

flash: $(OUTPUT_DIRECTORY)/$(PROJECT_NAME).hex
	nrfjprog --family NRF52 --program $< --sectorerase --verify
	nrfjprog --family NRF52 --reset

erase:
	nrfjprog --family NRF52 --eraseall
