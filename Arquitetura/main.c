#include <stdio.h>
#include <stdlib.h>
#include "man_db.h"

int main() {
    sqlite3 *banco;
    if(!conectar_banco(&banco)) {
        return 1;
    
    }
    int opcao;
    do {
        printf("\n=======================================\n");
        printf("\nSISTEMA DE ESTOQUE QR CODE\n");
        printf("\n========================================\n");
        printf("1 - inserir lote\n");
        printf("2 - buscar vencimento\n");
        printf("3 - listar produtos\n");
        printf("0 - sair\n");
        printf("\nEscolha: ");
        scanf("%d", &opcao);
        switch(opcao) {
            case 1: inserir_lote(banco);
            break;
            case 2: buscar_vencimentos(banco);
            break;
            case 3: listar_produtos(banco);
            break;
            case 0: printf("Encerrando. . . . . . . . . .\n");
            break;
            default: printf("opcao invalida.\n");
        }
    } while(opcao != 0);
    fechar_banco(banco);
    return 0;
}