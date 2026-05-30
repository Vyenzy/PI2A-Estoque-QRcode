#include "libs/raylib.h"
#include "estoque_db.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef enum { TELA_VISAO_GERAL = 0, TELA_ENTRADA_LOTE, TELA_VITRINE, TELA_CLIENTE, TELA_RELATORIO } TelaAtiva;

int main(void) {
    const int screenWidth = 1024;
    const int screenHeight = 768;

    InitWindow(screenWidth, screenHeight, "Dual-System: Gestao de Estoque QR Code");
    SetTargetFPS(60);
    srand(time(NULL)); 

    Color corFundo = (Color){ 245, 247, 250, 255 }; 
    Color corSidebar = (Color){ 33, 43, 54, 255 };  
    Color corBotaoAtivo = (Color){ 44, 123, 229, 255 }; 
    Color corTextoMenu = (Color){ 160, 174, 192, 255 }; 
    Color corDestaquePromo = (Color){ 220, 38, 38, 255 };
    Color corDescarte = (Color){ 128, 0, 32, 255 }; 
    Color corEntrada = (Color){ 46, 204, 113, 255 }; // Verde Sucesso

    TelaAtiva telaAtual = TELA_VISAO_GERAL;
    Rectangle btnVisao = { 15, 120, 220, 45 };
    Rectangle btnEntrada = { 15, 180, 220, 45 };
    Rectangle btnVitrine = { 15, 240, 220, 45 };
    Rectangle btnCliente = { 15, 300, 220, 45 }; 
    Rectangle btnRelatorio = { 15, 360, 220, 45 }; 
    Rectangle btnReset = { 15, 680, 220, 35 }; 

    Rectangle btnQR = { 290, 360, 270, 50 }; 
    Rectangle btnSimularVenda = { 290, 120, 270, 40 }; 
    bool qrLido = false;
    double tempoUltimoClique = 0; 

    int paginaAtual = 0;
    int itensPorPagina = 9; 
    Rectangle btnAnt = { 290, 640, 110, 35 };
    Rectangle btnProx = { 420, 640, 110, 35 };

    char ultimoLoteCodigo[20] = "";
    char ultimoLoteNome[50] = "";
    int ultimoLoteQtd = 0;
    int ultimoLoteDiasVencimento = 0;

    const char* catalogoNomes[] = {
        "Macarrao 500g", "Oleo de Soja 900ml", "Cafe Torrado 500g", "Leite Integral 1L", 
        "Farinha de Trigo", "Molho de Tomate", "Sabao em Po 1kg", "Detergente de Maca", 
        "Refrigerante Cola 2L", "Pao de Forma", "Manteiga 200g", "Queijo Mussarela 200g", 
        "Iogurte Morango 1L", "Biscoito Recheado", "Suco de Uva 1L", "Papel Higienico 4un", 
        "Desodorante Aerosol", "Creme Dental", "Arroz Parboilizado 5kg", "Feijao Preto 1kg"
    };

    Produto produtos[MAX_PRODUTOS];
    ResumoDia resumo[MAX_PRODUTOS]; // LISTA CONSOLIDADA ATUALIZADA
    
    int totalProdutos = carregarProdutos(produtos);
    int totalResumo = carregarResumoDia(resumo); // CARREGA O BALANCETE

    while (!WindowShouldClose()) {
        
        Vector2 mousePoint = GetMousePosition();
        int maxPaginas = (totalProdutos > 0) ? (totalProdutos - 1) / itensPorPagina : 0;
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            
            if (CheckCollisionPointRec(mousePoint, btnVisao)) { telaAtual = TELA_VISAO_GERAL; paginaAtual = 0; }
            if (CheckCollisionPointRec(mousePoint, btnEntrada)) telaAtual = TELA_ENTRADA_LOTE;
            if (CheckCollisionPointRec(mousePoint, btnVitrine)) telaAtual = TELA_VITRINE;
            if (CheckCollisionPointRec(mousePoint, btnCliente)) telaAtual = TELA_CLIENTE; 
            if (CheckCollisionPointRec(mousePoint, btnRelatorio)) telaAtual = TELA_RELATORIO; 

            if (CheckCollisionPointRec(mousePoint, btnReset)) {
                limparBancoDemo();
                totalProdutos = carregarProdutos(produtos); 
                totalResumo = carregarResumoDia(resumo); // ATUALIZA AQUI
                paginaAtual = 0;
                qrLido = false; 
            }

            if (telaAtual == TELA_VISAO_GERAL) {
                if (CheckCollisionPointRec(mousePoint, btnAnt) && paginaAtual > 0) paginaAtual--;
                if (CheckCollisionPointRec(mousePoint, btnProx) && paginaAtual < maxPaginas) paginaAtual++;
            }

            if (telaAtual == TELA_ENTRADA_LOTE && CheckCollisionPointRec(mousePoint, btnQR)) {
                if (GetTime() - tempoUltimoClique > 0.5) {
                    
                    int indexCat = rand() % 20;
                    char codigoBase[20];
                    sprintf(codigoBase, "PRD-%03d", indexCat + 1); 
                    
                    sprintf(ultimoLoteCodigo, "LT-%04d", rand() % 9999);
                    strcpy(ultimoLoteNome, catalogoNomes[indexCat]); 
                    
                    if (rand() % 100 < 30) {
                        ultimoLoteQtd = (rand() % 15) + 5;
                        ultimoLoteDiasVencimento = rand() % 15; 
                    } else {
                        ultimoLoteQtd = (rand() % 80) + 30;
                        ultimoLoteDiasVencimento = (rand() % 30) + 15; 
                    }
                    
                    simularEntradaQR(ultimoLoteNome, codigoBase, ultimoLoteCodigo, ultimoLoteQtd, ultimoLoteDiasVencimento);
                    totalProdutos = carregarProdutos(produtos); 
                    totalResumo = carregarResumoDia(resumo); // ATUALIZA AQUI
                    
                    qrLido = true;
                    tempoUltimoClique = GetTime(); 
                }
            }

            if (telaAtual == TELA_RELATORIO && CheckCollisionPointRec(mousePoint, btnSimularVenda)) {
                if (GetTime() - tempoUltimoClique > 0.5) {
                    simularVendasDoDia();
                    totalProdutos = carregarProdutos(produtos); 
                    totalResumo = carregarResumoDia(resumo); // ATUALIZA AQUI
                    tempoUltimoClique = GetTime();
                }
            }
        }

        BeginDrawing();
        ClearBackground(corFundo);

        // Sidebar
        DrawRectangle(0, 0, 250, screenHeight, corSidebar);
        DrawText("DUAL-SYSTEM", 40, 40, 22, WHITE);
        DrawText("v1.0 - Painel Operacional", 40, 70, 10, corTextoMenu);

        DrawRectangleRec(btnVisao, (telaAtual == TELA_VISAO_GERAL) ? corBotaoAtivo : corSidebar);
        DrawText("Visao Geral", 40, 135, 16, (telaAtual == TELA_VISAO_GERAL) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnEntrada, (telaAtual == TELA_ENTRADA_LOTE) ? corBotaoAtivo : corSidebar);
        DrawText("Entrada de Lote", 40, 195, 16, (telaAtual == TELA_ENTRADA_LOTE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnVitrine, (telaAtual == TELA_VITRINE) ? corBotaoAtivo : corSidebar);
        DrawText("Vitrine de Alertas", 40, 255, 16, (telaAtual == TELA_VITRINE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnCliente, (telaAtual == TELA_CLIENTE) ? corBotaoAtivo : corSidebar);
        DrawText("App do Cliente", 40, 315, 16, (telaAtual == TELA_CLIENTE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnRelatorio, (telaAtual == TELA_RELATORIO) ? corBotaoAtivo : corSidebar);
        DrawText("Resumo do Dia", 40, 375, 14, (telaAtual == TELA_RELATORIO) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnReset, (Color){ 192, 57, 43, 255 }); 
        DrawText("RESETAR DADOS (DEMO)", 38, 690, 14, WHITE);

        // Header
        DrawRectangle(250, 0, screenWidth - 250, 80, WHITE);
        DrawLine(250, 80, screenWidth, 80, LIGHTGRAY);

        // --- VISÃO GERAL ---
        if (telaAtual == TELA_VISAO_GERAL) {
            DrawText("Visao Geral do Estoque", 290, 30, 24, DARKGRAY);
            
            DrawRectangle(290, 120, screenWidth - 330, 500, WHITE);
            DrawRectangleLines(290, 120, screenWidth - 330, 500, LIGHTGRAY);
            DrawRectangle(290, 120, screenWidth - 330, 40, (Color){ 237, 242, 249, 255 });
            
            DrawText("CODIGO", 310, 132, 14, DARKGRAY);
            DrawText("PRODUTO", 410, 132, 14, DARKGRAY);
            DrawText("LOTE", 560, 132, 14, DARKGRAY);
            DrawText("QTD", 670, 132, 14, DARKGRAY);
            DrawText("VALIDADE", 760, 132, 14, DARKGRAY); 
            DrawText("STATUS", 860, 132, 14, DARKGRAY);

            int indexInicio = paginaAtual * itensPorPagina;
            int indexFim = indexInicio + itensPorPagina;
            if (indexFim > totalProdutos) indexFim = totalProdutos;

            for (int i = indexInicio; i < indexFim; i++) {
                int indexNaTela = i - indexInicio; 
                int yPos = 180 + (indexNaTela * 45); 

                DrawLine(290, yPos - 15, screenWidth - 40, yPos - 15, LIGHTGRAY);
                DrawText(produtos[i].codigo, 310, yPos, 14, GRAY);
                DrawText(produtos[i].nome, 410, yPos, 14, GRAY);
                DrawText(produtos[i].codigo_lote, 560, yPos, 14, GRAY);
                
                char qtdStr[10];
                sprintf(qtdStr, "%d un.", produtos[i].quantidade);
                DrawText(qtdStr, 670, yPos, 14, GRAY);
                
                char validadeStr[30];
                if (produtos[i].dias_para_vencer < 999) sprintf(validadeStr, "%d dias", produtos[i].dias_para_vencer);
                else strcpy(validadeStr, "-");
                
                Color corData = GRAY;
                if (produtos[i].dias_para_vencer <= 0) corData = corDescarte;
                else if (produtos[i].dias_para_vencer <= 14) corData = RED;
                DrawText(validadeStr, 760, yPos, 14, corData);
                
                Color corStatus = GRAY;
                if (strcmp(produtos[i].status, "VENCIDO") == 0) corStatus = corDescarte;
                else if (strcmp(produtos[i].status, "ESGOTADO") == 0) corStatus = RED;
                else if (strcmp(produtos[i].status, "ALERTA") == 0) corStatus = ORANGE;
                else if (strcmp(produtos[i].status, "OK") == 0) corStatus = LIME;
                
                DrawText(produtos[i].status, 860, yPos, 14, corStatus);
            }

            if (paginaAtual > 0) {
                DrawRectangleRec(btnAnt, corSidebar);
                DrawText("<< Anterior", 305, 650, 14, WHITE);
            }
            if (paginaAtual < maxPaginas) {
                DrawRectangleRec(btnProx, corSidebar);
                DrawText("Proxima >>", 440, 650, 14, WHITE);
            }
            char pagText[50];
            sprintf(pagText, "Pagina %d de %d (Total: %d lotes)", paginaAtual + 1, maxPaginas + 1, totalProdutos);
            DrawText(pagText, 700, 650, 14, GRAY);
        } 
        
        // --- ENTRADA DE LOTE ---
        else if (telaAtual == TELA_ENTRADA_LOTE) {
            DrawText("Entrada de Mercadoria (Leitura Automatica)", 290, 30, 24, DARKGRAY);
            
            DrawRectangleLines(290, 120, 250, 200, LIGHTGRAY);
            DrawText("[ AREA DE SCAN QR ]", 330, 210, 16, LIGHTGRAY);
            
            DrawRectangleRec(btnQR, qrLido ? corEntrada : corBotaoAtivo);
            DrawText(qrLido ? "LER PROXIMO LOTE" : "SIMULAR LEITURA QR", 335, 376, 16, WHITE);

            if (qrLido) {
                DrawText("SUCESSO! Gravado no Banco de Dados:", 580, 120, 16, DARKGRAY);
                DrawRectangle(580, 150, 400, 170, WHITE);
                DrawRectangleLines(580, 150, 400, 170, LIGHTGRAY);
                
                char txtNome[100], txtCodigo[50], txtQtd[50], txtValidade[50];
                sprintf(txtNome, "Produto: %s", ultimoLoteNome);
                sprintf(txtCodigo, "Codigo do Lote: %s", ultimoLoteCodigo);
                sprintf(txtQtd, "Quantidade Lida: %d un.", ultimoLoteQtd);
                
                if (ultimoLoteDiasVencimento <= 0) sprintf(txtValidade, "Validade: VENCIDO HOJE!");
                else sprintf(txtValidade, "Validade: Em %d dias", ultimoLoteDiasVencimento);

                DrawText(txtNome, 600, 170, 14, GRAY);
                DrawText(txtCodigo, 600, 195, 14, GRAY);
                DrawText(txtQtd, 600, 220, 14, GRAY);
                
                Color corTextoValidade = LIME;
                if (ultimoLoteDiasVencimento <= 0) corTextoValidade = corDescarte;
                else if (ultimoLoteDiasVencimento <= 14) corTextoValidade = RED;

                DrawText(txtValidade, 600, 255, 14, corTextoValidade);
            }
        } 
        
        // --- VITRINE DE ALERTAS ---
        else if (telaAtual == TELA_VITRINE) {
            DrawText("Painel do Gerente: Vitrine de Alertas", 290, 30, 24, DARKGRAY);
            
            DrawRectangle(290, 120, screenWidth - 330, 500, (Color){ 255, 245, 245, 255 }); 
            DrawRectangleLines(290, 120, screenWidth - 330, 500, RED);
            
            DrawRectangle(290, 120, screenWidth - 330, 40, (Color){ 255, 220, 220, 255 });
            DrawText("CODIGO", 310, 132, 14, DARKGRAY);
            DrawText("PRODUTO EM RISCO", 450, 132, 14, DARKGRAY);
            DrawText("QTD", 670, 132, 14, DARKGRAY);
            DrawText("VALIDADE", 740, 132, 14, DARKGRAY); 
            DrawText("ACAO", 850, 132, 14, DARKGRAY);
            
            int vitrineCount = 0;
            const char* prioridades[] = {"VENCIDO", "ESGOTADO", "ALERTA"};
            
            for (int p = 0; p < 3; p++) {
                int faixas_alerta[] = {2, 6, 10, 14, 999};
                int faixas_alerta_ant[] = {0, 2, 6, 10, 14};

                for (int f = 0; f < 5; f++) {
                    for (int i = 0; i < totalProdutos; i++) {
                        if (strcmp(produtos[i].status, prioridades[p]) == 0) {
                            
                            if (p == 2 && !(produtos[i].dias_para_vencer > faixas_alerta_ant[f] && produtos[i].dias_para_vencer <= faixas_alerta[f])) continue; 

                            int yPos = 180 + (vitrineCount * 45); 
                            if (yPos > 600) break; 
                            
                            DrawLine(290, yPos - 15, screenWidth - 40, yPos - 15, RED);
                            DrawText(produtos[i].codigo, 310, yPos, 14, DARKGRAY);
                            DrawText(produtos[i].nome, 450, yPos, 14, DARKGRAY);
                            
                            char qtdStr[10];
                            sprintf(qtdStr, "%d", produtos[i].quantidade);
                            DrawText(qtdStr, 670, yPos, 14, RED);
                            
                            char validadeStr[30];
                            if (produtos[i].dias_para_vencer < 999) sprintf(validadeStr, "%d dias", produtos[i].dias_para_vencer);
                            else strcpy(validadeStr, "-");
                            
                            if (p == 0) {
                                DrawText(validadeStr, 740, yPos, 14, corDescarte);
                                DrawText("DESCARTAR", 850, yPos, 14, corDescarte);
                            } else if (p == 1) {
                                DrawText(validadeStr, 740, yPos, 14, GRAY);
                                DrawText("REPOR", 850, yPos, 14, RED);
                            } else if (p == 2) {
                                DrawText(validadeStr, 740, yPos, 14, RED);
                                DrawText("APP / PROMO", 850, yPos, 14, ORANGE);
                            }
                            vitrineCount++;
                        }
                    }
                    if (p != 2) break; 
                }
            }
        }
        
        // --- APP DO CLIENTE ---
        else if (telaAtual == TELA_CLIENTE) {
            DrawText("App do Consumidor - Dual-System Supermercados", 290, 30, 24, corDestaquePromo);
            DrawText("Aproveite as promocoes de queima de estoque!", 290, 65, 14, GRAY);
            
            int qtCards = 0;
            int limites[] = {2, 6, 10, 14};
            int prev_limite = 0;

            for (int f = 0; f < 4; f++) {
                for (int i = 0; i < totalProdutos; i++) {
                    int dias = produtos[i].dias_para_vencer;
                    if (strcmp(produtos[i].status, "ALERTA") == 0 && dias > prev_limite && dias <= limites[f]) {
                        
                        int col = qtCards % 3; 
                        int row = qtCards / 3; 
                        if (row >= 3) break; 

                        int xCard = 290 + (col * 240); 
                        int yCard = 120 + (row * 160); 

                        int porcentagemDesconto;
                        if (f == 0) porcentagemDesconto = 80;
                        else if (f == 1) porcentagemDesconto = 60;
                        else if (f == 2) porcentagemDesconto = 40;
                        else porcentagemDesconto = 20;

                        DrawRectangle(xCard, yCard, 220, 140, WHITE);
                        DrawRectangleLines(xCard, yCard, 220, 140, LIGHTGRAY);
                        DrawRectangle(xCard, yCard, 220, 30, corDestaquePromo);
                        DrawText("OFERTA DO DIA", xCard + 50, yCard + 8, 14, WHITE);
                        
                        DrawText(produtos[i].nome, xCard + 15, yCard + 45, 14, DARKGRAY);

                        char txtDesconto[20];
                        sprintf(txtDesconto, "%d%% OFF!", porcentagemDesconto);
                        DrawText(txtDesconto, xCard + 15, yCard + 70, 26, RED);
                        
                        DrawText("Vencimento proximo!", xCard + 15, yCard + 105, 12, GRAY);
                        DrawText("APROVEITE AGORA!", xCard + 15, yCard + 120, 12, DARKGRAY);

                        qtCards++;
                    }
                }
                prev_limite = limites[f]; 
            }
            if (qtCards == 0) DrawText("Nenhuma oferta no momento. Volte mais tarde!", 290, 150, 16, GRAY);
        }

        // --- RELATÓRIO DO DIA (BALANCETE CONSOLIDADO) ---
        else if (telaAtual == TELA_RELATORIO) {
            DrawText("Fechamento de Caixa: Balancete Consolidado", 290, 30, 24, DARKGRAY);
            
            DrawRectangleRec(btnSimularVenda, corBotaoAtivo);
            DrawText("SIMULAR VENDAS (BAIXAR ESTOQUE)", 300, 132, 14, WHITE);

            DrawRectangle(290, 180, screenWidth - 330, 520, WHITE);
            DrawRectangleLines(290, 180, screenWidth - 330, 520, LIGHTGRAY);
            DrawRectangle(290, 180, screenWidth - 330, 40, (Color){ 237, 242, 249, 255 });
            
            DrawText("PRODUTO", 310, 192, 14, DARKGRAY);
            DrawText("CHEGARAM (+)", 550, 192, 14, DARKGRAY);
            DrawText("VENDIDOS (-)", 700, 192, 14, DARKGRAY);
            DrawText("ESTOQUE TOTAL", 850, 192, 14, DARKGRAY);

            for (int i = 0; i < totalResumo; i++) {
                int yPos = 240 + (i * 40); 
                if (yPos > 680) break;

                DrawLine(290, yPos - 10, screenWidth - 40, yPos - 10, LIGHTGRAY);
                
                DrawText(resumo[i].produto_nome, 310, yPos, 14, DARKGRAY);
                
                char entStr[20];
                sprintf(entStr, "+%d un.", resumo[i].entradas);
                DrawText(entStr, 550, yPos, 14, (resumo[i].entradas > 0) ? corEntrada : LIGHTGRAY);
                
                char saiStr[20];
                sprintf(saiStr, "-%d un.", resumo[i].saidas);
                DrawText(saiStr, 700, yPos, 14, (resumo[i].saidas > 0) ? RED : LIGHTGRAY);
                
                char totalStr[20];
                sprintf(totalStr, "%d un.", resumo[i].saldo_total);
                DrawText(totalStr, 850, yPos, 14, (resumo[i].saldo_total <= 0) ? RED : DARKGRAY);
            }

            if (totalResumo == 0) {
                DrawText("Nenhuma movimentacao ou produto registrado ainda.", 310, 240, 14, GRAY);
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}