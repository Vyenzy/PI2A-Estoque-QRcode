-- ================================================
-- Banco de Dados: Sistema de Estoque via QR Code
-- Projeto Integrador 2A - ENG183 | Grupo I | IESB
-- ================================================

-- Tabela 1: produtos
-- Guarda o cadastro fixo de cada produto do estoque
CREATE TABLE IF NOT EXISTS produtos (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    codigo      TEXT    NOT NULL UNIQUE,  -- ex: "ARR001"
    nome        TEXT    NOT NULL,         -- ex: "Arroz 5kg"
    categoria   TEXT,                     -- ex: "Alimentos"
    qtd_minima  INTEGER DEFAULT 5,        -- alerta se ficar abaixo disso
    qtd_atual   INTEGER DEFAULT 0,        -- atualizado a cada movimentação
    criado_em   DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabela 2: lotes
-- Cada leitura de QR Code gera um registro aqui
CREATE TABLE IF NOT EXISTS lotes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    produto_id  INTEGER NOT NULL,
    codigo_lote TEXT    NOT NULL UNIQUE,  -- ex: "LOT001"
    quantidade  INTEGER NOT NULL,
    validade    DATE    NOT NULL,          -- ex: "2026-12-01"
    data_entrada DATETIME DEFAULT CURRENT_TIMESTAMP,
    status      TEXT    DEFAULT 'ativo',  -- 'ativo', 'vencido', 'esgotado'
    FOREIGN KEY (produto_id) REFERENCES produtos(id)
);

-- Tabela 3: movimentacoes
-- Registra cada entrada ou saída do estoque
CREATE TABLE IF NOT EXISTS movimentacoes (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    lote_id     INTEGER NOT NULL,
    produto_id  INTEGER NOT NULL,
    tipo        TEXT    NOT NULL,   -- 'entrada' ou 'saida'
    quantidade  INTEGER NOT NULL,
    data_hora   DATETIME DEFAULT CURRENT_TIMESTAMP,
    responsavel TEXT    DEFAULT 'sistema',
    FOREIGN KEY (lote_id)    REFERENCES lotes(id),
    FOREIGN KEY (produto_id) REFERENCES produtos(id)
);

-- ================================================
-- Dados iniciais de exemplo (para testes)
-- ================================================

INSERT INTO produtos (codigo, nome, categoria, qtd_minima, qtd_atual)
VALUES
    ('ARR001', 'Arroz 5kg',    'Alimentos', 10, 0),
    ('FEI001', 'Feijão 1kg',   'Alimentos',  5, 0),
    ('ACU001', 'Açúcar 2kg',   'Alimentos',  8, 0);

-- ================================================
-- Consultas úteis para relatórios
-- ================================================

-- Ver estoque atual de todos os produtos:
-- SELECT codigo, nome, qtd_atual, qtd_minima FROM produtos;

-- Ver produtos com estoque abaixo do mínimo (alerta reposição):
-- SELECT nome, qtd_atual, qtd_minima FROM produtos WHERE qtd_atual < qtd_minima;

-- Ver lotes próximos do vencimento (7 dias):
-- SELECT l.codigo_lote, p.nome, l.validade, l.quantidade
-- FROM lotes l JOIN produtos p ON l.produto_id = p.id
-- WHERE l.validade <= DATE('now', '+7 days') AND l.status = 'ativo';

-- Ver histórico de movimentações:
-- SELECT m.data_hora, p.nome, m.tipo, m.quantidade
-- FROM movimentacoes m JOIN produtos p ON m.produto_id = p.id
-- ORDER BY m.data_hora DESC;
