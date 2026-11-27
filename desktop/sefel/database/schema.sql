CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE IF NOT EXISTS rol (
  id_rol SMALLINT PRIMARY KEY,
  nombre VARCHAR(40) NOT NULL UNIQUE
);
CREATE TABLE IF NOT EXISTS usuario (
  id_usuario SERIAL PRIMARY KEY,
  id_rol SMALLINT NOT NULL REFERENCES rol(id_rol),
  nombre VARCHAR(100) NOT NULL,
  email VARCHAR(150) NOT NULL UNIQUE,
  hash_clave TEXT NOT NULL,
  activo BOOLEAN NOT NULL DEFAULT TRUE
);
CREATE TABLE IF NOT EXISTS recurso (
  id_recurso SERIAL PRIMARY KEY,
  codigo VARCHAR(30) NOT NULL UNIQUE,
  descripcion VARCHAR(200) NOT NULL,
  unidad VARCHAR(15) NOT NULL,
  tipo VARCHAR(15) NOT NULL CHECK (tipo IN ('MATERIAL','MANO_OBRA','EQUIPO','HERRAMIENTA')),
  precio_vigente NUMERIC(14,2) NOT NULL CHECK (precio_vigente > 0),
  activo BOOLEAN NOT NULL DEFAULT TRUE
);
ALTER TABLE recurso ADD COLUMN IF NOT EXISTS activo BOOLEAN NOT NULL DEFAULT TRUE;
-- Normaliza códigos antiguos y arbitrarios para que cumplan el formato del tipo.
DO $$
DECLARE r RECORD; prefix TEXT; next_code INTEGER; candidate TEXT;
BEGIN
  FOR r IN SELECT id_recurso,tipo,codigo FROM recurso ORDER BY id_recurso LOOP
    prefix := CASE r.tipo
      WHEN 'MATERIAL' THEN 'MAT'
      WHEN 'MANO_OBRA' THEN 'MO'
      WHEN 'EQUIPO' THEN 'EQP'
      ELSE 'HER'
    END;
    IF r.codigo !~ ('^' || prefix || '-[0-9]{4}$') THEN
      SELECT COALESCE(MAX(substring(codigo from '[0-9]+$')::integer),0)+1
        INTO next_code FROM recurso
        WHERE tipo=r.tipo AND codigo ~ ('^' || prefix || '-[0-9]{4}$');
      candidate := prefix || '-' || lpad(next_code::text,4,'0');
      WHILE EXISTS (SELECT 1 FROM recurso WHERE codigo=candidate) LOOP
        next_code := next_code + 1;
        candidate := prefix || '-' || lpad(next_code::text,4,'0');
      END LOOP;
      UPDATE recurso SET codigo=candidate WHERE id_recurso=r.id_recurso;
    END IF;
  END LOOP;
END $$;
DO $$
BEGIN
  IF NOT EXISTS (SELECT 1 FROM pg_constraint WHERE conname='ck_recurso_codigo_formato') THEN
    ALTER TABLE recurso ADD CONSTRAINT ck_recurso_codigo_formato CHECK (
      codigo ~ '^(MAT|MO|EQP|HER)-[0-9]{4}$' AND
      ((tipo='MATERIAL' AND codigo LIKE 'MAT-%') OR
       (tipo='MANO_OBRA' AND codigo LIKE 'MO-%') OR
       (tipo='EQUIPO' AND codigo LIKE 'EQP-%') OR
       (tipo='HERRAMIENTA' AND codigo LIKE 'HER-%'))
    ) NOT VALID;
  END IF;
END $$;
CREATE TABLE IF NOT EXISTS historial_precio (
  id_historial SERIAL PRIMARY KEY,
  id_recurso INTEGER NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  id_usuario INTEGER NOT NULL REFERENCES usuario(id_usuario),
  precio_anterior NUMERIC(14,2),
  precio_nuevo NUMERIC(14,2) NOT NULL CHECK (precio_nuevo > 0),
  origen VARCHAR(10) NOT NULL DEFAULT 'MANUAL' CHECK (origen IN ('MANUAL','AUTOMATICO')),
  fecha TIMESTAMPTZ NOT NULL DEFAULT now()
);
-- Corrige recursos heredados en cero a $0,10 y deja trazabilidad si hay usuarios.
DO $$
DECLARE migration_user INTEGER;
BEGIN
  SELECT id_usuario INTO migration_user FROM usuario ORDER BY id_usuario LIMIT 1;
  IF migration_user IS NULL THEN
    UPDATE recurso SET precio_vigente=0.10 WHERE precio_vigente<=0;
  ELSE
    WITH fixed AS (
      UPDATE recurso SET precio_vigente=0.10 WHERE precio_vigente<=0
      RETURNING id_recurso
    )
    INSERT INTO historial_precio(id_recurso,id_usuario,precio_anterior,precio_nuevo,origen)
      SELECT id_recurso,migration_user,0,0.10,'MANUAL' FROM fixed;
  END IF;
  IF NOT EXISTS (SELECT 1 FROM pg_constraint WHERE conname='ck_recurso_precio_positivo' AND conrelid='recurso'::regclass) THEN
    ALTER TABLE recurso ADD CONSTRAINT ck_recurso_precio_positivo CHECK (precio_vigente > 0) NOT VALID;
  END IF;
  IF NOT EXISTS (SELECT 1 FROM pg_constraint WHERE conname='ck_historial_precio_positivo' AND conrelid='historial_precio'::regclass) THEN
    ALTER TABLE historial_precio ADD CONSTRAINT ck_historial_precio_positivo CHECK (precio_nuevo > 0) NOT VALID;
  END IF;
END $$;
CREATE TABLE IF NOT EXISTS apu (
  id_apu SERIAL PRIMARY KEY,
  nombre VARCHAR(150) NOT NULL,
  unidad VARCHAR(15) NOT NULL
);
CREATE TABLE IF NOT EXISTS apu_detalle (
  id_apu_detalle SERIAL PRIMARY KEY,
  id_apu INTEGER NOT NULL REFERENCES apu(id_apu) ON DELETE CASCADE,
  id_recurso INTEGER NOT NULL REFERENCES recurso(id_recurso) ON DELETE RESTRICT,
  cantidad NUMERIC(14,4) NOT NULL CHECK (cantidad > 0),
  UNIQUE (id_apu,id_recurso)
);
CREATE TABLE IF NOT EXISTS cliente (
  id_cliente SERIAL PRIMARY KEY,
  nombre VARCHAR(150) NOT NULL,
  rut VARCHAR(12) NOT NULL UNIQUE
);
CREATE TABLE IF NOT EXISTS proyecto (
  id_proyecto SERIAL PRIMARY KEY,
  id_cliente INTEGER NOT NULL REFERENCES cliente(id_cliente),
  id_jefe INTEGER REFERENCES usuario(id_usuario),
  nombre VARCHAR(150) NOT NULL,
  estado VARCHAR(15) NOT NULL DEFAULT 'PRESUPUESTO' CHECK (estado IN ('PRESUPUESTO','EN_EJECUCION','CERRADO'))
);
CREATE TABLE IF NOT EXISTS presupuesto (
  id_presupuesto SERIAL PRIMARY KEY,
  nombre VARCHAR(150) NOT NULL,
  fecha DATE NOT NULL DEFAULT CURRENT_DATE,
  utilidad_pct NUMERIC(5,2) NOT NULL DEFAULT 20 CHECK (utilidad_pct >= 0),
  total NUMERIC(16,2) NOT NULL DEFAULT 0 CHECK (total >= 0),
  id_usuario INTEGER NOT NULL REFERENCES usuario(id_usuario)
);
-- Migra instalaciones anteriores: agregan nombre/usuario y dejan opcional el proyecto legado.
ALTER TABLE presupuesto ADD COLUMN IF NOT EXISTS nombre VARCHAR(150);
ALTER TABLE presupuesto ADD COLUMN IF NOT EXISTS id_usuario INTEGER REFERENCES usuario(id_usuario);
UPDATE presupuesto
   SET nombre='Presupuesto ' || id_presupuesto::text
 WHERE nombre IS NULL OR btrim(nombre)='';
ALTER TABLE presupuesto ALTER COLUMN nombre SET NOT NULL;
UPDATE presupuesto
   SET id_usuario=(SELECT id_usuario FROM usuario ORDER BY id_usuario LIMIT 1)
 WHERE id_usuario IS NULL AND EXISTS (SELECT 1 FROM usuario);
DO $$
BEGIN
  IF EXISTS (
    SELECT 1 FROM information_schema.columns
     WHERE table_schema=current_schema() AND table_name='presupuesto' AND column_name='id_proyecto'
  ) THEN
    ALTER TABLE presupuesto ALTER COLUMN id_proyecto DROP NOT NULL;
  END IF;
END $$;
CREATE TABLE IF NOT EXISTS partida (
  id_partida SERIAL PRIMARY KEY,
  id_presupuesto INTEGER NOT NULL REFERENCES presupuesto(id_presupuesto) ON DELETE CASCADE,
  id_apu INTEGER REFERENCES apu(id_apu),
  descripcion VARCHAR(200) NOT NULL,
  unidad VARCHAR(15) NOT NULL,
  cantidad NUMERIC(14,4) NOT NULL CHECK (cantidad > 0),
  costo_unitario_neto NUMERIC(14,2) NOT NULL CHECK (costo_unitario_neto >= 0),
  costo_presupuestado NUMERIC(16,2) NOT NULL CHECK (costo_presupuestado >= 0)
);
CREATE OR REPLACE FUNCTION proteger_historial_precio() RETURNS trigger AS $$
BEGIN RAISE EXCEPTION 'El historial de precios es inmutable'; END;
$$ LANGUAGE plpgsql;
DROP TRIGGER IF EXISTS trg_historial_inmutable ON historial_precio;
CREATE TRIGGER trg_historial_inmutable BEFORE UPDATE OR DELETE ON historial_precio
FOR EACH ROW EXECUTE FUNCTION proteger_historial_precio();
CREATE INDEX IF NOT EXISTS idx_recurso_descripcion ON recurso (lower(descripcion));
CREATE INDEX IF NOT EXISTS idx_historial_recurso ON historial_precio (id_recurso, fecha DESC);
CREATE INDEX IF NOT EXISTS idx_partida_presupuesto ON partida (id_presupuesto);
