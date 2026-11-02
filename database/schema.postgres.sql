-- =====================================================================
-- Sistema de Gestión Integral de Presupuestos y Costos - SEFEL
-- Esquema relacional (PostgreSQL 14 o superior)
-- =====================================================================

CREATE TABLE rol (
  id_rol      SMALLINT     PRIMARY KEY,
  nombre      VARCHAR(40)  NOT NULL UNIQUE
);

CREATE TABLE usuario (
  id_usuario        SERIAL        PRIMARY KEY,
  id_rol            SMALLINT      NOT NULL REFERENCES rol(id_rol),
  nombre            VARCHAR(100)  NOT NULL,
  email             VARCHAR(150)  NOT NULL UNIQUE,
  hash_clave        VARCHAR(100)  NOT NULL,
  intentos_fallidos SMALLINT      NOT NULL DEFAULT 0,
  bloqueado_hasta   TIMESTAMPTZ,
  activo            BOOLEAN       NOT NULL DEFAULT TRUE
);

CREATE TABLE recurso (
  id_recurso      SERIAL         PRIMARY KEY,
  codigo          VARCHAR(30)    NOT NULL UNIQUE,
  descripcion     VARCHAR(200)   NOT NULL,
  unidad          VARCHAR(15)    NOT NULL,
  tipo            VARCHAR(15)    NOT NULL
                  CHECK (tipo IN ('MATERIAL','MANO_OBRA','EQUIPO','HERRAMIENTA')),
  precio_vigente  NUMERIC(14,2)  NOT NULL CHECK (precio_vigente >= 0),
  url_fuente      VARCHAR(300)                      -- fuente web para la actualización automática (opcional)
);

CREATE TABLE historial_precio (
  id_historial     SERIAL         PRIMARY KEY,
  id_recurso       INTEGER        NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  id_usuario       INTEGER        NOT NULL REFERENCES usuario(id_usuario),
  precio_anterior  NUMERIC(14,2),                       -- NULL en el registro inicial
  precio_nuevo     NUMERIC(14,2)  NOT NULL CHECK (precio_nuevo >= 0),
  origen           VARCHAR(10)    NOT NULL DEFAULT 'MANUAL' CHECK (origen IN ('MANUAL','AUTOMATICO')),
  fecha            TIMESTAMPTZ    NOT NULL DEFAULT now()
);

-- Auditoría de accesos, altas, modificaciones y errores del scraper (ACT-17)
CREATE TABLE auditoria (
  id_auditoria  SERIAL        PRIMARY KEY,
  id_usuario    INTEGER       NOT NULL REFERENCES usuario(id_usuario),
  accion        VARCHAR(30)   NOT NULL,        -- LOGIN, ALTA, MODIFICACION, ERROR_SCRAPER...
  entidad       VARCHAR(30),
  id_entidad    INTEGER,
  detalle       VARCHAR(300),
  fecha         TIMESTAMPTZ   NOT NULL DEFAULT now()
);

CREATE TABLE apu (
  id_apu  SERIAL        PRIMARY KEY,
  nombre  VARCHAR(150)  NOT NULL,
  unidad  VARCHAR(15)   NOT NULL
);

CREATE TABLE apu_detalle (
  id_apu_detalle  SERIAL         PRIMARY KEY,
  id_apu          INTEGER        NOT NULL REFERENCES apu(id_apu) ON DELETE CASCADE,
  id_recurso      INTEGER        NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  cantidad        NUMERIC(14,4)  NOT NULL CHECK (cantidad > 0),
  UNIQUE (id_apu, id_recurso)
);

CREATE TABLE cliente (
  id_cliente  SERIAL        PRIMARY KEY,
  nombre      VARCHAR(150)  NOT NULL,
  rut         VARCHAR(12)   NOT NULL UNIQUE
);

CREATE TABLE proyecto (
  id_proyecto  SERIAL        PRIMARY KEY,
  id_cliente   INTEGER       NOT NULL REFERENCES cliente(id_cliente),
  id_jefe      INTEGER       REFERENCES usuario(id_usuario),   -- se asigna al aprobarse
  nombre       VARCHAR(150)  NOT NULL,
  estado       VARCHAR(15)   NOT NULL DEFAULT 'PRESUPUESTO'
               CHECK (estado IN ('PRESUPUESTO','EN_EJECUCION','CERRADO'))
);

CREATE TABLE presupuesto (
  id_presupuesto  SERIAL         PRIMARY KEY,
  id_proyecto     INTEGER        NOT NULL REFERENCES proyecto(id_proyecto),
  fecha           DATE           NOT NULL DEFAULT CURRENT_DATE,
  utilidad_pct    NUMERIC(5,2)   NOT NULL DEFAULT 20.00 CHECK (utilidad_pct >= 0),
  total           NUMERIC(16,2)  NOT NULL DEFAULT 0 CHECK (total >= 0)
);

CREATE TABLE partida (
  id_partida           SERIAL         PRIMARY KEY,
  id_presupuesto       INTEGER        NOT NULL REFERENCES presupuesto(id_presupuesto) ON DELETE CASCADE,
  id_apu               INTEGER        REFERENCES apu(id_apu),
  descripcion          VARCHAR(200)   NOT NULL,
  unidad               VARCHAR(15)    NOT NULL,
  cantidad             NUMERIC(14,4)  NOT NULL CHECK (cantidad > 0),
  costo_unitario_neto  NUMERIC(14,2)  NOT NULL CHECK (costo_unitario_neto >= 0),
  costo_presupuestado  NUMERIC(16,2)  NOT NULL CHECK (costo_presupuestado >= 0)
);

CREATE TABLE costo_real (
  id_costo     SERIAL         PRIMARY KEY,
  id_proyecto  INTEGER        NOT NULL REFERENCES proyecto(id_proyecto),
  id_partida   INTEGER        REFERENCES partida(id_partida),
  id_usuario   INTEGER        NOT NULL REFERENCES usuario(id_usuario),
  tipo         VARCHAR(10)    NOT NULL CHECK (tipo IN ('EJECUCION','COMPRA')),
  categoria    VARCHAR(10)    NOT NULL CHECK (categoria IN ('DIRECTO','INDIRECTO')),
  estado       VARCHAR(12)    NOT NULL CHECK (estado IN ('COMPROMETIDO','EJECUTADO')),
  monto        NUMERIC(16,2)  NOT NULL CHECK (monto > 0),
  descripcion  VARCHAR(200),
  fecha        DATE           NOT NULL DEFAULT CURRENT_DATE,
  -- Un costo indirecto puede no tener partida; uno directo siempre la tiene (HU-07)
  CHECK (categoria = 'INDIRECTO' OR id_partida IS NOT NULL)
);

-- Historial de solo inserción (RNF-03)
CREATE OR REPLACE FUNCTION proteger_historial() RETURNS trigger AS $$
BEGIN
  RAISE EXCEPTION 'tabla de solo insercion (RNF-03)';
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_historial_inmutable
  BEFORE UPDATE OR DELETE ON historial_precio
  FOR EACH ROW EXECUTE FUNCTION proteger_historial();

CREATE TRIGGER trg_auditoria_inmutable
  BEFORE UPDATE OR DELETE ON auditoria
  FOR EACH ROW EXECUTE FUNCTION proteger_historial();

-- Índices para consultas frecuentes (RNF-04)
CREATE INDEX idx_recurso_descripcion ON recurso (lower(descripcion));
CREATE INDEX idx_historial_recurso   ON historial_precio (id_recurso, fecha);
CREATE INDEX idx_partida_presupuesto ON partida (id_presupuesto);
CREATE INDEX idx_costo_proyecto      ON costo_real (id_proyecto, estado);
CREATE INDEX idx_auditoria_fecha     ON auditoria (fecha);
