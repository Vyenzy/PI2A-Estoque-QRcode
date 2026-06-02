#include "libs/raylib.h"
#include "estoque_db.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

// --- ENUMS E ESTRUTURAS ---
typedef enum 
{ 
    TELA_VISAO_GERAL = 0, 
    TELA_ENTRADA_LOTE, 
    TELA_VITRINE, 
    TELA_CLIENTE, 
    TELA_RELATORIO 
} TelaAtiva;

typedef struct
{
    char nome[50];
    char codigo_lote[30]; // Necessário para debitar no banco
    int quantidade;
} ItemCarrinho;

// --- FUNÇÃO AUXILIAR PARA FONTES ---
void DrawTextUI(Font fonte, const char *texto, float x, float y, float tamanho, Color cor)
{
    DrawTextEx(fonte, texto, (Vector2){x, y}, tamanho, 1, cor);
}

int main(void)
{
    // --- CONFIGURAÇÕES DA JANELA ---
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 768, "Dual-System: Gestao de Estoque Inteligente");
    SetWindowMinSize(1024, 768);
    SetTargetFPS(60);
    srand(time(NULL)); 

    Font fontePrincipal = LoadFontEx("fonte.ttf", 32, 0, 250);
    if (fontePrincipal.texture.id == 0) 
    {
        fontePrincipal = GetFontDefault();
    }

    // --- PALETA DE CORES ---
    Color corFundo = (Color){ 245, 247, 250, 255 }; 
    Color corSidebar = (Color){ 26, 32, 44, 255 };  
    Color corBotaoAtivo = (Color){ 49, 130, 206, 255 }; 
    Color corTextoMenu = (Color){ 160, 174, 192, 255 }; 
    Color corDestaquePromo = (Color){ 220, 38, 38, 255 };
    Color corDescarte = (Color){ 153, 27, 27, 255 }; 
    Color corEntrada = (Color){ 16, 185, 129, 255 }; 

    // --- VARIÁVEIS DE ESTADO ---
    TelaAtiva telaAtual = TELA_VISAO_GERAL;
    bool qrLido = false;
    double tempoUltimoClique = 0; 
    int paginaAtual = 0;
    int itensPorPagina = 9; 

    // Variáveis do Leitor Físico
    char bufferLeitor[256] = "\0";
    int tamanhoBuffer = 0;
    char ultimoLoteCodigo[30] = "";
    char ultimoLoteNome[50] = "";
    int ultimoLoteQtd = 0;
    int ultimoLoteDiasVencimento = 0;

    // Variáveis do Carrinho e Compra
    ItemCarrinho carrinho[100];
    int totalItensCarrinho = 0;
    bool compraRealizada = false;
    double tempoMsgCompra = 0;

    const char* catalogoNomes[] = 
    {
        "Macarrao 500g", "Oleo de Soja 900ml", "Cafe Torrado 500g", "Leite Integral 1L", 
        "Farinha de Trigo", "Molho de Tomate", "Sabao em Po 1kg", "Detergente de Maca", 
        "Refrigerante Cola 2L", "Pao de Forma", "Manteiga 200g", "Queijo Mussarela 200g", 
        "Iogurte Morango 1L", "Biscoito Recheado", "Suco de Uva 1L", "Papel Higienico 4un", 
        "Desodorante Aerosol", "Creme Dental", "Arroz Parboilizado 5kg", "Feijao Preto 1kg"
    };

    Produto produtos[MAX_PRODUTOS];
    ResumoDia resumo[MAX_PRODUTOS];
    int totalProdutos = carregarProdutos(produtos);
    int totalResumo = carregarResumoDia(resumo); 

    // --- LOOP PRINCIPAL ---
    while (!WindowShouldClose())
    {
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        Vector2 mousePoint = GetMousePosition();
        
        Rectangle btnVisao = { 15, 130, 220, 40 };
        Rectangle btnEntrada = { 15, 180, 220, 40 };
        Rectangle btnVitrine = { 15, 230, 220, 40 };
        Rectangle btnRelatorio = { 15, 280, 220, 40 }; 
        Rectangle btnCliente = { 15, 380, 220, 40 }; 
        Rectangle btnReset = { 15, sh - 60, 220, 40 }; 

        Rectangle btnQR = { 290, 360, 270, 45 }; 
        Rectangle btnSimularVenda = { 290, 120, 270, 40 }; 
        Rectangle btnAnt = { 290, sh - 60, 110, 35 };
        Rectangle btnProx = { 420, sh - 60, 110, 35 };

        int maxPaginas = 0;
        if (telaAtual == TELA_VISAO_GERAL) 
        {
            maxPaginas = (totalProdutos > 0) ? (totalProdutos - 1) / itensPorPagina : 0;
        }
        else if (telaAtual == TELA_RELATORIO) 
        {
            maxPaginas = (totalResumo > 0) ? (totalResumo - 1) / 10 : 0;
        }
        
        // --- EVENTOS E CLICKS ---
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mousePoint, btnVisao)) { telaAtual = TELA_VISAO_GERAL; paginaAtual = 0; }
            if (CheckCollisionPointRec(mousePoint, btnEntrada)) { telaAtual = TELA_ENTRADA_LOTE; paginaAtual = 0; }
            if (CheckCollisionPointRec(mousePoint, btnVitrine)) { telaAtual = TELA_VITRINE; paginaAtual = 0; }
            if (CheckCollisionPointRec(mousePoint, btnRelatorio)) { telaAtual = TELA_RELATORIO; paginaAtual = 0; }
            
            if (CheckCollisionPointRec(mousePoint, btnCliente) && telaAtual != TELA_CLIENTE) 
            { 
                telaAtual = TELA_CLIENTE; 
                paginaAtual = 0; 
                // REMOVIDO o reset do carrinho aqui! O carrinho persiste!
            }

            if (CheckCollisionPointRec(mousePoint, btnReset))
            {
                limparBancoDemo();
                totalProdutos = carregarProdutos(produtos); 
                totalResumo = carregarResumoDia(resumo); 
                paginaAtual = 0;
                totalItensCarrinho = 0;
                qrLido = false; 
            }

            if (telaAtual == TELA_VISAO_GERAL || telaAtual == TELA_RELATORIO)
            {
                if (CheckCollisionPointRec(mousePoint, btnAnt) && paginaAtual > 0) paginaAtual--;
                if (CheckCollisionPointRec(mousePoint, btnProx) && paginaAtual < maxPaginas) paginaAtual++;
            }

            if (telaAtual == TELA_RELATORIO && CheckCollisionPointRec(mousePoint, btnSimularVenda))
            {
                if (GetTime() - tempoUltimoClique > 0.2)
                {
                    simularVendasDoDia();
                    totalProdutos = carregarProdutos(produtos); 
                    totalResumo = carregarResumoDia(resumo); 
                    tempoUltimoClique = GetTime();
                }
            }
        }

        // --- LÓGICA DO LEITOR FÍSICO ---
        if (telaAtual == TELA_ENTRADA_LOTE)
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if ((key >= 32) && (key <= 125) && (tamanhoBuffer < 255))
                {
                    bufferLeitor[tamanhoBuffer] = (char)key;
                    bufferLeitor[tamanhoBuffer + 1] = '\0'; 
                    tamanhoBuffer++;
                }
                key = GetCharPressed(); 
            }

            if (IsKeyPressed(KEY_ENTER) && tamanhoBuffer > 0)
            {
                char nomeLido[50], codProdLido[20], codLoteLido[30];
                int qtdLida = 0, diasLidos = 0;
                
                if (sscanf(bufferLeitor, "%[^,],%[^,],%[^,],%d,%d", nomeLido, codProdLido, codLoteLido, &qtdLida, &diasLidos) == 5)
                {
                    simularEntradaQR(nomeLido, codProdLido, codLoteLido, qtdLida, diasLidos);
                    totalProdutos = carregarProdutos(produtos); 
                    totalResumo = carregarResumoDia(resumo);
                    
                    strcpy(ultimoLoteNome, nomeLido);
                    strcpy(ultimoLoteCodigo, codLoteLido);
                    ultimoLoteQtd = qtdLida;
                    ultimoLoteDiasVencimento = diasLidos;
                    qrLido = true;
                }
                tamanhoBuffer = 0;
                bufferLeitor[0] = '\0';
            }

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePoint, btnQR))
            {
                if (GetTime() - tempoUltimoClique > 0.2)
                {
                    int idx = rand() % 20;
                    char codigoBase[20];
                    sprintf(codigoBase, "PRD-%03d", idx + 1); 
                    sprintf(ultimoLoteCodigo, "LT-%06d", rand() % 999999);
                    strcpy(ultimoLoteNome, catalogoNomes[idx]); 
                    
                    if (rand() % 100 < 30)
                    {
                        ultimoLoteQtd = (rand() % 15) + 5;
                        ultimoLoteDiasVencimento = rand() % 15; 
                    }
                    else
                    {
                        ultimoLoteQtd = (rand() % 80) + 30;
                        ultimoLoteDiasVencimento = (rand() % 30) + 15; 
                    }
                    
                    simularEntradaQR(ultimoLoteNome, codigoBase, ultimoLoteCodigo, ultimoLoteQtd, ultimoLoteDiasVencimento);
                    totalProdutos = carregarProdutos(produtos); 
                    totalResumo = carregarResumoDia(resumo); 
                    qrLido = true;
                    tempoUltimoClique = GetTime(); 
                }
            }
        }

        // --- RENDERIZAÇÃO GRÁFICA ---
        BeginDrawing();
        ClearBackground(corFundo);

        DrawRectangle(250, 0, sw - 250, 80, WHITE);
        DrawLine(250, 80, sw, 80, LIGHTGRAY);

        DrawRectangle(0, 0, 250, sh, corSidebar);
        DrawTextUI(fontePrincipal, "DUAL-SYSTEM", 35, 30, 24, WHITE);
        DrawTextUI(fontePrincipal, "v1.0 - ERP & PDV", 35, 60, 12, corTextoMenu);

        DrawTextUI(fontePrincipal, "GESTAO E OPERACAO", 20, 105, 12, GRAY);
        
        DrawRectangleRec(btnVisao, (telaAtual == TELA_VISAO_GERAL) ? corBotaoAtivo : corSidebar);
        DrawTextUI(fontePrincipal, "Visao Geral", 40, 142, 14, (telaAtual == TELA_VISAO_GERAL) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnEntrada, (telaAtual == TELA_ENTRADA_LOTE) ? corBotaoAtivo : corSidebar);
        DrawTextUI(fontePrincipal, "Entrada de Lote", 40, 192, 14, (telaAtual == TELA_ENTRADA_LOTE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnVitrine, (telaAtual == TELA_VITRINE) ? corBotaoAtivo : corSidebar);
        DrawTextUI(fontePrincipal, "Vitrine de Alertas", 40, 242, 14, (telaAtual == TELA_VITRINE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnRelatorio, (telaAtual == TELA_RELATORIO) ? corBotaoAtivo : corSidebar);
        DrawTextUI(fontePrincipal, "Resumo do Dia", 40, 292, 14, (telaAtual == TELA_RELATORIO) ? WHITE : corTextoMenu);

        DrawLine(20, 345, 230, 345, Fade(LIGHTGRAY, 0.2f));
        DrawTextUI(fontePrincipal, "CONSUMIDOR FINAL", 20, 355, 12, GRAY);

        DrawRectangleRec(btnCliente, (telaAtual == TELA_CLIENTE) ? corBotaoAtivo : corSidebar);
        DrawTextUI(fontePrincipal, "App do Cliente", 40, 392, 14, (telaAtual == TELA_CLIENTE) ? WHITE : corTextoMenu);

        DrawRectangleRec(btnReset, corDescarte); 
        DrawTextUI(fontePrincipal, "RESETAR BANCO", 55, sh - 48, 14, WHITE);

        // Retângulo padrão para listas
        Rectangle bgCard = { 290, 120, sw - 330, sh - 220 }; 

        if (telaAtual == TELA_VISAO_GERAL)
        {
            DrawTextUI(fontePrincipal, "Visao Geral do Estoque", 290, 30, 24, DARKGRAY);
            
            DrawRectangleRec(bgCard, WHITE);
            DrawRectangleLines(bgCard.x, bgCard.y, bgCard.width, bgCard.height, LIGHTGRAY);
            DrawRectangle(290, 120, sw - 330, 40, (Color){ 237, 242, 249, 255 });
            
            DrawTextUI(fontePrincipal, "CODIGO", 310, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "PRODUTO", 410, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "LOTE", 610, 132, 14, DARKGRAY); 
            DrawTextUI(fontePrincipal, "QTD", 700, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "VALIDADE", 780, 132, 14, DARKGRAY); 
            DrawTextUI(fontePrincipal, "STATUS", 880, 132, 14, DARKGRAY);

            int indexFim = (paginaAtual * itensPorPagina) + itensPorPagina;
            if (indexFim > totalProdutos) indexFim = totalProdutos;

            for (int i = paginaAtual * itensPorPagina; i < indexFim; i++)
            {
                int yPos = 180 + ((i % itensPorPagina) * 45); 
                DrawLine(290, yPos - 15, sw - 40, yPos - 15, Fade(LIGHTGRAY, 0.5f));
                
                DrawTextUI(fontePrincipal, produtos[i].codigo, 310, yPos, 14, GRAY);
                DrawTextUI(fontePrincipal, produtos[i].nome, 410, yPos, 14, GRAY);
                DrawTextUI(fontePrincipal, produtos[i].codigo_lote, 610, yPos, 14, GRAY); 
                
                char str[30];
                sprintf(str, "%d un.", produtos[i].quantidade);
                DrawTextUI(fontePrincipal, str, 700, yPos, 14, GRAY);
                
                if (produtos[i].dias_para_vencer < 999) sprintf(str, "%d dias", produtos[i].dias_para_vencer);
                else strcpy(str, "-");
                
                Color cData = (produtos[i].dias_para_vencer <= 0) ? corDescarte : (produtos[i].dias_para_vencer <= 14) ? RED : GRAY;
                DrawTextUI(fontePrincipal, str, 780, yPos, 14, cData);
                
                Color cStatus = LIME;
                if (strcmp(produtos[i].status, "VENCIDO") == 0) cStatus = corDescarte;
                else if (strcmp(produtos[i].status, "ESGOTADO") == 0) cStatus = RED;
                else if (strcmp(produtos[i].status, "ALERTA") == 0) cStatus = ORANGE;
                
                DrawTextUI(fontePrincipal, produtos[i].status, 880, yPos, 14, cStatus);
            }

            if (paginaAtual > 0) { DrawRectangleRec(btnAnt, corSidebar); DrawTextUI(fontePrincipal, "<< Anterior", 305, sh - 50, 14, WHITE); }
            if (paginaAtual < maxPaginas) { DrawRectangleRec(btnProx, corSidebar); DrawTextUI(fontePrincipal, "Proxima >>", 435, sh - 50, 14, WHITE); }
            
            char pagText[50]; 
            sprintf(pagText, "Pagina %d de %d (Total: %d lotes)", paginaAtual + 1, maxPaginas + 1, totalProdutos);
            DrawTextUI(fontePrincipal, pagText, 700, sh - 50, 14, GRAY);
        } 
        
        else if (telaAtual == TELA_ENTRADA_LOTE)
        {
            DrawTextUI(fontePrincipal, "Entrada de Mercadoria", 290, 30, 24, DARKGRAY);
            
            Rectangle scanArea = { 290, 120, 270, 200 };
            DrawRectangleLines(scanArea.x, scanArea.y, scanArea.width, scanArea.height, LIGHTGRAY);
            DrawTextUI(fontePrincipal, "[ AREA DE SCAN QR ]", 330, 210, 14, LIGHTGRAY);
            
            DrawRectangleRec(btnQR, qrLido ? corEntrada : corBotaoAtivo);
            DrawTextUI(fontePrincipal, qrLido ? "LER PROXIMO LOTE" : "SIMULAR LEITURA QR", 335, 372, 14, WHITE);

            if (qrLido)
            {
                DrawTextUI(fontePrincipal, "SUCESSO! Gravado no Banco de Dados:", 580, 120, 16, corEntrada);
                Rectangle cardSucesso = { 580, 150, 400, 180 };
                DrawRectangleRec(cardSucesso, WHITE);
                DrawRectangleLines(cardSucesso.x, cardSucesso.y, cardSucesso.width, cardSucesso.height, LIGHTGRAY);
                
                char str[100];
                sprintf(str, "Produto: %s", ultimoLoteNome); DrawTextUI(fontePrincipal, str, 600, 170, 14, DARKGRAY);
                sprintf(str, "Codigo do Lote: %s", ultimoLoteCodigo); DrawTextUI(fontePrincipal, str, 600, 200, 14, DARKGRAY);
                sprintf(str, "Quantidade Lida: %d un.", ultimoLoteQtd); DrawTextUI(fontePrincipal, str, 600, 230, 14, DARKGRAY);
                
                if (ultimoLoteDiasVencimento <= 0) sprintf(str, "Validade: VENCIDO HOJE!");
                else sprintf(str, "Validade: Em %d dias", ultimoLoteDiasVencimento);
                DrawTextUI(fontePrincipal, str, 600, 270, 14, (ultimoLoteDiasVencimento <= 14) ? RED : LIME);
            }
        } 
        
        else if (telaAtual == TELA_VITRINE)
        {
            DrawTextUI(fontePrincipal, "Painel do Gerente: Vitrine de Alertas", 290, 30, 24, corDescarte);
            
            DrawRectangleRec(bgCard, (Color){ 254, 242, 242, 255 }); 
            DrawRectangleLines(bgCard.x, bgCard.y, bgCard.width, bgCard.height, RED);
            DrawRectangle(290, 120, sw - 330, 40, (Color){ 254, 226, 226, 255 });
            
            DrawTextUI(fontePrincipal, "CODIGO", 310, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "PRODUTO EM RISCO", 450, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "QTD", 670, 132, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "VALIDADE", 750, 132, 14, DARKGRAY); 
            DrawTextUI(fontePrincipal, "ACAO", 860, 132, 14, DARKGRAY);
            
            int vCount = 0;
            const char* prios[] = {"VENCIDO", "ESGOTADO", "ALERTA"};
            for (int p = 0; p < 3; p++)
            {
                int fx[] = {2, 6, 10, 14, 999}, fx_ant[] = {0, 2, 6, 10, 14};
                for (int f = 0; f < 5; f++)
                {
                    for (int i = 0; i < totalProdutos; i++)
                    {
                        if (strcmp(produtos[i].status, prios[p]) == 0)
                        {
                            if (p == 2 && !(produtos[i].dias_para_vencer > fx_ant[f] && produtos[i].dias_para_vencer <= fx[f])) continue; 
                            
                            int yPos = 180 + (vCount * 45); 
                            if (yPos > sh - 100) break; 
                            
                            DrawLine(290, yPos - 15, sw - 40, yPos - 15, Fade(RED, 0.3f));
                            DrawTextUI(fontePrincipal, produtos[i].codigo, 310, yPos, 14, DARKGRAY);
                            DrawTextUI(fontePrincipal, produtos[i].nome, 450, yPos, 14, DARKGRAY);
                            
                            char str[30]; 
                            sprintf(str, "%d", produtos[i].quantidade); 
                            DrawTextUI(fontePrincipal, str, 670, yPos, 14, RED);
                            
                            if (produtos[i].dias_para_vencer < 999) sprintf(str, "%d dias", produtos[i].dias_para_vencer); 
                            else strcpy(str, "-");
                            
                            if (p == 0) { DrawTextUI(fontePrincipal, str, 750, yPos, 14, corDescarte); DrawTextUI(fontePrincipal, "DESCARTAR", 860, yPos, 14, corDescarte); }
                            else if (p == 1) { DrawTextUI(fontePrincipal, str, 750, yPos, 14, GRAY); DrawTextUI(fontePrincipal, "REPOR", 860, yPos, 14, RED); }
                            else { DrawTextUI(fontePrincipal, str, 750, yPos, 14, RED); DrawTextUI(fontePrincipal, "APP / PROMO", 860, yPos, 14, ORANGE); }
                            vCount++;
                        }
                    }
                    if (p != 2) break; 
                }
            }
        }
        
        else if (telaAtual == TELA_CLIENTE)
        {
            DrawTextUI(fontePrincipal, "App do Consumidor - Loja e Promocoes", 290, 30, 24, corDestaquePromo);
            DrawTextUI(fontePrincipal, "Aproveite para comprar! Estoque proximo ao vencimento.", 290, 65, 14, GRAY);
            
            // --- ÁREA ESQUERDA: OFERTAS ---
            int qtCards = 0, lim[] = {2, 6, 10, 14}, prev = 0;
            for (int f = 0; f < 4; f++)
            {
                for (int i = 0; i < totalProdutos; i++)
                {
                    int dias = produtos[i].dias_para_vencer;
                    if (strcmp(produtos[i].status, "ALERTA") == 0 && dias > prev && dias <= lim[f])
                    {
                        int col = qtCards % 2, row = qtCards / 2; 
                        if (row >= 3) break; 
                        
                        Rectangle cRec = { 280 + (col * 240), 120 + (row * 155), 220, 145 };
                        DrawRectangleRec(cRec, WHITE);
                        DrawRectangleLines(cRec.x, cRec.y, cRec.width, cRec.height, LIGHTGRAY);
                        
                        Rectangle cHead = { cRec.x, cRec.y, cRec.width, 30 };
                        DrawRectangle(cHead.x, cHead.y, cHead.width, cHead.height, corDestaquePromo); 
                        DrawTextUI(fontePrincipal, "OFERTA DO DIA", cRec.x + 50, cRec.y + 8, 14, WHITE);
                        DrawTextUI(fontePrincipal, produtos[i].nome, cRec.x + 15, cRec.y + 40, 14, DARKGRAY);

                        char str[20];
                        sprintf(str, "%d%% OFF!", (f==0)?80:(f==1)?60:(f==2)?40:20);
                        DrawTextUI(fontePrincipal, str, cRec.x + 15, cRec.y + 60, 24, RED);

                        // NOVO: Mostra o Estoque Disponível
                        char qtdStrPromo[30];
                        sprintf(qtdStrPromo, "Estoque: %d un.", produtos[i].quantidade);
                        DrawTextUI(fontePrincipal, qtdStrPromo, cRec.x + 15, cRec.y + 85, 12, GRAY);
                        
                        Rectangle btnAdd = { cRec.x + 15, cRec.y + 105, 190, 30 };
                        DrawRectangleRec(btnAdd, corEntrada);
                        DrawTextUI(fontePrincipal, "ADICIONAR (+1)", cRec.x + 45, cRec.y + 113, 12, WHITE);

                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePoint, btnAdd))
                        {
                            if (GetTime() - tempoUltimoClique > 0.1)
                            {
                                // Verifica quantos desse item já estão no carrinho
                                int qtdNoCarrinho = 0;
                                for (int c = 0; c < totalItensCarrinho; c++)
                                {
                                    if (strcmp(carrinho[c].codigo_lote, produtos[i].codigo_lote) == 0)
                                    {
                                        qtdNoCarrinho = carrinho[c].quantidade;
                                        break;
                                    }
                                }

                                // Só adiciona se o carrinho tiver menos que o estoque real
                                if (qtdNoCarrinho < produtos[i].quantidade)
                                {
                                    bool achou = false;
                                    for (int c = 0; c < totalItensCarrinho; c++)
                                    {
                                        if (strcmp(carrinho[c].codigo_lote, produtos[i].codigo_lote) == 0)
                                        {
                                            carrinho[c].quantidade++;
                                            achou = true;
                                            break;
                                        }
                                    }
                                    if (!achou && totalItensCarrinho < 100)
                                    {
                                        strcpy(carrinho[totalItensCarrinho].nome, produtos[i].nome);
                                        strcpy(carrinho[totalItensCarrinho].codigo_lote, produtos[i].codigo_lote);
                                        carrinho[totalItensCarrinho].quantidade = 1;
                                        totalItensCarrinho++;
                                    }
                                }
                                tempoUltimoClique = GetTime();
                            }
                        }
                        
                        qtCards++;
                    }
                }
                prev = lim[f]; 
            }
            if (qtCards == 0) DrawTextUI(fontePrincipal, "Nenhuma oferta no momento.", 290, 150, 16, GRAY);

            // --- ÁREA DIREITA: CARRINHO E CHECKOUT ---
            Rectangle rectCarrinho = { sw - 260, 120, 240, sh - 150 };
            DrawRectangleRec(rectCarrinho, WHITE);
            DrawRectangleLines(rectCarrinho.x, rectCarrinho.y, rectCarrinho.width, rectCarrinho.height, LIGHTGRAY);
            
            Rectangle headCarrinho = { rectCarrinho.x, rectCarrinho.y, rectCarrinho.width, 40 };
            DrawRectangle(headCarrinho.x, headCarrinho.y, headCarrinho.width, headCarrinho.height, corBotaoAtivo);
            DrawTextUI(fontePrincipal, "SEU CARRINHO", rectCarrinho.x + 60, rectCarrinho.y + 12, 14, WHITE);

            if (totalItensCarrinho == 0)
            {
                DrawTextUI(fontePrincipal, "Carrinho vazio.", rectCarrinho.x + 60, rectCarrinho.y + 70, 14, GRAY);
            }
            else
            {
                // Desenha a lista de itens do carrinho
                for (int c = 0; c < totalItensCarrinho; c++)
                {
                    int itemY = rectCarrinho.y + 60 + (c * 40);
                    if (itemY > sh - 150) break; 
                    
                    DrawTextUI(fontePrincipal, carrinho[c].nome, rectCarrinho.x + 15, itemY, 12, DARKGRAY);
                    
                    char qtdStr[10];
                    sprintf(qtdStr, "x%d", carrinho[c].quantidade);
                    DrawTextUI(fontePrincipal, qtdStr, rectCarrinho.x + 200, itemY, 14, corBotaoAtivo);
                    
                    DrawLine(rectCarrinho.x + 10, itemY + 25, rectCarrinho.x + rectCarrinho.width - 10, itemY + 25, Fade(LIGHTGRAY, 0.5f));
                }

                // Botão de Finalizar Compra
                Rectangle btnFinalizar = { rectCarrinho.x + 20, rectCarrinho.y + rectCarrinho.height - 50, rectCarrinho.width - 40, 35 };
                DrawRectangleRec(btnFinalizar, corDescarte);
                DrawTextUI(fontePrincipal, "FINALIZAR COMPRA", btnFinalizar.x + 35, btnFinalizar.y + 10, 12, WHITE);

                // Lógica de Checkout no Banco de Dados
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePoint, btnFinalizar))
                {
                    if (GetTime() - tempoUltimoClique > 0.5)
                    {
                        for (int c = 0; c < totalItensCarrinho; c++)
                        {
                            for (int q = 0; q < carrinho[c].quantidade; q++)
                            {
                                realizarVendaCliente(carrinho[c].codigo_lote, carrinho[c].nome);
                            }
                        }
                        totalProdutos = carregarProdutos(produtos);
                        totalResumo = carregarResumoDia(resumo);
                        totalItensCarrinho = 0; // Esvazia o carrinho pós compra
                        
                        compraRealizada = true;
                        tempoMsgCompra = GetTime();
                        tempoUltimoClique = GetTime();
                    }
                }
            }

            // Mensagem Verde de Sucesso
            if (compraRealizada)
            {
                if (GetTime() - tempoMsgCompra < 3.0) 
                {
                    DrawTextUI(fontePrincipal, "Compra realizada com sucesso!", rectCarrinho.x + 20, rectCarrinho.y + rectCarrinho.height - 80, 12, corEntrada);
                } 
                else 
                {
                    compraRealizada = false;
                }
            }
        }

        else if (telaAtual == TELA_RELATORIO)
        {
            DrawTextUI(fontePrincipal, "Fechamento: Balancete Consolidado", 290, 30, 24, DARKGRAY);
            
            // CORREÇÃO: O Cartão Branco é desenhado primeiro (e um pouco mais pra baixo)
            Rectangle bgRelatorio = { 290, 180, sw - 330, sh - 280 }; 
            DrawRectangleRec(bgRelatorio, WHITE);
            DrawRectangleLines(bgRelatorio.x, bgRelatorio.y, bgRelatorio.width, bgRelatorio.height, LIGHTGRAY);
            DrawRectangle(290, 180, sw - 330, 40, (Color){ 237, 242, 249, 255 });
            
            // O botão Simular Vendas é desenhado depois, isolado no topo!
            DrawRectangleRec(btnSimularVenda, corBotaoAtivo);
            DrawTextUI(fontePrincipal, "SIMULAR VENDAS (BAIXAR ESTOQUE)", 310, 132, 14, WHITE);

            DrawTextUI(fontePrincipal, "PRODUTO", 310, 192, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "CHEGARAM (+)", 550, 192, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "VENDIDOS (-)", 700, 192, 14, DARKGRAY);
            DrawTextUI(fontePrincipal, "ESTOQUE TOTAL", 850, 192, 14, DARKGRAY); 

            int idxFim = (paginaAtual * 10) + 10;
            if (idxFim > totalResumo) idxFim = totalResumo;

            for (int i = paginaAtual * 10; i < idxFim; i++)
            {
                int yPos = 240 + ((i % 10) * 40); 
                DrawLine(290, yPos - 10, sw - 40, yPos - 10, Fade(LIGHTGRAY, 0.5f));
                
                DrawTextUI(fontePrincipal, resumo[i].produto_nome, 310, yPos, 14, DARKGRAY);
                
                char str[20];
                sprintf(str, "+%d un.", resumo[i].entradas);
                DrawTextUI(fontePrincipal, str, 550, yPos, 14, (resumo[i].entradas > 0) ? corEntrada : LIGHTGRAY);
                
                sprintf(str, "-%d un.", resumo[i].saidas);
                DrawTextUI(fontePrincipal, str, 700, yPos, 14, (resumo[i].saidas > 0) ? RED : LIGHTGRAY);
                
                sprintf(str, "%d un.", resumo[i].saldo_total);
                DrawTextUI(fontePrincipal, str, 850, yPos, 14, (resumo[i].saldo_total <= 0) ? RED : DARKGRAY);
            }

            if (totalResumo == 0) DrawTextUI(fontePrincipal, "Nenhuma movimentacao registrada ainda.", 310, 240, 14, GRAY);
            
            if (paginaAtual > 0) { DrawRectangleRec(btnAnt, corSidebar); DrawTextUI(fontePrincipal, "<< Anterior", 305, sh - 50, 14, WHITE); }
            if (paginaAtual < maxPaginas) { DrawRectangleRec(btnProx, corSidebar); DrawTextUI(fontePrincipal, "Proxima >>", 435, sh - 50, 14, WHITE); }
            
            char pagText[50]; 
            sprintf(pagText, "Pagina %d de %d (Total: %d registros)", paginaAtual + 1, maxPaginas + 1, totalResumo);
            DrawTextUI(fontePrincipal, pagText, 700, sh - 50, 14, GRAY);
        }

        EndDrawing();
    }
    
    if (fontePrincipal.texture.id != GetFontDefault().texture.id) 
    {
        UnloadFont(fontePrincipal);
    }
    CloseWindow();
    return 0;
}