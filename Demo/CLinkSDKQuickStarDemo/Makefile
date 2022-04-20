Q := @

TOP_DIR := $(shell pwd)
DEMOS_DIR := demos
OUT_DIR := output

CC := gcc 

BLD_CFLAGS := $(CFLAGS) -Wall -Werror
BLD_CFLAGS += -Os

BLD_LDFLAGS := $(LDFLAGS) -lpthread

#找到除了demos下的文件夹路径
SRC_DIR := $(shell find . -type d \( ! -name demos \)) 

#在每个路径前添加-I 方便编译时添加
HDR_DIR := $(SRC_DIR)
BLD_CFLAGS += $(addprefix -I,$(HDR_DIR))

#找到除了demos文件夹下的其余的所有的.c
SRC_FILES := $(shell find $(SRC_DIR) -not -path "*/demos/*.c" -name "*.c")

#将所有的.c替换成.o 并在每个文件前加前缀 output/
OBJ_FILES := $(SRC_FILES:.c=.o)
OBJ_FILES := $(addprefix $(OUT_DIR)/,$(OBJ_FILES))

#把文件mqtt_basic_demo.c 改为mqtt-basic-demo  patsubs函数去除.c后缀
PROG_TARGET := $(subst _,-,$(patsubst %.c,%,$(wildcard demos/*_demo.c)))


all: prepare $(PROG_TARGET)

prepare:
	$(Q)mkdir -p output

$(PROG_TARGET): $(OBJ_FILES)
#dir 取第一个斜线的目录 notdir 取最后一个目录  42行 就是把demo.c
	$(Q)echo "+ Linking $(OUT_DIR)/$(notdir $@) ..."
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CC) -o $@ \
	    $(patsubst $(OUT_DIR)/%,%,$(addsuffix .c,$(subst $(notdir $@),$(subst -,_,$(notdir $@)),$@))) \
	    $(BLD_CFLAGS) $^ $(BLD_LDFLAGS)

	$(Q)mv $@ $(OUT_DIR)

$(OUT_DIR)/%.o: %.c
	$(Q)echo ": Compiling $< ..."
	$(Q)mkdir -p $(OUT_DIR)/$(dir $<)
	$(Q)$(CC) -o $@ -c $< $(BLD_CFLAGS)

clean:
	$(Q)rm -rf $(OUT_DIR)
