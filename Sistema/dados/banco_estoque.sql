-- ================================================
-- Banco de Dados: Sistema de Estoque via QR Code
-- Projeto Integrador 2A - ENG183 | Grupo I | IESB
-- ================================================

-- Tabela 1: produtos
CREATE TABLE IF NOT EXISTS produtos (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    codigo      TEXT    NOT NULL UNIQUE,
    nome        TEXT    NOT NULL,
    categoria   TEXT,
    qtd_minima  INTEGER DEFAULT 5,
    qtd_atual   INTEGER DEFAULT 0,
    criado_em   DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabela 2: lotes
CREATE TABLE IF NOT EXISTS lotes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    produto_id  INTEGER NOT NULL,
    codigo_lote TEXT    NOT NULL UNIQUE,
    quantidade  INTEGER NOT NULL,
    validade    DATE    NOT NULL,
    data_entrada DATETIME DEFAULT CURRENT_TIMESTAMP,
    status      TEXT    DEFAULT 'ativo',
    FOREIGN KEY (produto_id) REFERENCES produtos(id)
);

-- Tabela 3: movimentacoes (Histórico geral/Legado)
CREATE TABLE IF NOT EXISTS movimentacoes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    lote_id     INTEGER NOT NULL,
    produto_id  INTEGER NOT NULL,
    tipo        TEXT    NOT NULL,
    quantidade  INTEGER NOT NULL,
    data_hora   DATETIME DEFAULT CURRENT_TIMESTAMP,
    responsavel TEXT    DEFAULT 'sistema',
    FOREIGN KEY (lote_id)    REFERENCES lotes(id),
    FOREIGN KEY (produto_id) REFERENCES produtos(id)
);

-- Tabela 4: vendas (Saída direta para o Relatório do Dia)
CREATE TABLE IF NOT EXISTS vendas (
    id           INTEGER PRIMARY KEY AUTOINCREMENT,
    cliente_id   TEXT,
    produto_nome TEXT,
    quantidade   INTEGER,
    data_hora    DATETIME DEFAULT (datetime('now', 'localtime'))
);

-- ================================================
-- Dados iniciais de exemplo (Sementes)
-- ================================================
INSERT INTO produtos (codigo, nome, categoria, qtd_minima, qtd_atual)
VALUES
    ('PRD-001', 'Arroz 5kg',    'Alimentos', 20, 0),
    ('PRD-002', 'Feijao 1kg',   'Alimentos', 15, 0),
    ('PRD-003', 'Acucar 2kg',   'Alimentos', 15, 0);