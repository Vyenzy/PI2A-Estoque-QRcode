# Sistema Autônomo de Monitoramento de Estoque Varejista via QR Code
**Projeto Integrador 2A - ENG183 | Grupo 1 | IESB**

## 📌 Sobre o Projeto
Este repositório contém o código-fonte e a documentação do projeto de gestão inteligente de estoque (Dual-System). O sistema utiliza uma **Gestão de Lotes Híbrida**, lendo QR Codes de caixas master para alimentar um banco de dados local. 

O sistema possui duas frentes:
1. **Back-end (Prevenção de Perdas):** Monitora validades e alerta sobre produtos vencendo.
2. **Front-end (Vitrine de Oportunidades):** Produtos próximos ao vencimento (ex: < 7 dias) recebem alertas de desconto.

## 🛠️ Tecnologias Utilizadas
* **Banco de Dados:** SQLite (`.sql` e `.db`)
* **Lógica e Arquitetura:** Linguagem C (com biblioteca `sqlite3.h`)
* **Interface:** Terminal interativo (CLI)

---

## 📋 Divisão de Tarefas - Fase de Execução

Abaixo estão as responsabilidades de cada especialista para a integração do protótipo:

* **@Andrew (Regras de Negócio e Logística):**
  * Desenvolver os scripts em C para Inserção (INSERT) e Busca (SELECT) conectando ao arquivo `.db`.
  * Implementar a lógica de alerta para lotes com validade inferior a 7 dias.

* **@Pedro Ivan (Arquitetura de Software):**
  * Estruturar o Menu Interativo principal no terminal.
  * Conectar as opções do menu às funções de banco de dados desenvolvidas pela regra de negócio.

* **@Sabino (Especificação de Requisitos):**
  * Redigir o documento final com os Requisitos Funcionais e Não-Funcionais baseados no escopo Dual-System implementado.

* **@Lucas Ferreira (Testes e Qualidade):**
  * Montar a planilha de Cenários de Teste.
  * Executar e documentar testes de limite (ex: tentar inserir lote duplicado, verificar disparo de alertas de vencimento).

---

## 🚀 Como rodar o projeto localmente
1. Clone o repositório: `git clone https://github.com/Vyenzy/PI2A-Estoque-QRcode.git`
2. Mude para a branch de desenvolvimento: `git checkout dev`
3. Compile o código em C (necessário GCC e SQLite3):
   `gcc main.c -o estoque -lsqlite3`
4. Execute o programa:
   `./estoque` (Linux/Mac) ou `estoque.exe` (Windows)
5. Como usar o docker:
   O docker é um facilitador na hora de usar o GCC. Caso você já tenha o gcc instalado, ignore este passo;
   Baixe o docker no seu computador, e clique no arquivo 'docker-compose.yml'. Neste, haverá um ">run all services" no topo. Apenas clique neste, e ele gerará um container no docker.
   Assim que a imagem de gcc for gerada no docker, a execute pelo docker, e abra o container na aba de "containers" do vscode com o botão direito, na opção "Attach Visual studio code".