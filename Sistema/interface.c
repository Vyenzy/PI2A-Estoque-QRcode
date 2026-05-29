#include "libs/raylib.h"
#include "libs/sqlite3.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PRODUTOS 500
#define DB_PATH "dados/banco_estoque.db"

typedef struct {
    char codigo[20];
    char nome[50];
    int quantidade;
    char status[20];
    int dias_para_vencer; 
} Produto;

// --- GARANTIA DE INFRAESTRUTURA PARA A BANCA ---
void inicializarBanco() {
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) == SQLITE_OK) {
        const char *sql = 
            "CREATE TABLE IF NOT EXISTS produtos ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, codigo TEXT UNIQUE, nome TEXT, categoria TEXT, qtd_minima INTEGER, qtd_atual INTEGER DEFAULT 0);"
            "CREATE TABLE IF NOT EXISTS lotes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, produto_id INTEGER, codigo_lote TEXT UNIQUE, quantidade INTEGER, validade DATE, status TEXT DEFAULT 'ativo');";
        sqlite3_exec(db, sql, NULL, 0, NULL);
        sqlite3_close(db);
    }
}

// --- FUNÇÕES DE BANCO DE DADOS ---
int carregarProdutos(Produto *lista) {
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return 0;

    const char *sql = 
        "SELECT p.codigo, p.nome, p.qtd_atual, p.qtd_minima, "
        "MIN(CAST(julianday(l.validade) - julianday('now') AS INT)) as dias_restantes "
        "FROM produtos p "
        "LEFT JOIN lotes l ON p.id = l.produto_id AND l.status = 'ativo' "
        "GROUP BY p.id ORDER BY p.id DESC;";
    
    sqlite3_stmt *stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW && count < MAX_PRODUTOS) {
            strcpy(lista[count].codigo, (const char*)sqlite3_column_text(stmt, 0));
            strcpy(lista[count].nome, (const char*)sqlite3_column_text(stmt, 1));
            
            int qtd_atual = sqlite3_column_int(stmt, 2);
            int qtd_minima = sqlite3_column_int(stmt, 3);
            lista[count].quantidade = qtd_atual;
            
            if (sqlite3_column_type(stmt, 4) == SQLITE_INTEGER) {
                lista[count].dias_para_vencer = sqlite3_column_int(stmt, 4);
            } else {
                lista[count].dias_para_vencer = 999; 
            }
            
            // NOVA LÓGICA DE STATUS
            if (qtd_atual == 0) strcpy(lista[count].status, "ESGOTADO");
            else if (lista[count].dias_para_vencer <= 0) strcpy(lista[count].status, "VENCIDO"); // 0 dias ou menos = Lixo
            else if (lista[count].dias_para_vencer <= 14 || qtd_atual < qtd_minima) 
                strcpy(lista[count].status, "ALERTA");
            else strcpy(lista[count].status, "OK");
            
            count++;
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return count;
}

void simularEntradaQR(const char *nomeProduto, const char *codigoProduto, const char *codigoLote, int quantidade, int diasValidade) {
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) return;

    int produto_id = 0;
    sqlite3_stmt *stmt;
    char sql[512];

    snprintf(sql, sizeof(sql), "SELECT id FROM produtos WHERE nome = '%s';", nomeProduto);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) produto_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (produto_id == 0) {
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

    snprintf(sql, sizeof(sql), "UPDATE produtos SET qtd_atual = IFNULL((SELECT SUM(quantidade) FROM lotes WHERE produto_id = %d AND status = 'ativo'), 0) WHERE id = %d;", produto_id, produto_id);
    sqlite3_exec(db, sql, NULL, 0, NULL);

    sqlite3_close(db);
}

void limparBancoDemo() {
    sqlite3 *db;
    if (sqlite3_open(DB_PATH, &db) == SQLITE_OK) {
        sqlite3_exec(db, "DELETE FROM lotes WHERE codigo_lote LIKE 'LT-%';", NULL, 0, NULL);
        sqlite3_exec(db, "DELETE FROM produtos WHERE codigo LIKE 'PRD-%';", NULL, 0, NULL);
        sqlite3_exec(db, "UPDATE produtos SET qtd_atual = IFNULL((SELECT SUM(quantidade) FROM lotes WHERE lotes.produto_id = produtos.id AND status = 'ativo'), 0);", NULL, 0, NULL);
        sqlite3_close(db);
    }
}

typedef enum { TELA_VISAO_GERAL = 0, TELA_ENTRADA_LOTE, TELA_VITRINE, TELA_CLIENTE } TelaAtiva;

int main(void) {
    const int screenWidth = 1024;
    const int screenHeight = 768;

    InitWindow(screenWidth, screenHeight, "Dual-System: Gestao de Estoque QR Code");
    SetTargetFPS(60);
    srand(time(NULL)); 
    
    inicializarBanco(); 

    Color corFundo = (Color){ 245, 247, 250, 255 }; 
    Color corSidebar = (Color){ 33, 43, 54, 255 };  
    Color corBotaoAtivo = (Color){ 44, 123, 229, 255 }; 
    Color corTextoMenu = (Color){ 160, 174, 192, 255 }; 
    Color corDestaquePromo = (Color){ 220, 38, 38, 255 };
    Color corDescarte = (Color){ 128, 0, 32, 255 }; // Cor Vinho para Vencidos (Maroon)

    TelaAtiva telaAtual = TELA_VISAO_GERAL;
    Rectangle btnVisao = { 15, 120, 220, 45 };
    Rectangle btnEntrada = { 15, 180, 220, 45 };
    Rectangle btnVitrine = { 15, 240, 220, 45 };
    Rectangle btnCliente = { 15, 300, 220, 45 }; 
    Rectangle btnReset = { 15, 680, 220, 35 }; 

    Rectangle btnQR = { 290, 360, 270, 50 }; 
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
    int totalProdutos = carregarProdutos(produtos);

    while (!WindowShouldClose()) {
        
        Vector2 mousePoint = GetMousePosition();
        int maxPaginas = (totalProdutos > 0) ? (totalProdutos - 1) / itensPorPagina : 0;
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            
            if (CheckCollisionPointRec(mousePoint, btnVisao)) { telaAtual = TELA_VISAO_GERAL; paginaAtual = 0; }
            if (CheckCollisionPointRec(mousePoint, btnEntrada)) telaAtual = TELA_ENTRADA_LOTE;
            if (CheckCollisionPointRec(mousePoint, btnVitrine)) telaAtual = TELA_VITRINE;
            if (CheckCollisionPointRec(mousePoint, btnCliente)) telaAtual = TELA_CLIENTE; 

            if (CheckCollisionPointRec(mousePoint, btnReset)) {
                limparBancoDemo();
                totalProdutos = carregarProdutos(produtos); 
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
                    
                    // Ajuste: Chance de cair em VENCIDO (0 dias) na hora da demo
                    if (rand() % 100 < 30) {
                        ultimoLoteQtd = (rand() % 15) + 5;
                        ultimoLoteDiasVencimento = rand() % 15; // De 0 a 14 dias (Pode dar 0!)
                    } else {
                        ultimoLoteQtd = (rand() % 80) + 30;
                        ultimoLoteDiasVencimento = (rand() % 30) + 15; 
                    }
                    
                    simularEntradaQR(ultimoLoteNome, codigoBase, ultimoLoteCodigo, ultimoLoteQtd, ultimoLoteDiasVencimento);
                    totalProdutos = carregarProdutos(produtos); 
                    
                    qrLido = true;
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
        DrawText("App do Cliente (Promocoes)", 40, 315, 14, (telaAtual == TELA_CLIENTE) ? WHITE : corTextoMenu);

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
            DrawText("PRODUTO", 450, 132, 14, DARKGRAY);
            DrawText("QTD", 680, 132, 14, DARKGRAY);
            DrawText("VALIDADE", 780, 132, 14, DARKGRAY); 
            DrawText("STATUS", 880, 132, 14, DARKGRAY);

            int indexInicio = paginaAtual * itensPorPagina;
            int indexFim = indexInicio + itensPorPagina;
            if (indexFim > totalProdutos) indexFim = totalProdutos;

            for (int i = indexInicio; i < indexFim; i++) {
                int indexNaTela = i - indexInicio; 
                int yPos = 180 + (indexNaTela * 45); 

                DrawLine(290, yPos - 15, screenWidth - 40, yPos - 15, LIGHTGRAY);
                DrawText(produtos[i].codigo, 310, yPos, 14, GRAY);
                DrawText(produtos[i].nome, 450, yPos, 14, GRAY);
                
                char qtdStr[10];
                sprintf(qtdStr, "%d un.", produtos[i].quantidade);
                DrawText(qtdStr, 680, yPos, 14, GRAY);
                
                char validadeStr[30];
                if (produtos[i].dias_para_vencer < 999) {
                    sprintf(validadeStr, "%d dias", produtos[i].dias_para_vencer);
                } else {
                    strcpy(validadeStr, "-");
                }
                
                // Colorir a validade se for risco
                Color corData = GRAY;
                if (produtos[i].dias_para_vencer <= 0) corData = corDescarte;
                else if (produtos[i].dias_para_vencer <= 14) corData = RED;
                DrawText(validadeStr, 780, yPos, 14, corData);
                
                Color corStatus = GRAY;
                if (strcmp(produtos[i].status, "VENCIDO") == 0) corStatus = corDescarte;
                else if (strcmp(produtos[i].status, "ESGOTADO") == 0) corStatus = RED;
                else if (strcmp(produtos[i].status, "ALERTA") == 0) corStatus = ORANGE;
                else if (strcmp(produtos[i].status, "OK") == 0) corStatus = LIME;
                
                DrawText(produtos[i].status, 880, yPos, 14, corStatus);
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
            sprintf(pagText, "Pagina %d de %d (Total: %d itens)", paginaAtual + 1, maxPaginas + 1, totalProdutos);
            DrawText(pagText, 700, 650, 14, GRAY);
        } 
        
        // --- ENTRADA DE LOTE ---
        else if (telaAtual == TELA_ENTRADA_LOTE) {
            DrawText("Entrada de Mercadoria (Leitura Automatica)", 290, 30, 24, DARKGRAY);
            
            DrawRectangleLines(290, 120, 250, 200, LIGHTGRAY);
            DrawText("[ AREA DE SCAN QR ]", 330, 210, 16, LIGHTGRAY);
            
            DrawRectangleRec(btnQR, qrLido ? (Color){46, 204, 113, 255} : corBotaoAtivo);
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

                if (ultimoLoteDiasVencimento <= 0) {
                    DrawText("[!] ALERTA CRITICO:", 600, 280, 12, corDescarte);
                    DrawText("Mercadoria imprópria. Separar para descarte.", 600, 295, 12, GRAY);
                }
                else if (ultimoLoteDiasVencimento <= 14 || ultimoLoteQtd < 25) {
                    DrawText("[!] ALERTA DUAL-SYSTEM:", 600, 280, 12, ORANGE);
                    DrawText("Regra de negocio acionada. Enviado a Vitrine.", 600, 295, 12, GRAY);
                }
            }
        } 
        
        // --- VITRINE DE ALERTAS (ORDEM DE PRIORIDADE MULTI-PASS) ---
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

            // STATUS PARA BUSCAR EM ORDEM DE PRIORIDADE
            const char* prioridades[] = {"VENCIDO", "ESGOTADO", "ALERTA"};
            
            // Lógica de 3 Passos para desenhar na ordem exata de urgência
            for (int p = 0; p < 3; p++) {
                
                // Dentro do "ALERTA", vamos buscar primeiro os que vencem mais rápido
                int faixas_alerta[] = {2, 6, 10, 14, 999};
                int faixas_alerta_ant[] = {0, 2, 6, 10, 14};

                for (int f = 0; f < 5; f++) {
                    for (int i = 0; i < totalProdutos; i++) {
                        
                        // Verifica se o produto se encaixa no passo atual (p)
                        if (strcmp(produtos[i].status, prioridades[p]) == 0) {
                            
                            // Se for ALERTA, aplica o filtro extra de dias (f)
                            if (p == 2 && !(produtos[i].dias_para_vencer > faixas_alerta_ant[f] && produtos[i].dias_para_vencer <= faixas_alerta[f])) {
                                continue; 
                            }

                            int yPos = 180 + (vitrineCount * 45); 
                            if (yPos > 600) break; 
                            
                            DrawLine(290, yPos - 15, screenWidth - 40, yPos - 15, RED);
                            DrawText(produtos[i].codigo, 310, yPos, 14, DARKGRAY);
                            DrawText(produtos[i].nome, 450, yPos, 14, DARKGRAY);
                            
                            char qtdStr[10];
                            sprintf(qtdStr, "%d", produtos[i].quantidade);
                            DrawText(qtdStr, 670, yPos, 14, RED);
                            
                            char validadeStr[30];
                            if (produtos[i].dias_para_vencer < 999) {
                                sprintf(validadeStr, "%d dias", produtos[i].dias_para_vencer);
                            } else {
                                strcpy(validadeStr, "-");
                            }
                            
                            // Cores e Textos Baseados na Prioridade
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
                    if (p != 2) break; // Só faz o loop de 'f' (faixas) se for ALERTA
                }
            }
        }
        
        // --- APP DO CLIENTE (OFERTAS ORDENADAS POR DESCONTO) ---
        else if (telaAtual == TELA_CLIENTE) {
            DrawText("App do Consumidor - Dual-System Supermercados", 290, 30, 24, corDestaquePromo);
            DrawText("Aproveite as promocoes de queima de estoque!", 290, 65, 14, GRAY);
            
            int qtCards = 0;
            
            // FAIXAS DE DESCONTO: Busca primeiro quem tem <=2 dias, depois <=6, etc.
            int limites[] = {2, 6, 10, 14};
            int prev_limite = 0;

            for (int f = 0; f < 4; f++) {
                for (int i = 0; i < totalProdutos; i++) {
                    
                    int dias = produtos[i].dias_para_vencer;

                    // O Item SÓ VAI PRO APP se for ALERTA e estiver na faixa atual do loop
                    if (strcmp(produtos[i].status, "ALERTA") == 0 && dias > prev_limite && dias <= limites[f]) {
                        
                        int col = qtCards % 3; 
                        int row = qtCards / 3; 
                        if (row >= 3) break; 

                        int xCard = 290 + (col * 240); 
                        int yCard = 120 + (row * 160); 

                        // Calcula Porcentagem (Vinculado a Faixa atual 'f')
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
                prev_limite = limites[f]; // Prepara para buscar a próxima faixa de dias
            }
            if (qtCards == 0) {
                DrawText("Nenhuma oferta no momento. Volte mais tarde!", 290, 150, 16, GRAY);
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}