# Settings generator
.PHONY: settings clean-settings
UTILS_DIR               = $(ROOT)/src/utils
SETTINGS_GENERATOR      = $(UTILS_DIR)/settings.rb

GENERATED_SETTINGS      = $(TARGET_OBJ_DIR)/settings_generated.h $(TARGET_OBJ_DIR)/settings_generated.c
SETTINGS_FILE           = $(SRC_DIR)/fc/settings.yaml
GENERATED_FILES         = $(GENERATED_SETTINGS)
$(GENERATED_SETTINGS): $(SETTINGS_GENERATOR) $(SETTINGS_FILE) $(STAMP)

CLEAN_ARTIFACTS			+= $(GENERATED_SETTINGS)

# Make sure the generated files are in the include path
CFLAGS                  += -I$(TARGET_OBJ_DIR)

# Use a pattern rule, since they're different than normal rules.
# See https://www.gnu.org/software/make/manual/make.html#Pattern-Examples
%generated.h %generated.c:
	$(V1) echo "settings.yaml -> settings_generated.h, settings_generated.c" "$(STDOUT)"
	$(V1) CPP_PATH="$(ARM_SDK_DIR)/bin" CFLAGS="$(CFLAGS)" TARGET=$(TARGET) ruby $(SETTINGS_GENERATOR) . $(SETTINGS_FILE) -o $(TARGET_OBJ_DIR)

settings-json:
	$(V0) CPP_PATH="$(ARM_SDK_DIR)/bin" CFLAGS="$(CFLAGS)" TARGET=$(TARGET) ruby $(SETTINGS_GENERATOR) . $(SETTINGS_FILE) --json settings.json

clean-settings:
	$(V1) $(RM) $(GENERATED_SETTINGS)
