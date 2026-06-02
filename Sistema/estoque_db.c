#include "estoque_db.h"
#include "libs/sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

int carregarProdutos(Produto *lista)
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) 
    {
        return 0;
    }

    const char *sql = 
        "SELECT p.codigo, p.nome, "
        "IFNULL(l.quantidade, 0) as qtd_lote, "
        "p.qtd_minima, "
        "CAST(julianday(l.validade) - julianday(date('now', 'localtime')) AS INT) as dias_restantes, "
        "p.qtd_atual, "
        "IFNULL(l.codigo_lote, '-') as lote "
        "FROM produtos p "
        "LEFT JOIN lotes l ON p.id = l.produto_id AND l.status = 'ativo' AND l.quantidade > 0 "
        "ORDER BY dias_restantes IS NULL, dias_restantes ASC, p.id DESC;";
    
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_PRODUTOS)
        {
            strcpy(lista[count].codigo, (const char*)sqlite3_column_text(stmt, 0));
            strcpy(lista[count].nome, (const char*)sqlite3_column_text(stmt, 1));
            
            int qtd_lote = sqlite3_column_int(stmt, 2);
            int qtd_minima = sqlite3_column_int(stmt, 3);
            int qtd_total = sqlite3_column_int(stmt, 5); 
            
            strcpy(lista[count].codigo_lote, (const char*)sqlite3_column_text(stmt, 6)); 
            lista[count].quantidade = qtd_lote;
            
            if (sqlite3_column_type(stmt, 4) == SQLITE_INTEGER) 
            {
                lista[count].dias_para_vencer = sqlite3_column_int(stmt, 4);
            }
            else 
            {
                lista[count].dias_para_vencer = 999; 
            }
            
            if (qtd_total == 0 || qtd_lote == 0)
            {
                strcpy(lista[count].status, "ESGOTADO");
                lista[count].quantidade = 0;
            } 
            else if (lista[count].dias_para_vencer <= 0)
            {
                strcpy(lista[count].status, "VENCIDO"); 
            }
            else if (lista[count].dias_para_vencer <= 14 || qtd_total < qtd_minima)
            {
                strcpy(lista[count].status, "ALERTA");
            }
            else
            {
                strcpy(lista[count].status, "OK");
            }
            
            count++;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

int carregarResumoDia(ResumoDia *lista)
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) 
    {
        return 0;
    }

    const char *sql = 
        "SELECT p.nome, "
        "IFNULL(SUM(CASE WHEN m.tipo = 'entrada' THEN m.quantidade ELSE 0 END), 0) as entradas, "
        "IFNULL(SUM(CASE WHEN m.tipo = 'saida' THEN m.quantidade ELSE 0 END), 0) as saidas, "
        "p.qtd_atual "
        "FROM produtos p "
        "LEFT JOIN movimentacoes m ON p.id = m.produto_id "
        "GROUP BY p.id "
        "HAVING entradas > 0 OR saidas > 0 OR p.qtd_atual > 0 "
        "ORDER BY p.nome ASC;";
        
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_PRODUTOS)
        {
            strcpy(lista[count].produto_nome, (const char*)sqlite3_column_text(stmt, 0));
            lista[count].entradas = sqlite3_column_int(stmt, 1);
            lista[count].saidas = sqlite3_column_int(stmt, 2);
            lista[count].saldo_total = sqlite3_column_int(stmt, 3);
            count++;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

void simularEntradaQR(const char *nomeProduto, const char *codigoProduto, const char *codigoLote, int quantidade, int diasValidade)
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) 
    {
        return;
    }

    int produto_id = 0;
    sqlite3_stmt *stmt;
    char sql[512];

    snprintf(sql, sizeof(sql), "SELECT id FROM produtos WHERE nome = '%s';", nomeProduto);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW) 
        {
            produto_id = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    if (produto_id == 0)
    {
        snprintf(sql, sizeof(sql), "INSERT INTO produtos (codigo, nome, categoria, qtd_minima, qtd_atual) VALUES ('%s', '%s', 'Geral', 20, 0);", codigoProduto, nomeProduto);
        sqlite3_exec(db, sql, NULL, 0, NULL);
        produto_id = (int)sqlite3_last_insert_rowid(db);
    }

    time_t agora = time(NULL);
    time_t data_futura = agora + (diasValidade * 24 * 60 * 60);
    struct tm *data_struct = localtime(&data_futura);
    char data_str[20];
    strftime(data_str, sizeof(data_str), "%Y-%m-%d", data_struct);

    snprintf(sql, sizeof(sql), "INSERT INTO lotes (produto_id, codigo_lote, quantidade, validade, status) VALUES (%d, '%s', %d, '%s', 'ativo');", produto_id, codigoLote, quantidade, data_str);
    sqlite3_exec(db, sql, NULL, 0, NULL);
    
    int lote_id = (int)sqlite3_last_insert_rowid(db); 

    snprintf(sql, sizeof(sql), "INSERT INTO movimentacoes (lote_id, produto_id, tipo, quantidade) VALUES (%d, %d, 'entrada', %d);", lote_id, produto_id, quantidade);
    sqlite3_exec(db, sql, NULL, 0, NULL);

    snprintf(sql, sizeof(sql), "UPDATE produtos SET qtd_atual = IFNULL((SELECT SUM(quantidade) FROM lotes WHERE produto_id = %d AND status = 'ativo'), 0) WHERE id = %d;", produto_id, produto_id);
    sqlite3_exec(db, sql, NULL, 0, NULL);

    sqlite3_close(db);
}

void simularVendasDoDia()
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) 
    {
        return;
    }

    const char *sql_select = "SELECT l.id, p.id, p.nome, l.quantidade FROM lotes l JOIN produtos p ON l.produto_id = p.id WHERE l.quantidade > 0 AND l.status = 'ativo' ORDER BY RANDOM() LIMIT 5;";
    sqlite3_stmt *stmt;
    
    typedef struct 
    { 
        int lote_id; 
        int prod_id; 
        char nome[50]; 
        int qtd_disp; 
    } LoteSorteado;
    
    LoteSorteado sorteados[5];
    int num_sorteados = 0;

    if (sqlite3_prepare_v2(db, sql_select, -1, &stmt, NULL) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW && num_sorteados < 5)
        {
            sorteados[num_sorteados].lote_id = sqlite3_column_int(stmt, 0);
            sorteados[num_sorteados].prod_id = sqlite3_column_int(stmt, 1);
            strcpy(sorteados[num_sorteados].nome, (const char*)sqlite3_column_text(stmt, 2));
            sorteados[num_sorteados].qtd_disp = sqlite3_column_int(stmt, 3);
            num_sorteados++;
        }
    }
    sqlite3_finalize(stmt);

    char sql_update[512];
    for (int i = 0; i < num_sorteados; i++)
    {
        int qtd_comprada = (rand() % 5) + 1; 
        if (qtd_comprada > sorteados[i].qtd_disp) 
        {
            qtd_comprada = sorteados[i].qtd_disp; 
        }

        snprintf(sql_update, sizeof(sql_update), "UPDATE lotes SET quantidade = quantidade - %d WHERE id = %d;", qtd_comprada, sorteados[i].lote_id);
        sqlite3_exec(db, sql_update, NULL, 0, NULL);

        snprintf(sql_update, sizeof(sql_update), "UPDATE produtos SET qtd_atual = IFNULL((SELECT SUM(quantidade) FROM lotes WHERE produto_id = %d AND status = 'ativo'), 0) WHERE id = %d;", sorteados[i].prod_id, sorteados[i].prod_id);
        sqlite3_exec(db, sql_update, NULL, 0, NULL);

        snprintf(sql_update, sizeof(sql_update), "INSERT INTO movimentacoes (lote_id, produto_id, tipo, quantidade) VALUES (%d, %d, 'saida', %d);", sorteados[i].lote_id, sorteados[i].prod_id, qtd_comprada);
        sqlite3_exec(db, sql_update, NULL, 0, NULL);

        char cliente_id[20];
        sprintf(cliente_id, "CLI-%04d", rand() % 9999);
        snprintf(sql_update, sizeof(sql_update), "INSERT INTO vendas (cliente_id, produto_nome, quantidade) VALUES ('%s', '%s', %d);", cliente_id, sorteados[i].nome, qtd_comprada);
        sqlite3_exec(db, sql_update, NULL, 0, NULL);
    }
    sqlite3_close(db);
}

void realizarVendaCliente(const char *codigoLote, const char *nomeProduto)
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) 
    {
        return;
    }

    char sql[512];
    int lote_id = 0, prod_id = 0;

    snprintf(sql, sizeof(sql), "SELECT l.id, p.id FROM lotes l JOIN produtos p ON l.produto_id = p.id WHERE l.codigo_lote = '%s';", codigoLote);
    sqlite3_stmt *stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            lote_id = sqlite3_column_int(stmt, 0);
            prod_id = sqlite3_column_int(stmt, 1);
        }
    }
    sqlite3_finalize(stmt);

    if (lote_id != 0 && prod_id != 0)
    {
        snprintf(sql, sizeof(sql), "UPDATE lotes SET quantidade = quantidade - 1 WHERE id = %d;", lote_id);
        sqlite3_exec(db, sql, NULL, 0, NULL);

        snprintf(sql, sizeof(sql), "UPDATE produtos SET qtd_atual = IFNULL((SELECT SUM(quantidade) FROM lotes WHERE produto_id = %d AND status = 'ativo'), 0) WHERE id = %d;", prod_id, prod_id);
        sqlite3_exec(db, sql, NULL, 0, NULL);

        snprintf(sql, sizeof(sql), "INSERT INTO movimentacoes (lote_id, produto_id, tipo, quantidade) VALUES (%d, %d, 'saida', 1);", lote_id, prod_id);
        sqlite3_exec(db, sql, NULL, 0, NULL);

        snprintf(sql, sizeof(sql), "INSERT INTO vendas (cliente_id, produto_nome, quantidade) VALUES ('APP-CLIENTE', '%s', 1);", nomeProduto);
        sqlite3_exec(db, sql, NULL, 0, NULL);
    }
    sqlite3_close(db);
}

void limparBancoDemo()
{
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) == SQLITE_OK)
    {
        sqlite3_exec(db, "DELETE FROM movimentacoes;", NULL, 0, NULL); 
        sqlite3_exec(db, "DELETE FROM lotes;", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM produtos;", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM vendas;", NULL, 0, NULL); 
        
        // Zera os IDs incrementais para manter o banco "novo"
        sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='movimentacoes';", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='lotes';", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='produtos';", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM sqlite_sequence WHERE name='vendas';", NULL, 0, NULL);
        
        sqlite3_close(db);
    }
}