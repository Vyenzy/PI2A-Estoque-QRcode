#include <stdio.h>
#include <stdlib.h>
#include "man_db.h"

void limpar_tela() 
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausar() 
{
    printf("\nPressione ENTER para voltar ao menu...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); 
    getchar();
}

int main() 
{
    sqlite3 *banco;
    if(!conectar_banco(&banco)) 
    {
        return 1;
    }
    
    int opcao;
    do {
        limpar_tela();
        
        printf("\n=======================================\n");
        printf("       SISTEMA DE ESTOQUE QR CODE      \n");
        printf("=======================================\n");
        printf("1 - Inserir lote\n");
        printf("2 - Buscar vencimentos (Vitrine)\n");
        printf("3 - Listar produtos\n");
        printf("0 - Sair\n");
        printf("\nEscolha: ");
        scanf("%d", &opcao);
        limpar_tela();
        
        switch(opcao) 
        {
            case 1: 
                inserir_lote(banco);
                pausar();
                break;
            case 2: 
                buscar_vencimentos(banco);
                pausar();
                break;
            case 3: 
                listar_produtos(banco);
                pausar();
                break;
            case 0: 
                printf("Encerrando o sistema...\n");
                break;
            default: 
                printf("Opcao invalida. Tente novamente.\n");
                pausar();
        }
    } while(opcao != 0);
    
    fechar_banco(banco);
    return 0;
}