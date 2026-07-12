TARGET = main

ifdef windows
    CC = x86_64-w64-mingw32-gcc
    EXT = .exe
    C_STANDARD = -std=c2x
    LIBS =
else
    CC = gcc
    EXT =
    C_STANDARD = -std=c23
    LIBS =
endif

CFLAGS = $(C_STANDARD) -Wall -Wextra -g -I include

SRC_DIR = src
APP_DIR = apps
INC_DIR = include
OBJ_DIR = obj

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
APP_FILES = $(wildcard $(APP_DIR)/*.c)

OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
APP_OBJ_FILES = $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/apps/%.o, $(APP_FILES))

all: $(TARGET)$(EXT)

$(TARGET)$(EXT): $(OBJ_FILES) $(APP_OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/%.o: $(APP_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/apps
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET).exe