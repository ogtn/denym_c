include ../common.mk

LFLAGS = -fPIC
WIN_LFLAGS =

INC = -I$(DIR_INC) -I$(DIR_CGLM) -I$(DIR_STB) -I$(DIR_FASTOBJ)

SRC = $(wildcard $(DIR_SRC)/*.c)
OBJ = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%.o, $(SRC))
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))
WIN_OBJ = $(patsubst $(DIR_SRC)/%.c, $(WIN_DIR_BIN)/%.o, $(SRC))

#COMPIL_DB = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.o.json, $(SRC))

-include $(DEP)

all: $(DIR_BIN)/$(SO_NAME) $(WIN_DIR_BIN)/$(WIN_LIB_NAME)

$(DIR_BIN)/$(SO_NAME): $(OBJ)
	$(CC) $(LFLAGS) -shared -Wl,-soname,$(SO_NAME) $^ -o $@

$(WIN_DIR_BIN)/$(WIN_LIB_NAME): $(WIN_OBJ)
	$(WIN_LD) $(WIN_LFLAGS) $^ /OUT:$@

$(DIR_BIN)/%.o: $(DIR_SRC)/%.c | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) -D_DEBUG $(LFLAGS) $(INC) -c -MJ $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.o.json, $<) -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) $< -o $@

$(WIN_DIR_BIN)/%.o: $(DIR_SRC)/%.c | $(WIN_DIR_BIN) $(DIR_DEP)
	$(WIN_CC) $(CFLAGS) -D_DEBUG $(WIN_LFLAGS) $(INC) -I../include/ -I$(WIN_DIR_VULKAN) -c $< -o $@

#$(DIR_DEP)/compile_commands.json: all
#	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(COMPIL_DB) > $(DIR_DEP)/compile_commands.json

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP) $(WIN_DIR_BIN)