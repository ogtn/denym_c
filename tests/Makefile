include ../common.mk

DIR_DENYM = ../denym
LIB_DENYM = $(DIR_DENYM)/$(DIR_BIN)/$(LIB_NAME)
DIR_SHADERS = resources/shaders

LFLAGS = -lvulkan -lglfw -ldenym -L$(DIR_DENYM)/bin -lm

INC = -I$(DIR_INC) -I$(DIR_DENYM)/include -I$(DIR_CGLM)

SRC = $(wildcard $(DIR_SRC)/*.c)
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))
EXE = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%, $(SRC))
TEST_TARGETS = $(patsubst $(DIR_SRC)/%.c, run_%, $(SRC))
DEBUG_TARGETS = $(patsubst $(DIR_SRC)/%.c, debug_%, $(SRC))

SHADER_SRC = $(wildcard $(DIR_SHADERS)/*.vert) $(wildcard $(DIR_SHADERS)/*.frag)
SHADERS_SPV = $(patsubst $(DIR_SHADERS)/%, $(DIR_SHADERS)/%.spv, $(SHADER_SRC))

-include $(DEP)

all: $(EXE) shaders

shaders: $(SHADERS_SPV)

# create one run_* target per .c, and allow to execute the test
$(TEST_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst run_%, run/%, $@)

# same for debug targets
$(DEBUG_TARGETS):
	@$(MAKE) --no-print-directory $(patsubst debug_%, debug/%, $@)

# run the specified executable
run/%: $(DIR_BIN)/% shaders
	LD_LIBRARY_PATH=$(DIR_DENYM)/bin ./$<

# run the specified executable, with debugger attached
debug/%: $(DIR_BIN)/% shaders
	LD_LIBRARY_PATH=$(DIR_DENYM)/bin $(DB) ./$<

$(LIB_DENYM): FORCE
	@$(MAKE) --no-print-directory -C $(DIR_DENYM)

FORCE:

$(DIR_BIN)/%: $(DIR_SRC)/%.c $(LIB_DENYM) | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) $(LFLAGS) $(INC) -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) $< -o $@

$(DIR_SHADERS)/%.spv: $(DIR_SHADERS)/%
	glslangValidator -V $< -o $@

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP) $(wildcard $(DIR_SHADERS)/*.spv)
	@$(MAKE) --no-print-directory -C $(DIR_DENYM) clean
