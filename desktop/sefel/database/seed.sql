INSERT INTO rol(id_rol,nombre) VALUES
 (1,'ENCARGADO_PRESUPUESTOS'),(2,'JEFE_PROYECTO'),(3,'GERENCIA')
ON CONFLICT (id_rol) DO NOTHING;
INSERT INTO usuario(id_rol,nombre,email,hash_clave) VALUES
 (1,'Encargado de Presupuestos','presupuestos@sefel.local',crypt('Demo123!',gen_salt('bf'))),
 (2,'Jefe de Proyecto','jefe@sefel.local',crypt('Demo123!',gen_salt('bf'))),
 (3,'Gerencia General','gerencia@sefel.local',crypt('Demo123!',gen_salt('bf')))
ON CONFLICT (email) DO NOTHING;
INSERT INTO recurso(codigo,descripcion,unidad,tipo,precio_vigente) VALUES
 ('MAT-001','Cemento portland 25 kg','saco','MATERIAL',6500),
 ('MAT-002','Arena gruesa','m3','MATERIAL',18000),
 ('MOB-001','Maestro albañil','hora','MANO_OBRA',5200),
 ('EQP-001','Betonera 150 L','dia','EQUIPO',22000)
ON CONFLICT (codigo) DO NOTHING;
INSERT INTO historial_precio(id_recurso,id_usuario,precio_anterior,precio_nuevo)
 SELECT r.id_recurso,u.id_usuario,NULL,r.precio_vigente FROM recurso r
 CROSS JOIN LATERAL (SELECT id_usuario FROM usuario WHERE email='presupuestos@sefel.local') u
 WHERE NOT EXISTS (SELECT 1 FROM historial_precio h WHERE h.id_recurso=r.id_recurso);
INSERT INTO cliente(nombre,rut) VALUES ('Cliente de demostración','11111111-1') ON CONFLICT (rut) DO NOTHING;
INSERT INTO proyecto(id_cliente,nombre) SELECT id_cliente,'Obra de demostración' FROM cliente WHERE rut='11111111-1'
 AND NOT EXISTS (SELECT 1 FROM proyecto WHERE nombre='Obra de demostración');
