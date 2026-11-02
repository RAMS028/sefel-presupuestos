# Sistema de Gestión Integral de Presupuestos y Costos – SEFEL

Proyecto Integrado (INACAP, sección TIHI43). Equipo: Roberto Gutiérrez, Tomás Cárdenas, Rodrigo Muñoz, Gabriel Garrido.

## Estructura
| Carpeta | Contenido |
|---|---|
| `frontend/` | Interfaz web por rol (Sprint 1 en adelante) |
| `backend/` | API REST, RBAC y reglas de negocio (`src/`) |
| `database/` | Esquema y datos de prueba (SQLite y PostgreSQL) y `crear_bd.py` |
| `tests/` | Pruebas unitarias, de integración y de rendimiento |
| `docs/` | Prototipo interactivo (`prototipo_sefel.html`) |

## Base de datos (ACT-02)
```bash
python database/crear_bd.py     # crea database/sefel.db y verifica las restricciones
```
Requiere solo Python 3. `sefel.db` no se sube a Git (se regenera con el script).
`schema.postgres.sql` / `seed.postgres.sql` son la versión para PostgreSQL (producción).

## Configuración
Copiar `.env.example` a `.env` (nunca subir `.env`).

## Flujo de trabajo
Ver [CONTRIBUTING.md](CONTRIBUTING.md).
