#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "man_db.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for(int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n-------------------------------------------------------\n");
    return 0;
}

int conectar_banco(sqlite3 **banco) {
   int status = sqlite3_open("\nbanco_estoque.db", banco);
   if(status != SQLITE_OK) {
    printf("Erro ao conectar no banco.\n");

    return 0;
   }

   printf("Banco conectado com sucesso!\n");
    return 1;
}

void fechar_banco(sqlite3 *banco) {
    sqlite3_close(banco);
    printf("Banco fechado!\n");
}

void inserir_lote(sqlite3 *banco) {
    char *erro = 0;
    int produto_id;
    char codigo_lote[50];
    int quantidade;
    char validade[20];

    printf("\n=== INSERIR LOTE ===\n");

    printf("ID do produto: ");
    scanf("%d", &produto_id);

    printf("Codigo do lote: ");
    scanf("%s", codigo_lote);

    printf("Quantidade: ");
    scanf("%d", &quantidade);

    printf("Validade (DD-MM-AAAA): ");
    scanf("%s", validade);

    char sql[500];

    sprintf(sql, "INSERT INTO lotes " "(produtos_id, codigo_lote, quantidade, validade) " "VALUES " "(%d, '%s', %d, '%s');", produto_id, codigo_lote, quantidade, validade);
    
    int status = sqlite3_exec(banco, sql, 0, 0, &erro);
    if(status != SQLITE_OK) {
        printf("Erro INSERT; %s\n", erro);
        sqlite3_free(erro);
    } else {
        printf("Lote inserido!\n");
    }
}

void buscar_vencimentos(sqlite3 *banco) {
    char *erro = 0;

    char *sql = "SELECT l.codigo_lote, "
                "p.nome_produto, "
                "l.validade "
                "FROM lotes l "
                "JOIN produtos p ON "
                "l.produtos_id = p.id "
                "WHERE date(l.validade) < date('now', '+7 days') "
                "AND l.status = 'ativo';";

    printf("\n=== VENCIMENTOS ===\n");

    int status = sqlite3_exec(banco, sql, callback, 0, &erro);
    if(status != SQLITE_OK) {
        printf("Erro SELECT: %s\n", erro);
        sqlite3_free(erro);
    }
}

void listar_produtos(sqlite3 *banco) {
    char *erro = 0;
    char *sql = "SELECT id, nome, categoria, qtd_atual " "FROM produtos;";
    printf("\n=== LISTA DE PRODUTOS ===\n");

    int status = sqlite3_exec(banco, sql, callback, 0, &erro);
if(status != SQLITE_OK) {
    printf("Erro SELECT: %s\n", erro);
    sqlite3_free(erro);
}
}