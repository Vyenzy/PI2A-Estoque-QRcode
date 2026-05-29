#include "libs/raylib.h"
#include "libs/sqlite3.h"
#include <stdio.h>
#include <string.h>

// 1. Estrutura para guardar os dados na Memória RAM
#define MAX_PRODUTOS 50
typedef struct {
    char codigo[20];
    char nome[50];
    int quantidade;
    char status[20];
} Produto;

// 2. Função que vai até a pasta 'dados' e lê o banco
int carregarProdutos(Produto *lista) {
    sqlite3 *db;
    // Conecta no arquivo do banco
    if (sqlite3_open("dados/banco_estoque.db", &db) != SQLITE_OK) return 0;

    // A mesma regra de negócio do backend
    const char *sql = "SELECT codigo, nome, qtd_atual, qtd_minima FROM produtos;";
    sqlite3_stmt *stmt;
    
    int count = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_PRODUTOS) {
            strcpy(lista[count].codigo, (const char*)sqlite3_column_text(stmt, 0));
            strcpy(lista[count].nome, (const char*)sqlite3_column_text(stmt, 1));
            
            int qtd_atual = sqlite3_column_int(stmt, 2);
            int qtd_minima = sqlite3_column_int(stmt, 3);
            lista[count].quantidade = qtd_atual;
            
            // Regra visual de status
            if (qtd_atual == 0) strcpy(lista[count].status, "ESGOTADO");
            else if (qtd_atual < qtd_minima) strcpy(lista[count].status, "ALERTA");
            else strcpy(lista[count].status, "OK");
            
            count++;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

// Definindo as telas
typedef enum { TELA_VISAO_GERAL = 0, TELA_ENTRADA_LOTE, TELA_VITRINE } TelaAtiva;

int main(void) {
    const int screenWidth = 1024;
    const int screenHeight = 768;

    InitWindow(screenWidth, screenHeight, "Dual-System: Gestao de Estoque QR Code");
    SetTargetFPS(60);

    Color corFundo = (Color){ 245, 247, 250, 255 }; 
    Color corSidebar = (Color){ 33, 43, 54, 255 };  
    Color corBotaoAtivo = (Color){ 44, 123, 229, 255 }; 
    Color corTextoMenu = (Color){ 160, 174, 192, 255 }; 

    TelaAtiva telaAtual = TELA_VISAO_GERAL;
    Rectangle btnVisao = { 15, 120, 220, 45 };
    Rectangle btnEntrada = { 15, 180, 220, 45 };
    Rectangle btnVitrine = { 15, 240, 220, 45 };

    // --- CARREGAMENTO DE DADOS (Executa SÓ UMA VEZ) ---
    Produto produtos[MAX_PRODUTOS];
    int totalProdutos = carregarProdutos(produtos);

    // --- LOOP GRÁFICO (60x por segundo) ---
    while (!WindowShouldClose()) {
        
        Vector2 mousePoint = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePoint, btnVisao)) telaAtual = TELA_VISAO_GERAL;
            if (CheckCollisionPointRec(mousePoint, btnEntrada)) telaAtual = TELA_ENTRADA_LOTE;
            if (CheckCollisionPointRec(mousePoint, btnVitrine)) telaAtual = TELA_VITRINE;
        }

        BeginDrawing();
        ClearBackground(corFundo);

        // Barra Lateral
        DrawRectangle(0, 0, 250, screenHeight, corSidebar);
        DrawText("DUAL-SYSTEM", 40, 40, 22, WHITE);
        DrawText("v1.0 - Painel Operacional", 40, 70, 10, corTextoMenu);

        DrawRectangleRec(btnVisao, (telaAtual == TELA_VISAO_GERAL) ? corBotaoAtivo : corSidebar);
        DrawText("Visao Geral", 40, 135, 16, (telaAtual == TELA_VISAO_GERAL) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnEntrada, (telaAtual == TELA_ENTRADA_LOTE) ? corBotaoAtivo : corSidebar);
        DrawText("Entrada de Lote", 40, 195, 16, (telaAtual == TELA_ENTRADA_LOTE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnVitrine, (telaAtual == TELA_VITRINE) ? corBotaoAtivo : corSidebar);
        DrawText("Vitrine de Alertas", 40, 255, 16, (telaAtual == TELA_VITRINE) ? WHITE : corTextoMenu);

        // Cabeçalho
        DrawRectangle(250, 0, screenWidth - 250, 80, WHITE);
        DrawLine(250, 80, screenWidth, 80, LIGHTGRAY);

        if (telaAtual == TELA_VISAO_GERAL) {
            DrawText("Visao Geral do Estoque", 290, 30, 24, DARKGRAY);
            
            DrawRectangle(290, 120, screenWidth - 330, 500, WHITE);
            DrawRectangleLines(290, 120, screenWidth - 330, 500, LIGHTGRAY);
            DrawRectangle(290, 120, screenWidth - 330, 40, (Color){ 237, 242, 249, 255 });
            DrawText("CODIGO", 310, 132, 14, DARKGRAY);
            DrawText("PRODUTO", 450, 132, 14, DARKGRAY);
            DrawText("QUANTIDADE", 680, 132, 14, DARKGRAY);
            DrawText("STATUS", 850, 132, 14, DARKGRAY);
            
            // Renderiza os dados direto da Memória RAM
            for (int i = 0; i < totalProdutos; i++) {
                int yPos = 180 + (i * 45); 
                
                DrawLine(290, yPos - 15, screenWidth - 40, yPos - 15, LIGHTGRAY);
                DrawText(produtos[i].codigo, 310, yPos, 14, GRAY);
                DrawText(produtos[i].nome, 450, yPos, 14, GRAY);
                
                // Converte INT para STRING para imprimir
                char qtdStr[10];
                sprintf(qtdStr, "%d un.", produtos[i].quantidade);
                DrawText(qtdStr, 680, yPos, 14, GRAY);
                
                // Define a cor baseada no status
                Color corStatus = GRAY;
                if (strcmp(produtos[i].status, "ESGOTADO") == 0) corStatus = RED;
                else if (strcmp(produtos[i].status, "ALERTA") == 0) corStatus = ORANGE;
                else if (strcmp(produtos[i].status, "OK") == 0) corStatus = LIME;
                
                DrawText(produtos[i].status, 850, yPos, 14, corStatus);
            }
        } 
        else if (telaAtual == TELA_ENTRADA_LOTE) {
            DrawText("Registrar Novo Lote", 290, 30, 24, DARKGRAY);
            DrawText("Modulo em desenvolvimento para a etapa 2...", 290, 150, 16, GRAY);
        } 
        else if (telaAtual == TELA_VITRINE) {
            DrawText("Vitrine Promocional (Alertas)", 290, 30, 24, DARKGRAY);
            DrawText("Modulo de vitrine de descontos em desenvolvimento...", 290, 150, 16, GRAY);
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}