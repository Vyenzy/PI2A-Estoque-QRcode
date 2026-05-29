#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "man_db.h"

// Funcao interna para imprimir a tabela bonitinha no terminal
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for(int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("-------------------------------------------------------\n");
    return 0;
}

int conectar_banco(sqlite3 **banco) {
   int status = sqlite3_open("banco_estoque.db", banco);
   
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

    printf("\n=== INSERIR LOTE (LEITURA QR CODE) ===\n");

    printf("ID do produto: ");
    scanf("%d", &produto_id);

    printf("Codigo do lote (Ex: LOT001): ");
    scanf("%s", codigo_lote);

    printf("Quantidade: ");
    scanf("%d", &quantidade);

    printf("Validade (AAAA-MM-DD): ");
    scanf("%s", validade);

    char sql[500];
    
    sprintf(sql, "INSERT INTO lotes (produto_id, codigo_lote, quantidade, validade, status) VALUES (%d, '%s', %d, '%s', 'ativo');", produto_id, codigo_lote, quantidade, validade);
    
    int status = sqlite3_exec(banco, sql, 0, 0, &erro);
    
    if(status != SQLITE_OK) {
        printf("Erro ao inserir Lote: %s\n", erro);
        sqlite3_free(erro);
    } else {
        printf("Lote %s inserido e monitorado com sucesso!\n", codigo_lote);
    }
}

void buscar_vencimentos(sqlite3 *banco) {
    char *erro = 0;

    char *sql = "SELECT l.codigo_lote, p.nome, l.validade "
                "FROM lotes l "
                "JOIN produtos p ON l.produto_id = p.id "
                "WHERE date(l.validade) <= date('now', '+7 days') "
                "AND l.status = 'ativo';";

    printf("\n=== ALERTA: PRODUTOS VENCENDO EM 7 DIAS (VITRINE) ===\n");

    int status = sqlite3_exec(banco, sql, callback, 0, &erro);
    
    if(status != SQLITE_OK) {
        printf("Erro na busca: %s\n", erro);
        sqlite3_free(erro);
    }
}

void listar_produtos(sqlite3 *banco) {
    char *erro = 0;

    char *sql = "SELECT codigo, nome, qtd_atual, qtd_minima FROM produtos;";
    
    printf("\n=== VISAO GERAL DE PRODUTOS ===\n");

    int status = sqlite3_exec(banco, sql, callback, 0, &erro);
    
    if(status != SQLITE_OK) {
        printf("Erro na listagem: %s\n", erro);
        sqlite3_free(erro);
    }
}