PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/GPTM/hal_st_lld.c

ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/GPTM/hal_gpt_lld.c
endif
else
PLATFORMSRC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/GPTM/hal_gpt_lld.c
endif

PLATFORMINC += $(CHIBIOS_CONTRIB)/os/hal/ports/TIVA/LLD/GPTM
