#ifndef ESTOQUE_DB_H
#define ESTOQUE_DB_H

#define MAX_PRODUTOS 500
#define DB_PATH "dados/banco_estoque.db"
typedef struct {
    char codigo[20];
    char nome[50];
    char codigo_lote[30]; 
    int quantidade;
    char status[20];
    int dias_para_vencer; 
} Produto;

typedef struct {
    char produto_nome[50];
    int entradas;      
    int saidas;        
    int saldo_total;   
} ResumoDia;

int carregarProdutos(Produto *lista);
int carregarResumoDia(ResumoDia *lista); 
void simularEntradaQR(const char *nomeProduto, const char *codigoProduto, const char *codigoLote, int quantidade, int diasValidade);
void simularVendasDoDia();
void limparBancoDemo();

#endif