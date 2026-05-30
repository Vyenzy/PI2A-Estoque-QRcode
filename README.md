# 📦 Dual-System: Gestão de Estoque Inteligente & Marketplace

![Status](https://img.shields.io/badge/Status-Em_Desenvolvimento-yellow)
![Linguagem](https://img.shields.io/badge/Linguagem-C-blue)
![UI](https://img.shields.io/badge/Interface-Raylib-red)
![Banco](https://img.shields.io/badge/Banco_de_Dados-SQLite-lightgrey)

O **Dual-System** é uma plataforma híbrida de gerenciamento de estoque desenvolvida em **C nativo**. O projeto une a robustez de um banco de dados relacional (SQLite) a uma interface gráfica fluida (Raylib), simulando o ciclo de vida completo de uma mercadoria: desde a chegada no caminhão até a venda promocional para o cliente final.

---

## ✨ Características e Funcionalidades

O sistema foi arquitetado para atender a duas pontas do negócio (por isso, *Dual-System*): o **Gerente** e o **Cliente**.

### 🏢 Visão Gerencial (Back-Office)
* **Visão Geral Paginada:** Monitoramento em tempo real do estoque. O sistema exibe a quantidade, os dias exatos para o vencimento de cada lote e classifica o status (`OK`, `ALERTA` e `ESGOTADO`).
* **Simulador de Scanner QR Code:** Módulo de "Entrada de Lote" que simula a leitura de manifestos de carga. Gera lotes dinâmicos (com quantidades e validades variadas), calcula os riscos e grava no banco de dados automaticamente.
* **Vitrine de Alertas (Fila de Prioridade):** Um painel de triagem inteligente. Produtos que acabaram (`ESGOTADO`) assumem o topo da lista para pedido de reposição. Produtos próximos ao vencimento (`ALERTA`) são listados logo abaixo com seus dias restantes exatos, sugerindo o envio para o app de ofertas.

### 🛒 Visão do Cliente (Front-End / Marketplace)
* **App do Consumidor:** Interface focada em conversão. O cliente não vê o painel burocrático, apenas uma grade atrativa de **Ofertas do Dia**.
* **Precificação Dinâmica:** O sistema aplica regras de negócio reais. Lotes que entram na janela crítica de 14 dias para o vencimento ganham descontos progressivos e agressivos (de **20% até 80% OFF**), gerando senso de urgência ("Restam só X unidades!").

### ⚙️ Funcionalidades Extras (Modo Apresentação)
* **Botão de Reset Dinâmico:** Uma funcionalidade exclusiva para demonstrações. Limpa todos os lotes "simulados" do banco de dados com um clique, restaurando o cenário original da loja para a próxima apresentação sem precisar reiniciar o app.

---

## 🚀 Arquitetura e Tecnologias

* **Linguagem Base:** `C` (Gerenciamento de memória, lógica e structs).
* **Persistência:** `SQLite3` (Vendorizado via *Amalgamation* para rodar localmente sem servidores externos).
* **Motor Gráfico:** `Raylib` (Renderização de UI a 60 FPS acelerada por hardware).
* **Build e Deploy:** `Makefile` e `Docker` (Cross-compilation de Linux para Windows `x86_64-w64-mingw32`).

---

## 📁 Estrutura do Projeto

```text
Sistema/
├── dados/
│   └── banco_estoque.db    # Banco de dados relacional (Schema e Dados)
├── libs/
│   ├── libraylib.a         # Motor gráfico pré-compilado
│   ├── raylib.h            # Headers da interface
│   ├── sqlite3.c           # Motor do banco de dados 
│   └── sqlite3.h
├── interface.c             # Ponto de entrada (Main), lógica de UI e regras de negócio
├── Makefile                # Automação das rotinas de compilação
└── painel.exe              # Executável de produção (Windows)

🛠️ Como Utilizar e Executar
Este projeto foi empacotado para execução direta e simplificada.

Para testes de qualidade, rode o programa "painel.exe" com o nome de Sistema.

Compilação para Desenvolvedores (Via Docker/Make)
Caso deseje modificar o código (interface.c) e gerar uma nova versão:

Abra o terminal do seu ambiente Docker mapeado para a pasta Sistema.

Utilize os comandos automatizados:

make: Compila o projeto em modo Debug (mantém o terminal de log aberto em segundo plano).

make banca: Compila o projeto em modo Release (oculta o terminal de comando, exibindo apenas a UI, ideal para produção).

make clean: Apaga o executável para uma compilação limpa.
