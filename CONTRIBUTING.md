# Flujo de trabajo con Git

- `main`: versión estable (solo se actualiza al cierre de cada Sprint).
- `develop`: integración del Sprint.
- `feature/HU-NN-descripcion`: una rama por historia o actividad (ej. `feature/HU-01-registro-recursos`, `feature/ACT-04-login`).

## Reglas
1. Crear la rama desde `develop`: `git switch develop && git pull && git switch -c feature/HU-01-registro-recursos`.
2. Commits pequeños y con mensaje claro: `HU-01: valida campos obligatorios`.
3. Subir y abrir un Pull Request hacia `develop`; debe aprobarlo **otro integrante** (revisión de código).
4. Antes de integrar: ejecutar las pruebas y no subir secretos (`.env`, claves, `sefel.db`).
5. Al cierre del Sprint (Review aceptada): `develop` → `main` con una etiqueta (`git tag sprint-1`).

## Comandos útiles
```bash
git clone <URL-del-repositorio>
git switch -c feature/ACT-04-login develop
git add . && git commit -m "ACT-04: login con roles"
git push -u origin feature/ACT-04-login
```
