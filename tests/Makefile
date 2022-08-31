DIR_SRC = src
DIR_INC = include
DIR_BIN = bin
DIR_DEP = dep
DIR_DENYM = ../denym
DIR_SHADERS = resources/shaders

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
--std=c17 #-pedantic #-pedantic-errors
CFLAGS = -Wall -Wextra

LFLAGS = -lvulkan -lglfw -ldenym

INC = -I$(DIR_INC) -I$(DIR_DENYM)/include -L$(DIR_DENYM)/bin

SRC = $(wildcard $(DIR_SRC)/*.c)
DEP = $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $(SRC))
EXE = $(patsubst $(DIR_SRC)/%.c, $(DIR_BIN)/%, $(SRC))

SHADER_SRC = $(wildcard $(DIR_SHADERS)/*.vert) $(wildcard $(DIR_SHADERS)/*.frag)
SHADERS_SPV = $(patsubst $(DIR_SHADERS)/%, $(DIR_SHADERS)/%.spv, $(SHADER_SRC))

all: $(EXE) shaders

shaders: $(SHADERS_SPV)

$(DIR_BIN)/%: $(DIR_SRC)/%.c | $(DIR_BIN) $(DIR_DEP)
	$(CC) $(CFLAGS) $(LFLAGS) $(INC) -MMD -MF $(patsubst $(DIR_SRC)/%.c, $(DIR_DEP)/%.d, $<) -o $@ $<

$(DIR_SHADERS)/%.spv: $(DIR_SHADERS)/%
	glslangValidator -V -o $@ $<

clean:
	@rm -rf $(DIR_BIN) $(DIR_DEP) $(wildcard $(DIR_SHADERS)/*.spv)

-include $(DEP)

$(DIR_BIN):
	@mkdir $@

$(DIR_DEP):
	@mkdir $@

.PHONY: clean all
