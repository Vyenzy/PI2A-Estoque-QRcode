#ifndef MAN_DB_H
#define MAN_DB_H

#include <sqlite3.h>

int conectar_banco(sqlite3 **banco);
void fechar_banco(sqlite3 *banco);

void inserir_lote(sqlite3 *banco);
void buscar_vencimentos(sqlite3 *banco);
void listar_produtos(sqlite3 *banco);

#endif