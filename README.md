# Sistema de Gestión Integral de Presupuestos y Costos – SEFEL

Proyecto Integrado (INACAP, sección TIHI43). Equipo: Roberto Gutiérrez, Tomás Cárdenas, Rodrigo Muñoz, Gabriel Garrido.

## Estructura
| Carpeta | Contenido |
|---|---|
| `frontend/` | Prototipo interactivo en React (TanStack Start + Tailwind + shadcn/ui): login por rol, recursos, precios, APU, costos e indicadores |
| `backend/` | API REST, RBAC y reglas de negocio (`src/`) |
| `database/` | Esquema y datos de prueba (SQLite y PostgreSQL) y `crear_bd.py` |
| `tests/` | Pruebas unitarias, de integración y de rendimiento |
| `docs/` | Documentación del proyecto |

## Base de datos (ACT-02)
```bash
python database/crear_bd.py     # crea database/sefel.db y verifica las restricciones
```
Requiere solo Python 3. `sefel.db` no se sube a Git (se regenera con el script).
`schema.postgres.sql` / `seed.postgres.sql` son la versión para PostgreSQL (producción).

## Prototipo (frontend)
```bash
cd frontend
npm install        # o: bun install
npm run dev        # abre http://localhost:3000 (o el puerto que indique la consola)
npm test           # pruebas (vitest)
```
Usuarios de prueba (clave `demo123`): `presupuestos`, `jefe`, `gerencia`.
Los datos y la sesión se guardan en el `localStorage` del navegador (no hay backend todavía).
Nota: el proyecto fue generado con Lovable; no reescribir el historial de Git (sin `--force`, `rebase` ni `--amend` sobre commits ya subidos).

## Configuración
Copiar `.env.example` a `.env` (nunca subir `.env`).

## Flujo de trabajo
Ver [CONTRIBUTING.md](CONTRIBUTING.md).
