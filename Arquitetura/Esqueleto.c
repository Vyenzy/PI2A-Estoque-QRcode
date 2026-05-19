#include <stdio.h>
#include <sqlite3.h> // Biblioteca para conectar C com o banco SQL

// Função que organiza e imprime os resultados da busca na tela, é só pesquisar as funções da biblioteca
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    printf("ALERTA DE DESCONTO - Produto proximo ao vencimento!\n");
    for (int i = 0; i < argc; i++) {
        printf("%s: %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("----------------------------------------\n");
    return 0;
}

int main() {
    sqlite3 *banco;
    char *mensagem_erro = 0;

    // 1. Abre a conexão com o arquivo do banco de dados local
    int status = sqlite3_open("banco_estoque.db", &banco);

    if (status) {
        printf("Erro ao conectar no banco de dados.\n");
        return 0;
    }

    // 2. Buscar os lotes vencendo em ate 7 dias
    char *sql = "SELECT l.codigo_lote, p.nome, l.validade, l.quantidade "
                "FROM lotes l JOIN produtos p ON l.produto_id = p.id "
                "WHERE l.validade <= DATE('now', '+7 days') AND l.status = 'ativo';";

    // 3. Executar a busca e chamar a função callback para imprimir na tela
    status = sqlite3_exec(banco, sql, callback, 0, &mensagem_erro);

    if (status != SQLITE_OK) {
        printf("Erro no SQL: %s\n", mensagem_erro);
        sqlite3_free(mensagem_erro);
    }

    // 4. Fecha a conexão com o banco
    sqlite3_close(banco);
    
    return 0;
}