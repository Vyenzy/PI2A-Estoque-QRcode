# 📦 Dual-System: Gestão de Estoque Inteligente & Marketplace

![Status](https://img.shields.io/badge/Status-Stable_v1.1-brightgreen)
![Linguagem](https://img.shields.io/badge/Linguagem-C_Nativo-blue)
![UI](https://img.shields.io/badge/Interface-Raylib-red)
![Banco](https://img.shields.io/badge/Banco_de_Dados-SQLite3-lightgrey)

O **Dual-System** é uma plataforma híbrida de gerenciamento de estoque desenvolvida 100% em **C nativo**. O projeto une a robustez de um banco de dados relacional (SQLite3) a uma interface gráfica fluida (Raylib), orquestrando a concorrência lógica entre o motor gráfico e as transações de banco. O ecossistema simula o ciclo de vida completo de uma mercadoria: desde a entrada logística via leitor de código de barras até a venda promocional para o cliente final.

Projeto desenvolvido para a disciplina de Projeto Integrador do curso de Engenharia da Computação do Centro Universitário IESB.

---

## ✨ Características e Funcionalidades (v1.1)

O sistema foi arquitetado para atender a duas pontas do negócio de forma simultânea: o **Gerente** (Back-Office) e o **Cliente** (Marketplace).

### 🏢 Visão Gerencial (Back-Office)
* **Integração de Hardware (Leitor USB):** Suporte nativo a leitores de código de barras físicos (HID-KBW). A engine foi otimizada para **240 FPS**, prevenindo *overflow* de buffer e perda de caracteres durante a leitura ultrarrápida do hardware.
* **Visão Geral Paginada:** Monitoramento do estoque com cálculos precisos de validade. A matemática de datas foi delegada ao banco de dados via função `julianday` para evitar bugs de fuso horário.
* **Vitrine de Alertas (Fila de Prioridade):** Painel de triagem inteligente que ordena produtos por urgência (`ESGOTADO`, `VENCIDO` e `ALERTA`).

### 🛒 Visão do Cliente (Marketplace)
* **Precificação Dinâmica:** Lotes que entram na janela crítica de 14 dias para o vencimento ganham descontos escalonados e automáticos (de **20% até 80% OFF**), gerando giro rápido de estoque.
* **Trava de Estoque Físico:** Verificação síncrona que impede o esmagamento de dados, bloqueando a adição de itens no carrinho além da quantidade física disponível no banco.
* **Checkout Transacional:** Processamento em lote das compras, garantindo a baixa imediata e segura no SQLite.

### 🎨 Arquitetura de UI/UX
* **Motor de Temas Dinâmico (Dark Mode):** Alternância em tempo real entre Tema Claro e Tema Escuro (tons Slate/Zinc), garantindo ergonomia visual para operações prolongadas.
* **Modern Flat Design:** Interface construída com primitivas ortogonais, realizando um bypass em cálculos curvos de OpenGL para garantir compatibilidade com GPUs integradas mais antigas.

---

## 🚀 Tecnologias Utilizadas

* **C (Padrão ANSI):** Lógica central, structs e ponteiros.
* **SQLite3:** Motor relacional vendorizado via *Amalgamation* (não requer servidor externo).
* **Raylib:** Renderização de UI bidimensional acelerada por hardware.

---

## 🛠️ Como Utilizar e Executar (Plug and Play)

Para facilitar a avaliação da banca e o uso em qualquer computador Windows (sem necessidade de configurar compiladores), o executável já está empacotado.

1. Clone ou baixe este repositório.
2. Navegue até a pasta raiz do projeto.
3. Dê um duplo clique no arquivo **`painel.exe`**.
4. O sistema iniciará automaticamente conectado ao banco de dados local `dados/banco_estoque.db`.

### 👨‍💻 Compilação para Desenvolvedores (Make / GCC)
Caso deseje modificar o código (`interface.c` ou `estoque_db.c`) e gerar uma nova versão de compilação cruzada (Linux -> Windows):

```bash
# Compila o projeto em modo Debug (mantém o terminal de log aberto em segundo plano)
make

# Compila o projeto em modo Release (oculta o terminal de comando, exibindo apenas a UI)
make banca

# Apaga o executável e arquivos objeto para uma compilação limpa
make clean

## 📁 Estrutura do Projeto

## 📁 Estrutura do Projeto

/
├── .vscode/                # Configurações de ambiente do editor
├── Sistema/                # Diretório principal da aplicação
│   ├── dados/              # Arquivos do banco de dados local (.db)
│   ├── libs/               # Dependências pré-compiladas (Raylib, SQLite3)
│   ├── estoque_db.c        # Módulo de banco de dados (Transações SQL)
│   ├── estoque_db.h        # Módulo de banco de dados (Cabeçalhos)
│   ├── fonte.ttf           # Fonte tipográfica customizada da interface
│   ├── interface.c         # Ponto de entrada (Main) e renderização visual
│   ├── Makefile            # Automação das rotinas de build
│   └── painel.exe          # Executável final de produção (Windows)
├── .gitignore              # Regras de exclusão de arquivos no controle de versão
├── docker-compose.yml      # Configuração do ambiente conteinerizado
└── README.md               # Documentação principal do repositório