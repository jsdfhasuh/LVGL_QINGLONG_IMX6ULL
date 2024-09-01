#
# Makefile
#
# ⭐全速编译命令： make clean && make -j$(nproc)

#CC ?= gcc
CC := arm-buildroot-linux-gnueabihf-gcc	# 注意配置工具链！
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0 -I$(LVGL_DIR)/  -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
CFLAGS += -I $(shell pwd)/include
LDFLAGS ?= -lm
BIN = 100ask_lvgl_demo


#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lvgl_func/lvgl_func.mk
include $(LVGL_DIR)/ql_api/ql_api.mk
include $(LVGL_DIR)/http/http.mk
include $(LVGL_DIR)/my_font/my_font.mk
include $(LVGL_DIR)/tool/tool.mk


OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES
$(info CSRCS = $(CSRCS))
$(info AOBJS = $(AOBJS))
$(info COBJS = $(COBJS))
$(info MAINOBJ = $(MAINOBJ))
$(info SRCS = $(SRCS))
$(info OBJS = $(OBJS))
## MAINOBJ -> OBJFILES

all: default


%.o: %.c
	$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)
	mkdir -p $(LVGL_DIR)/obj $(LVGL_DIR)/bin
## mv *.o $(LVGL_DIR)/obj/
	mv $(BIN) $(LVGL_DIR)/bin/
	cp $(LVGL_DIR)/bin/$(BIN) /home/jsdfhasuh/result

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ) ./bin/* ./obj/*


