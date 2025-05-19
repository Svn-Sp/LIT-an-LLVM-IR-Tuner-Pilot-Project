#!/bin/bash

sqlite3 results.db << 'EOF'
CREATE TABLE IF NOT EXISTS code (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS run (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    code_id INTEGER,
    avg_rt REAL NOT NULL,
    std_dev_rt REAL NOT NULL,
    FOREIGN KEY (code_id) REFERENCES code(id)
);

CREATE TABLE IF NOT EXISTS mutation (
    step INTEGER NOT NULL,
    run_id INTEGER NOT NULL,
    mutation_type TEXT NOT NULL,
    decisions TEXT NOT NULL,
    PRIMARY KEY (run_id, step),
    FOREIGN KEY (run_id) REFERENCES run(id)
);
EOF