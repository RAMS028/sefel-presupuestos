import * as XLSX from "xlsx";
import { costoUnitarioApu, type Apu, type Recurso } from "./store";

export function exportarPresupuesto(
  titulo: string,
  partidas: { apu: Apu; cantidad: number }[],
  recursos: Recurso[],
) {
  const wb = XLSX.utils.book_new();
  const resumen: (string | number)[][] = [
    ["CONSTRUCTORA — PRESUPUESTO"], [titulo], [`Fecha: ${new Date().toLocaleDateString("es-CL")}`], [],
    ["Código", "Partida", "Unidad", "Cantidad", "P. Unitario", "Total"],
  ];
  let total = 0;
  partidas.forEach(({ apu, cantidad }) => {
    const pu = Math.round(costoUnitarioApu(apu, recursos));
    total += pu * cantidad;
    resumen.push([apu.codigo, apu.nombre, apu.unidad, cantidad, pu, pu * cantidad]);
  });
  resumen.push([], ["", "", "", "", "Costo directo", total], ["", "", "", "", "IVA 19%", Math.round(total * 0.19)],
    ["", "", "", "", "TOTAL", Math.round(total * 1.19)]);
  const ws = XLSX.utils.aoa_to_sheet(resumen);
  ws["!cols"] = [{ wch: 10 }, { wch: 40 }, { wch: 8 }, { wch: 10 }, { wch: 14 }, { wch: 16 }];
  XLSX.utils.book_append_sheet(wb, ws, "Presupuesto");

  partidas.forEach(({ apu }) => {
    const rows: (string | number)[][] = [[`APU ${apu.codigo} — ${apu.nombre} (${apu.unidad})`], [],
      ["Código", "Recurso", "Unidad", "Cantidad", "Precio", "Subtotal"]];
    apu.items.forEach((it) => {
      const rec = recursos.find((x) => x.id === it.recursoId);
      if (rec) rows.push([rec.codigo, rec.nombre, rec.unidad, it.cantidad, rec.precio, Math.round(rec.precio * it.cantidad)]);
    });
    rows.push([], ["", "", "", "", "Costo unitario", Math.round(costoUnitarioApu(apu, recursos))]);
    const s = XLSX.utils.aoa_to_sheet(rows);
    s["!cols"] = [{ wch: 10 }, { wch: 36 }, { wch: 8 }, { wch: 10 }, { wch: 12 }, { wch: 14 }];
    XLSX.utils.book_append_sheet(wb, s, apu.codigo.slice(0, 31));
  });
  XLSX.writeFile(wb, `${titulo.replace(/[^\w-]+/g, "_")}.xlsx`);
}
