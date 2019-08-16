define create_dir
	test -d $(1) || mkdir $(1)
endef

PROJNAME := lime

CC := g++
CFLAGS := -Wall -g
CSTD := c++17
CLIBS := -lz
TARGET := bin/$(PROJNAME)

SRC_DIR := src
OBJ_DIR := obj

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

LDFLAGS := $(CFLAGS) -std=$(CSTD)

all: $(TARGET)
$(TARGET): $(OBJ_FILES)
	@$(call create_dir,$(@D))
	$(CC) $(LDFLAGS) -o $@ $^ $(CLIBS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call create_dir,$(@D))
	$(CC) -std=$(CSTD) -c -o $@ $<
clean:
	rm -rf $(OBJ_DIR)/*.o

.PHONY: all clean

