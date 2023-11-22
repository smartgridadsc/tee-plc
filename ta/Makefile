CFG_TEE_TA_LOG_LEVEL ?= 1
CFG_FTRACE_BUF_SIZE=4096
CFLAGS_ta_arm64=-O0

# The UUID for the Trusted Application
BINARY=0bbd4f5b-ab7b-4dcd-a1f5-9b2eedaf0d1e

-include $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk

ifeq ($(wildcard $(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk), )
clean:
	@echo 'Note: $$(TA_DEV_KIT_DIR)/mk/ta_dev_kit.mk not found, cannot clean TA'
	@echo 'Note: TA_DEV_KIT_DIR=$(TA_DEV_KIT_DIR)'
endif
