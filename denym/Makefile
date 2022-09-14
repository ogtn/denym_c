include ../common.mk

LFLAGS = -fPIC

INC = -I$(DIR_INC) -I$(DIR_CGLM) -I$(DIR_STB)

SRC = $(wildcard $(DIR_SRC)/*.c)
OBJ = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%.o, $(SRC))
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))
COMPIL_DB = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.o.json, $(SRC))

-include $(DEP)

all: $(DIR_BIN)/$(LIB_NAME)

$(DIR_BIN)/$(LIB_NAME): $(OBJ)
	$(CC) $(LFLAGS) -shared -Wl,-soname,$(LIB_NAME) $^ -o $@

$(DIR_BIN)/%.o: $(DIR_SRC)/%.c | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) -D_DEBUG $(LFLAGS) $(INC) -c -MJ $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.o.json, $<) -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) $< -o $@

$(DIR_DEP)/compile_commands.json: all
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $(COMPIL_DB) > $(DIR_DEP)/compile_commands.json

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP)
