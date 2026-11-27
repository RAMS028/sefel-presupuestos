# Pruebas funcionales y de seguridad — SEFEL Desktop MVP

**Versión:** 1.0.3  
**Alcance:** login por roles, recursos HU-01/02, precios e historial HU-03/04 y APU/presupuesto HU-05/06.  
**Nota:** completar los campos de resultado y evidencia durante la ejecución. Este documento no declara pruebas como aprobadas antes de ejecutarlas.

## Códigos de recursos

El código se genera automáticamente y no se edita manualmente. El formato es `PREFIJO-NNNN`, con secuencia independiente por tipo:

| Tipo | Prefijo | Ejemplo |
|---|---|---|
| MATERIAL | MAT | MAT-0001 |
| MANO_OBRA | MO | MO-0001 |
| EQUIPO | EQP | EQP-0001 |
| HERRAMIENTA | HER | HER-0001 |

La aplicación asigna el siguiente correlativo disponible para ese tipo. El precio mínimo permitido es **$0,10**; cero y cualquier valor menor al mínimo deben rechazarse.

## Prueba-Funcional-001 — Registrar recurso

- **Objetivo:** Comprobar que el Encargado de Presupuestos puede registrar un recurso.
- **Precondiciones:** Base de datos cargada; sesión iniciada como `ENCARGADO_PRESUPUESTOS`.
- **Procedimiento:**
  1. Abrir **Recursos**.
  2. Confirmar que el código se muestra automáticamente con formato, por ejemplo `MAT-0001`.
  3. Ingresar descripción y unidad.
  4. Seleccionar tipo (Material, Mano de obra, Equipo o Herramienta).
  5. Ingresar precio vigente `$0,10` y comprobar que se acepta; verificar que el control no permita `0,00`.
  6. Pulsar **Registrar recurso**.
- **Resultado esperado:** Se guarda el recurso con código único/prefijo correspondiente, precio de al menos `$0,10` y registro inicial en historial. `0,00` no es aceptado. La aplicación informa el código asignado.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-002 — Buscar por código o descripción

- **Objetivo:** Comprobar el filtro del catálogo por código o descripción.
- **Precondiciones:** Sesión iniciada como Encargado, Jefe de Proyecto o Gerencia; existe al menos un recurso.
- **Procedimiento:**
  1. Abrir **Recursos**.
  2. Escribir parte de un código estandarizado o de una descripción en la búsqueda.
  3. Pulsar **Buscar** (o Enter).
  4. Repetir con el otro criterio.
- **Resultado esperado:** Se muestran los recursos coincidentes; la búsqueda no distingue mayúsculas/minúsculas. Al dejar el campo vacío se muestra el catálogo completo.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-003 — Actualizar precio y consultar historial

- **Objetivo:** Verificar que se actualiza el precio vigente y que el cambio queda trazado.
- **Precondiciones:** Sesión iniciada como Encargado; existe un recurso con un registro de historial.
- **Procedimiento:**
  1. Abrir **Precios e historial**.
  2. Seleccionar un recurso.
  3. Ingresar un nuevo precio, por ejemplo `$0,10`; comprobar que no se pueda dejar en `0,00`.
  4. Pulsar **Guardar precio**.
  5. Pulsar **Ver historial**.
- **Resultado esperado:** El precio vigente refleja el nuevo valor (mínimo `$0,10`). El historial muestra valores anterior y nuevo, fecha, usuario y origen MANUAL.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-004 — Crear APU y presupuesto

- **Objetivo:** Comprobar el cálculo, guardado y exportación del presupuesto.
- **Precondiciones:** Sesión iniciada como Encargado; existe al menos un recurso.
- **Procedimiento:**
  1. Abrir **APU y presupuesto**.
  2. Ingresar nombre del presupuesto y actividad APU; revisar la unidad.
  3. Seleccionar recurso, ingresar cantidad por unidad y pulsar **Agregar recurso al APU**.
  4. Ingresar cantidad presupuestada y porcentaje de utilidad.
  5. Pulsar **Calcular** y comprobar el total: costo unitario neto × cantidad × (1 + utilidad/100).
  6. Pulsar **Guardar presupuesto**.
  7. Opcionalmente pulsar **Exportar CSV** y abrir el archivo.
- **Resultado esperado:** El total mostrado coincide con el cálculo; el presupuesto, APU, detalle y partida se guardan; la exportación incluye las líneas y totales.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-005 — Consultar recursos como Gerencia

- **Objetivo:** Confirmar que Gerencia puede buscar y revisar el catálogo.
- **Precondiciones:** Sesión iniciada con rol `GERENCIA`; existen recursos.
- **Procedimiento:** Abrir **Recursos**, buscar por código y luego por descripción.
- **Resultado esperado:** La tabla filtra los recursos correctamente. El registro de recursos está deshabilitado y las pestañas de precios e APU/presupuesto no son accesibles.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-006 — Consultar recursos como Jefe de Proyecto

- **Objetivo:** Confirmar que Jefe de Proyecto puede buscar y revisar el catálogo.
- **Precondiciones:** Sesión iniciada con rol `JEFE_PROYECTO`; existen recursos.
- **Procedimiento:** Abrir **Recursos**, buscar por código y luego por descripción.
- **Resultado esperado:** La tabla filtra los recursos correctamente. El registro de recursos está deshabilitado y las pestañas de precios e APU/presupuesto no son accesibles.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Seguridad-001 — Acceso a precios e historial por rol

- **Objetivo:** Confirmar que Jefe de Proyecto y Gerencia no pueden modificar precios ni consultar el historial desde el MVP.
- **Precondiciones:** Sesiones válidas para cada uno de esos roles.
- **Procedimiento:** En cada sesión, intentar seleccionar la pestaña **Precios e historial** desde Recursos.
- **Resultado esperado:** La pestaña permanece deshabilitada y no permite acceder a sus controles.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Seguridad-002 — Acceso a APU y presupuesto por rol

- **Objetivo:** Confirmar que Jefe de Proyecto y Gerencia no pueden crear APU ni presupuestos en este MVP.
- **Precondiciones:** Sesiones válidas para cada uno de esos roles.
- **Procedimiento:** En cada sesión, intentar seleccionar la pestaña **APU y presupuesto** desde Recursos.
- **Resultado esperado:** La pestaña permanece deshabilitada y no permite acceder a sus controles.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Seguridad-003 — Cerrar sesión

- **Objetivo:** Confirmar que la sesión actual termina y vuelve al login.
- **Precondiciones:** Sesión iniciada con cualquier rol.
- **Procedimiento:** Pulsar **Cerrar sesión**.
- **Resultado esperado:** Se cierra la ventana de negocio y vuelve la ventana de inicio de sesión. Una sesión nueva requiere autenticación.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-007 — Retirar recurso del catálogo

- **Objetivo:** Retirar un recurso sin romper el historial de precios ni presupuestos existentes.
- **Precondiciones:** Sesión iniciada como Encargado de Presupuestos; existe un recurso seleccionado.
- **Procedimiento:** En **Recursos**, seleccionar la fila, pulsar **Retirar recurso del catálogo** y confirmar.
- **Resultado esperado:** El recurso deja de aparecer en el catálogo activo y en las listas para nuevos APU. Sus registros de historial y presupuestos guardados se conservan.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-008 — Quitar una línea del APU en edición

- **Objetivo:** Corregir la composición del APU antes de guardar el presupuesto.
- **Precondiciones:** Sesión iniciada como Encargado; se agregó al menos un recurso al APU en edición.
- **Procedimiento:** Seleccionar una línea y pulsar **Quitar recurso seleccionado del APU**.
- **Resultado esperado:** La línea se quita del APU en edición y se recalculan el costo unitario y el total. El recurso sigue existiendo en el catálogo.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada

## Prueba-Funcional-009 — Eliminar presupuesto y APU guardados

- **Objetivo:** Eliminar un presupuesto guardado y limpiar su APU cuando ya no tenga referencias.
- **Precondiciones:** Sesión iniciada como Encargado; existe al menos un presupuesto guardado.
- **Procedimiento:** En **Presupuestos guardados**, seleccionar una fila, pulsar **Eliminar presupuesto y su APU** y confirmar.
- **Resultado esperado:** Se elimina el presupuesto y sus partidas. Su APU se elimina solo si no está referenciado por otra partida. Los recursos e historiales no se borran.
- **Resultado obtenido:** ____________________
- **Evidencia:** ____________________
- **Estado:** Pendiente / Aprobada / Rechazada
