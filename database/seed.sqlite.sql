-- Datos de prueba FICTICIOS (no son precios reales de SEFEL)
INSERT INTO rol (id_rol, nombre) VALUES
 (1,'ENCARGADO_PRESUPUESTOS'),(2,'JEFE_PROYECTO'),(3,'GERENCIA'),(4,'ADMINISTRADOR'),(5,'SISTEMA');
-- hash_clave de ejemplo: el backend debe generar hashes bcrypt reales
INSERT INTO usuario (id_rol, nombre, email, hash_clave) VALUES
 (1,'Usuario Presupuestos','presupuestos@prueba.cl','$2b$12$hashDeEjemplo1'),
 (2,'Usuario Jefe Proyecto','jefe@prueba.cl','$2b$12$hashDeEjemplo2'),
 (3,'Usuario Gerencia','gerencia@prueba.cl','$2b$12$hashDeEjemplo3'),
 (4,'Usuario Administrador','admin@prueba.cl','$2b$12$hashDeEjemplo4'),
 (5,'Actualizador automatico','sistema@prueba.cl','$2b$12$sinAcceso');
INSERT INTO recurso (codigo, descripcion, unidad, tipo, precio_vigente) VALUES
 ('MAT-001','Cemento portland 25 kg','saco','MATERIAL',6500),
 ('MAT-002','Arena gruesa','m3','MATERIAL',18000),
 ('MOB-001','Maestro albañil','hora','MANO_OBRA',5200),
 ('EQP-001','Betonera 150 L','dia','EQUIPO',22000);
-- ACT-09: 300 recursos de prueba
WITH RECURSIVE g(n) AS (SELECT 1 UNION ALL SELECT n+1 FROM g WHERE n < 300)
INSERT INTO recurso (codigo, descripcion, unidad, tipo, precio_vigente)
SELECT 'TST-' || substr('0000'||n, -4), 'Recurso de prueba ' || n, 'un', 'MATERIAL', 1000 + ((n*37) % 9000) FROM g;
INSERT INTO historial_precio (id_recurso, id_usuario, precio_anterior, precio_nuevo)
SELECT id_recurso, 1, NULL, precio_vigente FROM recurso;
INSERT INTO apu (nombre, unidad) VALUES ('Radier h=10 cm','m2'),('Muro de albañilería','m2');
INSERT INTO apu_detalle (id_apu, id_recurso, cantidad) VALUES (1,1,0.25),(1,2,0.1),(1,3,0.8),(1,4,0.05),(2,1,0.15),(2,3,1.4);
INSERT INTO cliente (nombre, rut) VALUES ('Cliente de prueba','11.111.111-1');
INSERT INTO proyecto (id_cliente, id_jefe, nombre, estado) VALUES (1,2,'Bodega Coelemu','EN_EJECUCION');
