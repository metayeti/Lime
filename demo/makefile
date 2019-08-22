define create_dir
	test -d $(1) || mkdir $(1)
endef

PROJNAME := demo

CC := g++
ifeq ($(BUILD),DEBUG)
	CFLAGS := -O0 -g -Wall
else
	CFLAGS := -O3
endif
CSTD := c++17
CLIBS := -lz -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
CINCLUDE := -I../unlime
TARGET := bin/$(PROJNAME)

SRC_DIR := src
OBJ_DIR := obj

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

LDFLAGS := -std=$(CSTD) $(CFLAGS)

all: $(TARGET)
debug:
	make "BUILD=DEBUG"
$(TARGET): $(OBJ_FILES)
	@$(call create_dir,$(@D))
	$(CC) $(LDFLAGS) -o $@ $^ $(CLIBS)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@$(call create_dir,$(@D))
	$(CC) -std=$(CSTD) -c $(CFLAGS) $(CINCLUDE) -o $@ $<
clean:
	rm -rf $(OBJ_DIR)/*.o

.PHONY: all clean

