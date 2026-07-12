TARGET = main

# --- COMPILAÇÃO MULTIPLATAFORMA ---
# compila de acordo com o sistema operacional
ifdef windows
	# compila código WINDOWS
    # Windows 32-bits
    #CC = i686-w64-mingw32-gcc
    # Windows 64-bits
    CC = x86_64-w64-mingw32-gcc
    EXT = .exe
    C_STANDARD = -std=c2x
    LIBS = -L lib -lpdcurses
    INCLUDES = -I include_win -I include
else
	# compila código LINUX
    CC = gcc
    EXT =
    C_STANDARD = -std=c23
    LIBS = -lncurses
    INCLUDES = -I include
endif

# Define o compilador e flags para retorno de erros
CFLAGS = $(C_STANDARD) -Wall -Wextra -g $(INCLUDES)

# Diretórios
SRC_DIR = src
APP_DIR = apps
INC_DIR = include
OBJ_DIR = obj

# Lê todos os arquivos .c nas pastas src/ e apps/
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
APP_FILES = $(wildcard $(APP_DIR)/*.c)

# Define os aquivos de objetos .o correspondente dentro de obj/
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
APP_OBJ_FILES = $(patsubst $(APP_DIR)/%.c, $(OBJ_DIR)/apps/%.o, $(APP_FILES))

# Lê os binários (se precisar executar os códigos de forma individual, trocar TARGET por PROGS)
# adicionado a extensão dinâmica .exe se for Windows
all: $(TARGET)$(EXT)

# Linkagem: junta todos os objetos em um arquivo executável
$(TARGET)$(EXT): $(OBJ_FILES) $(APP_OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# compila os arquivos na pasta src/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# compilação do arquivos na pasta e coloca-os na pasta obj/apps/ (cria ela se necessário)
$(OBJ_DIR)/apps/%.o: $(APP_DIR)/%.c | $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/apps
	$(CC) $(CFLAGS) -c $< -o $@

# cria a pasta obj/ se ela não existir
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Limpeza completa
clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET).exe
