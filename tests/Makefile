include ../common.mk

DIR_DENYM = ../denym
LIB_DENYM = $(DIR_DENYM)/$(DIR_BIN)/$(SO_NAME)
WIN_LIB_DENYM = $(DIR_DENYM)/$(WIN_DIR_BIN)/$(WIN_LIB_NAME)
DIR_SHADERS = resources/shaders

LFLAGS = -lvulkan -lglfw -ldenym -L$(DIR_DENYM)/$(DIR_BIN) -lm
WIN_LFLAGS = -lvulkan-1.lib -lglfw3_mt.lib -ldenym -L$(DIR_DENYM)/$(WIN_DIR_BIN) -L$(DIR_GLFW) -L"C:\VulkanSDK\1.3.231.1\Lib"
WIN_LFLAGS += -lkernel32.lib -luser32.lib -lgdi32.lib -lwinspool.lib -lcomdlg32.lib -ladvapi32.lib -lshell32.lib -lole32.lib -loleaut32.lib -luuid.lib -lodbc32.lib -lodbccp32.lib -llibucrt.lib

INC = -I$(DIR_INC) -I$(DIR_DENYM)/include -I$(DIR_CGLM)

SRC = $(wildcard $(DIR_SRC)/*.c)
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))
EXE = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%, $(SRC))
WIN_EXE = $(patsubst $(DIR_SRC)/%.c, $(WIN_DIR_BIN)/%.exe, $(SRC))
TEST_TARGETS = $(patsubst $(DIR_SRC)/%.c, run_%, $(SRC))
WIN_TEST_TARGETS = $(patsubst $(DIR_SRC)/%.c, win_run_%, $(SRC))
DEBUG_TARGETS = $(patsubst $(DIR_SRC)/%.c, debug_%, $(SRC))
WIN_DEBUG_TARGETS = $(patsubst $(DIR_SRC)/%.c, win_debug_%, $(SRC))

SHADER_SRC = $(wildcard $(DIR_SHADERS)/*.vert) $(wildcard $(DIR_SHADERS)/*.frag)
SHADERS_SPV = $(patsubst $(DIR_SHADERS)/%, $(DIR_SHADERS)/%.spv, $(SHADER_SRC))

-include $(DEP)

all: $(EXE) $(WIN_EXE) shaders

shaders: $(SHADERS_SPV)

# create one run_* target per .c, and allow to execute the test
$(TEST_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst run_%, run/%, $@)

$(WIN_TEST_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst win_run_%, win_run/%, $@)

# same for debug targets
$(DEBUG_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst debug_%, debug/%, $@)

$(WIN_DEBUG_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst win_debug_%, win_debug/%, $@)

# run the specified executable
run/%: $(DIR_BIN)/% shaders
	LD_LIBRARY_PATH=$(DIR_DENYM)/$(DIR_BIN) ./$<

win_run/%: $(WIN_DIR_BIN)/%.exe shaders
	./$<

# run the specified executable, with debugger attached
debug/%: $(DIR_BIN)/% shaders
	LD_LIBRARY_PATH=$(DIR_DENYM)/$(DIR_BIN) $(DBG) ./$<

win_debug/%: $(WIN_DIR_BIN)/%.exe shaders
	$(WIN_DBG) ./$<

$(LIB_DENYM) $(WIN_LIB_DENYM): FORCE
	@$(MAKE) --no-print-directory -C $(DIR_DENYM)

FORCE:

$(DIR_BIN)/%: $(DIR_SRC)/%.c $(LIB_DENYM) | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) $(LFLAGS) $(INC) -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) $< -o $@

$(WIN_DIR_BIN)/%.exe: $(DIR_SRC)/%.c $(WIN_LIB_DENYM) | $(WIN_DIR_BIN) $(DIR_DEP)
	$(WIN_CC) $(CFLAGS) $(WIN_LFLAGS) $(INC) $< -o $@
	@chmod u+x $@

$(DIR_SHADERS)/%.spv: $(DIR_SHADERS)/%
	glslangValidator.exe -V $< -o $@

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP) $(WIN_DIR_BIN) $(wildcard $(DIR_SHADERS)/*.spv)
	@$(MAKE) --no-print-directory -C $(DIR_DENYM) clean
