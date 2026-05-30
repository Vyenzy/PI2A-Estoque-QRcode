#ifndef ESTOQUE_DB_H
#define ESTOQUE_DB_H

#define MAX_PRODUTOS 500
#define MAX_VENDAS 100
#define DB_PATH "dados/banco_estoque.db"

typedef struct 
{
    char codigo[20];
    char nome[50];
    int quantidade;
    char status[20];
    int dias_para_vencer; 
} Produto;

typedef struct 
{
    char cliente_id[20];
    char produto_nome[50];
    int quantidade;
    char data_hora[30];
} Venda;

int carregarProdutos(Produto *lista);
int carregarVendas(Venda *lista);
void simularEntradaQR(const char *nomeProduto, const char *codigoProduto, const char *codigoLote, int quantidade, int diasValidade);
void simularVendasDoDia();
void limparBancoDemo();

#endif