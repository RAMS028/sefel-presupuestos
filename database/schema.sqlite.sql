-- SEFEL - Esquema SQLite (equivalente al esquema PostgreSQL de schema.postgres.sql)
PRAGMA foreign_keys = ON;

CREATE TABLE rol (
  id_rol INTEGER PRIMARY KEY,
  nombre TEXT NOT NULL UNIQUE
);
CREATE TABLE usuario (
  id_usuario INTEGER PRIMARY KEY AUTOINCREMENT,
  id_rol INTEGER NOT NULL REFERENCES rol(id_rol),
  nombre TEXT NOT NULL,
  email TEXT NOT NULL UNIQUE,
  hash_clave TEXT NOT NULL,
  intentos_fallidos INTEGER NOT NULL DEFAULT 0,
  bloqueado_hasta TEXT,
  activo INTEGER NOT NULL DEFAULT 1 CHECK (activo IN (0,1))
);
CREATE TABLE recurso (
  id_recurso INTEGER PRIMARY KEY AUTOINCREMENT,
  codigo TEXT NOT NULL UNIQUE,
  descripcion TEXT NOT NULL,
  unidad TEXT NOT NULL,
  tipo TEXT NOT NULL CHECK (tipo IN ('MATERIAL','MANO_OBRA','EQUIPO','HERRAMIENTA')),
  precio_vigente NUMERIC NOT NULL CHECK (precio_vigente >= 0),
  url_fuente TEXT
);
CREATE TABLE historial_precio (
  id_historial INTEGER PRIMARY KEY AUTOINCREMENT,
  id_recurso INTEGER NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  id_usuario INTEGER NOT NULL REFERENCES usuario(id_usuario),
  precio_anterior NUMERIC,
  precio_nuevo NUMERIC NOT NULL CHECK (precio_nuevo >= 0),
  origen TEXT NOT NULL DEFAULT 'MANUAL' CHECK (origen IN ('MANUAL','AUTOMATICO')),
  fecha TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE TABLE auditoria (
  id_auditoria INTEGER PRIMARY KEY AUTOINCREMENT,
  id_usuario INTEGER NOT NULL REFERENCES usuario(id_usuario),
  accion TEXT NOT NULL,
  entidad TEXT,
  id_entidad INTEGER,
  detalle TEXT,
  fecha TEXT NOT NULL DEFAULT (datetime('now'))
);
CREATE TABLE apu (
  id_apu INTEGER PRIMARY KEY AUTOINCREMENT,
  nombre TEXT NOT NULL,
  unidad TEXT NOT NULL
);
CREATE TABLE apu_detalle (
  id_apu_detalle INTEGER PRIMARY KEY AUTOINCREMENT,
  id_apu INTEGER NOT NULL REFERENCES apu(id_apu) ON DELETE CASCADE,
  id_recurso INTEGER NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  cantidad NUMERIC NOT NULL CHECK (cantidad > 0),
  UNIQUE (id_apu, id_recurso)
);
CREATE TABLE cliente (
  id_cliente INTEGER PRIMARY KEY AUTOINCREMENT,
  nombre TEXT NOT NULL,
  rut TEXT NOT NULL UNIQUE
);
CREATE TABLE proyecto (
  id_proyecto INTEGER PRIMARY KEY AUTOINCREMENT,
  id_cliente INTEGER NOT NULL REFERENCES cliente(id_cliente),
  id_jefe INTEGER REFERENCES usuario(id_usuario),
  nombre TEXT NOT NULL,
  estado TEXT NOT NULL DEFAULT 'PRESUPUESTO' CHECK (estado IN ('PRESUPUESTO','EN_EJECUCION','CERRADO'))
);
CREATE TABLE presupuesto (
  id_presupuesto INTEGER PRIMARY KEY AUTOINCREMENT,
  id_proyecto INTEGER NOT NULL REFERENCES proyecto(id_proyecto),
  fecha TEXT NOT NULL DEFAULT (date('now')),
  utilidad_pct NUMERIC NOT NULL DEFAULT 20 CHECK (utilidad_pct >= 0),
  total NUMERIC NOT NULL DEFAULT 0 CHECK (total >= 0)
);
CREATE TABLE partida (
  id_partida INTEGER PRIMARY KEY AUTOINCREMENT,
  id_presupuesto INTEGER NOT NULL REFERENCES presupuesto(id_presupuesto) ON DELETE CASCADE,
  id_apu INTEGER REFERENCES apu(id_apu),
  descripcion TEXT NOT NULL,
  unidad TEXT NOT NULL,
  cantidad NUMERIC NOT NULL CHECK (cantidad > 0),
  costo_unitario_neto NUMERIC NOT NULL CHECK (costo_unitario_neto >= 0),
  costo_presupuestado NUMERIC NOT NULL CHECK (costo_presupuestado >= 0)
);
CREATE TABLE costo_real (
  id_costo INTEGER PRIMARY KEY AUTOINCREMENT,
  id_proyecto INTEGER NOT NULL REFERENCES proyecto(id_proyecto),
  id_partida INTEGER REFERENCES partida(id_partida),
  id_usuario INTEGER NOT NULL REFERENCES usuario(id_usuario),
  tipo TEXT NOT NULL CHECK (tipo IN ('EJECUCION','COMPRA')),
  categoria TEXT NOT NULL CHECK (categoria IN ('DIRECTO','INDIRECTO')),
  estado TEXT NOT NULL CHECK (estado IN ('COMPROMETIDO','EJECUTADO')),
  monto NUMERIC NOT NULL CHECK (monto > 0),
  descripcion TEXT,
  fecha TEXT NOT NULL DEFAULT (date('now')),
  CHECK (categoria = 'INDIRECTO' OR id_partida IS NOT NULL)
);

-- Tablas de solo inserción (RNF-03)
CREATE TRIGGER trg_historial_no_update BEFORE UPDATE ON historial_precio
BEGIN SELECT RAISE(ABORT, 'historial_precio es de solo insercion (RNF-03)'); END;
CREATE TRIGGER trg_historial_no_delete BEFORE DELETE ON historial_precio
BEGIN SELECT RAISE(ABORT, 'historial_precio es de solo insercion (RNF-03)'); END;
CREATE TRIGGER trg_auditoria_no_update BEFORE UPDATE ON auditoria
BEGIN SELECT RAISE(ABORT, 'auditoria es de solo insercion (RNF-03)'); END;
CREATE TRIGGER trg_auditoria_no_delete BEFORE DELETE ON auditoria
BEGIN SELECT RAISE(ABORT, 'auditoria es de solo insercion (RNF-03)'); END;

CREATE INDEX idx_recurso_descripcion ON recurso (lower(descripcion));
CREATE INDEX idx_historial_recurso   ON historial_precio (id_recurso, fecha);
CREATE INDEX idx_partida_presupuesto ON partida (id_presupuesto);
CREATE INDEX idx_costo_proyecto      ON costo_real (id_proyecto, estado);
CREATE INDEX idx_auditoria_fecha     ON auditoria (fecha);
