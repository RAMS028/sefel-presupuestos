# SEFEL Desktop — MVP

Aplicación de escritorio en C++/Qt Widgets con PostgreSQL. Incluye login con roles, catálogo y búsqueda de recursos (HU-01/02), modificación de precios con historial (HU-03/04), edición de APU y cálculo/guardado/exportación CSV del presupuesto (HU-05/06). HU-07 a HU-10 quedan fuera del MVP ejecutable.

## Requisitos

- Qt 6.4+ con módulos **Widgets** y **Sql**.
- PostgreSQL 14+ y el driver Qt **QPSQL**.
- CMake 3.21+ y compilador C++17.

## Inicio rápido

1. Crear una BD vacía, por ejemplo `sefel`.
2. Ejecutar `psql -d sefel -f database/schema.sql` y después `psql -d sefel -f database/seed.sql`.
3. Configurar conexión mediante variables `SEFEL_DB_HOST`, `SEFEL_DB_PORT`, `SEFEL_DB_NAME`, `SEFEL_DB_USER`, `SEFEL_DB_PASSWORD` (valores predeterminados: `localhost`, `5432`, `sefel`, `postgres`, contraseña vacía).
4. Compilar: `cmake -S . -B build -DCMAKE_PREFIX_PATH=<ruta-a-Qt>` y `cmake --build build`.
5. Ejecutar `sefel-desktop`.

Usuarios de demostración (contraseña: `Demo123!`):

| Correo | Rol |
|---|---|
| presupuestos@sefel.local | Encargado de Presupuestos |
| jefe@sefel.local | Jefe de Proyecto |
| gerencia@sefel.local | Gerencia General |

La contraseña de cada usuario se guarda con `crypt()` de `pgcrypto`. El usuario PostgreSQL de la aplicación requiere acceso a las tablas y a `crypt`; para un despliegue real se recomienda crear un rol SQL de mínimo privilegio y cambiar las credenciales de demostración.

## Decisiones del recorte

- Interfaz de escritorio Qt Widgets; acceso directo a PostgreSQL mediante Qt SQL.
- Roles de negocio del informe: Encargado (gestiona recursos, precios y APU/presupuestos), Jefe de Proyecto y Gerencia (acceso de lectura al MVP). La autorización de interfaz es una ayuda; se debe complementar con permisos PostgreSQL por usuario/rol para un despliegue multiusuario.
- Los precios ficticios de `seed.sql` son solo demostración.
- El presupuesto conserva el costo unitario neto como copia al momento de guardarlo. La utilidad inicial es 20% y se puede editar.
- Exportación CSV UTF-8 compatible con Excel, evitando dependencias externas.
- Los códigos de recurso son automáticos y siguen `PREFIJO-NNNN`: `MAT`, `MO`, `EQP` y `HER` por tipo. El precio mínimo permitido es $0,10.
- Los recursos se retiran del catálogo activo sin borrar historial ni presupuestos. Se pueden quitar líneas del APU en edición y eliminar presupuestos guardados; el APU se elimina solo si deja de tener referencias.
- **Cerrar sesión** vuelve a mostrar el login. La matriz de casos funcionales y de seguridad está en `docs/pruebas_funcionales_mvp.md`.
- Interfaz con paleta rojo, negro, gris y blanco aplicada al login, navegación, formularios, tablas y botones principales.
