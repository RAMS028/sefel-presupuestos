-- =====================================================================
-- Datos de prueba (ficticios; NO corresponden a precios reales de SEFEL)
-- =====================================================================
INSERT INTO rol (id_rol, nombre) VALUES
  (1,'ENCARGADO_PRESUPUESTOS'), (2,'JEFE_PROYECTO'), (3,'GERENCIA'), (4,'ADMINISTRADOR'), (5,'SISTEMA');

-- hash_clave: valores de ejemplo; en el sistema se genera con bcrypt
INSERT INTO usuario (id_rol, nombre, email, hash_clave) VALUES
  (1,'Usuario Presupuestos','presupuestos@prueba.cl','$2b$12$hashDeEjemplo1'),
  (2,'Usuario Jefe Proyecto','jefe@prueba.cl','$2b$12$hashDeEjemplo2'),
  (3,'Usuario Gerencia','gerencia@prueba.cl','$2b$12$hashDeEjemplo3'),
  (4,'Usuario Administrador','admin@prueba.cl','$2b$12$hashDeEjemplo4'),
  (5,'Actualizador automatico','sistema@prueba.cl','$2b$12$sinAcceso');   -- usuario técnico del scraper

INSERT INTO recurso (codigo, descripcion, unidad, tipo, precio_vigente) VALUES
  ('MAT-001','Cemento portland 25 kg','saco','MATERIAL',6500),
  ('MAT-002','Arena gruesa','m3','MATERIAL',18000),
  ('MOB-001','Maestro albañil','hora','MANO_OBRA',5200),
  ('EQP-001','Betonera 150 L','dia','EQUIPO',22000);

-- 300 recursos de prueba para volumen y rendimiento (ACT-09, ACT-15)
INSERT INTO recurso (codigo, descripcion, unidad, tipo, precio_vigente)
  SELECT 'TST-' || lpad(g::text, 4, '0'), 'Recurso de prueba ' || g, 'un', 'MATERIAL', 1000 + ((g * 37) % 9000)
  FROM generate_series(1, 300) AS g;

-- Registro inicial del historial (precio_anterior = NULL)
INSERT INTO historial_precio (id_recurso, id_usuario, precio_anterior, precio_nuevo)
  SELECT id_recurso, 1, NULL, precio_vigente FROM recurso;
