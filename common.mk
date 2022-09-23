DIR_SRC = src
DIR_INC = include
DIR_BIN = bin/linux
DIR_DEP = dep
WIN_DIR_BIN = bin/win
WIN_DIR_VULKAN = "C:\VulkanSDK\1.3.224.1\Include"
WIN_DIR_VULKAN_ = "$(cmd.exe /c echo %VULKAN_SDK% | tr -d '\r')"

DIR_CGLM = ../dependencies/cglm/include/
DIR_STB = ../dependencies/stb/
DIR_FASTOBJ = ../dependencies/fast_obj
DIR_GLFW = ../lib

SO_NAME = libdenym.so
WIN_LIB_NAME = denym.lib

CC = clang
WIN_CC = /mnt/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Professional/VC/Tools/Llvm/bin/clang.exe
WIN_LD = /mnt/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Professional/VC/Tools/Llvm/bin/llvm-lib.exe
DBG = lldb
WIN_DBG = /mnt/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Professional/VC/Tools/Llvm/bin/lldb.exe

# hardcore mode :)
# http://blogs.gnome.org/otte/2008/12/22/warning-options/
# http://mces.blogspot.fr/2008/12/year-end-cleaning-ie-on-warning-options.html
# http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
CFLAGS = -Wall -Wextra -fno-common \
	-Wformat=2 -Winit-self -Winline -Wpacked -Wp,-D_FORTIFY_SOURCE=2 \
	-Wpointer-arith -Wlarger-than-65500 -Wmissing-declarations \
	-Wmissing-format-attribute -Wmissing-noreturn \
	-Wnested-externs -Wold-style-definition -Wredundant-decls \
	-Wsign-compare -Wstrict-aliasing=2 -Wstrict-prototypes -Wswitch \
	-Wundef -Wunreachable-code -Wwrite-strings -Wconversion \
	-Wenum-compare -Wpadded -pedantic #-Weverything -Wdeclaration-after-statement

CFLAGS += -Werror=implicit-function-declaration -Werror=return-type \
	-Werror=incompatible-pointer-types -Werror=missing-prototypes

CFLAGS += -Wno-missing-field-initializers -Wno-unused-parameter

CFLAGS += -std=c17 -m64 -O0 -g

#CFLAGS += -fsanitize=memory -fsanitize-address-use-after-scope -fsanitize=undefined -fsanitize-recover=memory -fno-omit-frame-pointer #-fsanitize=address
#LFAGS = -fsanitize=memory -fsanitize-address-use-after-scope -fsanitize=undefined -fsanitize-recover=memory -fno-omit-frame-pointer #-fsanitize=address

print_dbg:
	@echo $(WIN_DIR_VULKAN)
	@echo $(WIN_DIR_VULKAN_)
	@cmd.exe /c echo %VULKAN_SDK% | tr -d '\r'

$(DIR_BIN):
	@mkdir -p $@

$(DIR_DEP):
	@mkdir -p $@

$(WIN_DIR_BIN):
	@mkdir -p $@

.PHONY: clean all

.DEFAULT_GOAL:= all
