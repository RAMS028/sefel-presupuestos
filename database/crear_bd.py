"""Crea database/sefel.db desde cero y ejecuta verificaciones de integridad (ACT-02).
Uso:  python database/crear_bd.py        (requiere solo Python 3)"""
import sqlite3, pathlib, sys
D = pathlib.Path(__file__).parent
DB = D / "sefel.db"
if DB.exists():
    DB.unlink()
c = sqlite3.connect(DB)
c.execute("PRAGMA foreign_keys = ON")
c.executescript((D / "schema.sqlite.sql").read_text(encoding="utf-8"))
c.executescript((D / "seed.sqlite.sql").read_text(encoding="utf-8"))
c.commit()

def debe_fallar(nombre, sql):
    try:
        c.execute(sql); c.commit()
    except sqlite3.Error as e:
        print(f"  OK    {nombre} -> rechazado ({e})"); return True
    print(f"  FALLA {nombre} -> NO fue rechazado"); return False

print("Tablas:", ", ".join(r[0] for r in c.execute("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY 1")))
print("Recursos cargados:", c.execute("SELECT COUNT(*) FROM recurso").fetchone()[0])
print("Verificaciones:")
ok = all([
 debe_fallar("precio negativo", "INSERT INTO recurso(codigo,descripcion,unidad,tipo,precio_vigente) VALUES('X1','x','u','MATERIAL',-1)"),
 debe_fallar("codigo duplicado (RNF-05)", "INSERT INTO recurso(codigo,descripcion,unidad,tipo,precio_vigente) VALUES('MAT-001','x','u','MATERIAL',1)"),
 debe_fallar("UPDATE al historial (RNF-03)", "UPDATE historial_precio SET precio_nuevo=1 WHERE id_historial=1"),
 debe_fallar("DELETE al historial (RNF-03)", "DELETE FROM historial_precio WHERE id_historial=1"),
 debe_fallar("costo directo sin partida", "INSERT INTO costo_real(id_proyecto,id_usuario,tipo,categoria,estado,monto) VALUES(1,2,'COMPRA','DIRECTO','EJECUTADO',100)"),
 debe_fallar("FK inexistente", "INSERT INTO apu_detalle(id_apu,id_recurso,cantidad) VALUES(1,99999,1)"),
])
c.execute("INSERT INTO costo_real(id_proyecto,id_usuario,tipo,categoria,estado,monto) VALUES(1,2,'COMPRA','INDIRECTO','COMPROMETIDO',120000)")
c.commit(); print("  OK    costo indirecto sin partida -> aceptado (HU-07)")
print("RESULTADO:", "TODO OK" if ok else "HAY FALLAS"); sys.exit(0 if ok else 1)
