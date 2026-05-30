# Nome do executável
TARGET = painel.exe

# O Operário (Compilador Cruzado)
CC = x86_64-w64-mingw32-gcc

# O Mapa (Onde achar os arquivos da Raylib e do SQLite)
INCLUDES = -I./libs
LIBS = -L./libs -lraylib -lopengl32 -lgdi32 -lwinmm

# Os Ingredientes (Seus códigos) - AQUI FOI ADICIONADO O ESTOQUE_DB.C!
SOURCES = interface.c estoque_db.c libs/sqlite3.c

# 1. Comando Padrão (Para desenvolvimento com o terminal de debug)
all:
	$(CC) $(SOURCES) -o $(TARGET) $(INCLUDES) $(LIBS)

# 2. Comando para a Banca (Oculta o terminal preto usando -mwindows)
banca:
	$(CC) $(SOURCES) -o $(TARGET) $(INCLUDES) $(LIBS) -mwindows

# 3. Comando para limpar a casa
clean:
	rm -f $(TARGET)