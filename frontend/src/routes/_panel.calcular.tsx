import { createFileRoute } from "@tanstack/react-router";
import { useState } from "react";
import { toast } from "sonner";
import { Minus, Plus, Search, Trash2 } from "lucide-react";
import { PageHeader } from "@/components/PageHeader";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { clp, useStore } from "@/lib/store";
import { exportarPresupuesto } from "@/lib/excel";

export const Route = createFileRoute("/_panel/calcular")({
  head: () => ({ meta: [
    { title: "Calcular APU — Sistema de Presupuestos" },
    { name: "description", content: "Creación de APU y presupuestos seleccionando materiales y cantidades." },
    { property: "og:title", content: "Calcular APU — Sistema de Presupuestos" },
    { property: "og:description", content: "Creación de APU y presupuestos con exportación a Excel." },
  ] }),
  component: CalcularPage,
});

function CalcularPage() {
  const { recursos, addApu, apus } = useStore();
  const [q, setQ] = useState("");
  const [items, setItems] = useState<Record<string, number>>({});
  const [info, setInfo] = useState({ codigo: `APU-${String(apus.length + 1).padStart(2, "0")}`, nombre: "", unidad: "m²", cantidadObra: "1" });

  const filtrados = recursos.filter((r) => !q || (r.nombre + r.codigo + r.categoria).toLowerCase().includes(q.toLowerCase()));
  const lineas = Object.entries(items).map(([id, cant]) => ({ rec: recursos.find((r) => r.id === id)!, cant })).filter((l) => l.rec);
  const unitario = lineas.reduce((s, l) => s + l.rec.precio * l.cant, 0);
  const cantObra = Number(info.cantidadObra) || 0;

  const setCant = (id: string, v: number) => setItems((s) => ({ ...s, [id]: Math.max(0, Math.round(v * 100) / 100) }));
  const quitar = (id: string) => setItems((s) => { const n = { ...s }; delete n[id]; return n; });

  const crear = () => {
    if (!info.nombre || !info.codigo) { toast.error("Ingresa código y nombre de la APU"); return; }
    if (!lineas.length || lineas.some((l) => l.cant <= 0)) { toast.error("Agrega materiales con cantidades mayores a 0"); return; }
    if (cantObra <= 0) { toast.error("Ingresa la cantidad estimada para el proyecto"); return; }
    const apu = addApu({ codigo: info.codigo, nombre: info.nombre, unidad: info.unidad,
      items: lineas.map((l) => ({ recursoId: l.rec.id, cantidad: l.cant })) });
    exportarPresupuesto(`${apu.codigo} ${apu.nombre}`, [{ apu, cantidad: cantObra }], recursos);
    toast.success("APU creada y presupuesto exportado a Excel");
    setItems({});
    setInfo({ codigo: `APU-${String(apus.length + 2).padStart(2, "0")}`, nombre: "", unidad: "m²", cantidadObra: "1" });
  };

  return (
    <div>
      <PageHeader hu="HU-06" title="Calcular APU y presupuesto" />
      <div className="grid gap-6 xl:grid-cols-2">
        <Card>
          <CardHeader><CardTitle className="font-display uppercase">Materiales disponibles</CardTitle></CardHeader>
          <CardContent>
            <div className="relative mb-3">
              <Search className="absolute left-3 top-2.5 text-muted-foreground" size={16} />
              <Input className="pl-9" placeholder="Buscar material, mano de obra o equipo…" value={q} onChange={(e) => setQ(e.target.value)} />
            </div>
            <div className="max-h-[520px] divide-y overflow-y-auto border">
              {filtrados.map((r) => (
                <div key={r.id} className="flex items-center justify-between gap-3 p-3">
                  <div><p className="text-sm font-medium">{r.nombre}</p><p className="text-xs text-muted-foreground">{r.codigo} · {r.categoria} · {clp(r.precio)}/{r.unidad}</p></div>
                  <Button size="sm" variant={items[r.id] !== undefined ? "secondary" : "default"} onClick={() => setCant(r.id, (items[r.id] ?? 0) + 1)}><Plus size={14} /> Agregar</Button>
                </div>
              ))}
              {!filtrados.length && <p className="p-4 text-sm text-muted-foreground">Sin coincidencias.</p>}
            </div>
          </CardContent>
        </Card>

        <Card>
          <CardHeader><CardTitle className="font-display uppercase">Nueva APU</CardTitle></CardHeader>
          <CardContent className="space-y-4">
            <div className="grid grid-cols-[120px_1fr_90px] gap-3">
              <div className="space-y-1"><Label>Código</Label><Input value={info.codigo} onChange={(e) => setInfo({ ...info, codigo: e.target.value })} /></div>
              <div className="space-y-1"><Label>Nombre</Label><Input value={info.nombre} onChange={(e) => setInfo({ ...info, nombre: e.target.value })} placeholder="Ej: Radier e=10 cm" /></div>
              <div className="space-y-1"><Label>Unidad</Label><Input value={info.unidad} onChange={(e) => setInfo({ ...info, unidad: e.target.value })} /></div>
            </div>
            <Table>
              <TableHeader><TableRow><TableHead>Recurso</TableHead><TableHead className="w-44">Cantidad / {info.unidad}</TableHead><TableHead className="text-right">Subtotal</TableHead><TableHead /></TableRow></TableHeader>
              <TableBody>
                {lineas.map((l) => (
                  <TableRow key={l.rec.id}>
                    <TableCell className="text-sm">{l.rec.nombre}</TableCell>
                    <TableCell><div className="flex items-center gap-1">
                      <Button size="icon" variant="outline" className="h-8 w-8" onClick={() => setCant(l.rec.id, l.cant - 1)}><Minus size={14} /></Button>
                      <Input type="number" className="h-8 w-20" value={l.cant} onChange={(e) => setCant(l.rec.id, Number(e.target.value))} />
                      <Button size="icon" variant="outline" className="h-8 w-8" onClick={() => setCant(l.rec.id, l.cant + 1)}><Plus size={14} /></Button>
                    </div></TableCell>
                    <TableCell className="text-right">{clp(l.rec.precio * l.cant)}</TableCell>
                    <TableCell><Button size="icon" variant="ghost" onClick={() => quitar(l.rec.id)}><Trash2 size={16} /></Button></TableCell>
                  </TableRow>
                ))}
                {!lineas.length && <TableRow><TableCell colSpan={4} className="py-8 text-center text-muted-foreground">Agrega recursos desde el panel izquierdo.</TableCell></TableRow>}
              </TableBody>
            </Table>
            <div className="space-y-2 bg-ink p-4 text-ink-foreground">
              <div className="flex justify-between"><span>Costo unitario APU</span><b>{clp(unitario)} / {info.unidad}</b></div>
              <div className="flex items-center justify-between gap-3">
                <span>Cantidad estimada en proyecto ({info.unidad})</span>
                <Input type="number" className="w-28 bg-card text-card-foreground" value={info.cantidadObra} onChange={(e) => setInfo({ ...info, cantidadObra: e.target.value })} />
              </div>
              <div className="flex justify-between border-t border-ink-muted pt-2 text-lg"><span>Presupuesto</span><b className="text-primary">{clp(unitario * cantObra)}</b></div>
            </div>
            <Button className="w-full" onClick={crear}>Crear APU y presupuesto (Excel)</Button>
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
