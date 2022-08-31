DIR_SRC = src
DIR_INC = include
DIR_BIN = bin
DIR_DEP = dep

CC = clang

# hardcore mode :)
# http://blogs.gnome.org/otte/2008/12/22/warning-options/
# http://mces.blogspot.fr/2008/12/year-end-cleaning-ie-on-warning-options.html
# http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
CFLAGS = -Wall -Wextra -fno-common -Wdeclaration-after-statement \
-Wformat=2 -Winit-self -Winline -Wpacked -Wp,-D_FORTIFY_SOURCE=2 \
-Wpointer-arith -Wlarger-than-65500 -Wmissing-declarations \
-Wmissing-format-attribute -Wmissing-noreturn -Wmissing-prototypes \
-Wnested-externs -Wold-style-definition -Wredundant-decls \
-Wsign-compare -Wstrict-aliasing=2 -Wstrict-prototypes -Wswitch \
-Wundef -Wunreachable-code -Wwrite-strings -Wconversion \
-Werror=implicit-function-declaration -Wenum-compare -Wpadded \
--std=c17 \
-Wno-missing-field-initializers \
#-pedantic #-pedantic-errors

LFLAGS = -fPIC

LIB_NAME = libdenym.so
INC = -I$(DIR_INC)

SRC = $(wildcard $(DIR_SRC)/*.c)
OBJ = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%.o, $(SRC))
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))

all: $(DIR_BIN)/$(LIB_NAME)

$(DIR_BIN)/$(LIB_NAME): $(OBJ)
	$(CC) $(LFLAGS) -shared -Wl,-soname,$(LIB_NAME) -o $@ $<

$(DIR_BIN)/%.o: $(DIR_SRC)/%.c | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) $(LFLAGS) $(INC) -c -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) -o $@ $<

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP)

-include $(DEP)

$(DIR_BIN):
	@mkdir $@

$(DIR_DEP):
	@mkdir $@

.PHONY: clean all
