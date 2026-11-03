import { createFileRoute } from "@tanstack/react-router";
import { useState } from "react";
import { toast } from "sonner";
import { Search } from "lucide-react";
import { PageHeader } from "@/components/PageHeader";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Badge } from "@/components/ui/badge";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { EstadoBadge } from "@/components/shared";
import { clp, hoy, resumenProyecto, useStore } from "@/lib/store";

export const Route = createFileRoute("/_panel/proyectos")({
  head: () => ({ meta: [
    { title: "Costos de proyecto — Sistema de Presupuestos" },
    { name: "description", content: "Registro de costos reales y comparación con lo presupuestado." },
    { property: "og:title", content: "Costos de proyecto — Sistema de Presupuestos" },
    { property: "og:description", content: "Costos reales, desviaciones y estado financiero por proyecto." },
  ] }),
  component: ProyectosPage,
});

function ProyectosPage() {
  const { proyectos, apus, recursos, addCosto } = useStore();
  const [pid, setPid] = useState(proyectos[0]?.id ?? "");
  const [consulta, setConsulta] = useState(false);
  const p = proyectos.find((x) => x.id === pid);
  const [f, setF] = useState({ tipo: "Compra" as "Compra" | "Ejecución", descripcion: "", apuId: "", monto: "", fecha: hoy() });

  if (!p) return null;
  const res = resumenProyecto(p, apus, recursos);
  const partidasApu = p.partidas.map((pa) => apus.find((a) => a.id === pa.apuId)!).filter(Boolean);

  const registrar = (e: React.FormEvent) => {
    e.preventDefault();
    if (!f.descripcion || !f.apuId || !(Number(f.monto) > 0)) { toast.error("Completa descripción, partida y monto"); return; }
    addCosto(p.id, { ...f, monto: Number(f.monto) });
    toast.success("Costo real registrado. Costo del proyecto actualizado.");
    setF({ ...f, descripcion: "", monto: "" });
  };

  return (
    <div className="space-y-6">
      <PageHeader hu="HU-07 · HU-08 · HU-09" title="Costos de proyecto">
        <div className="w-72 space-y-1"><Label>Proyecto registrado</Label>
          <Select value={pid} onValueChange={(v) => { setPid(v); setConsulta(false); setF((s) => ({ ...s, apuId: "" })); }}>
            <SelectTrigger><SelectValue /></SelectTrigger>
            <SelectContent>{proyectos.map((x) => <SelectItem key={x.id} value={x.id}>{x.nombre}</SelectItem>)}</SelectContent>
          </Select></div>
      </PageHeader>

      <div className="grid gap-4 md:grid-cols-4">
        <Stat label="Cliente" value={p.cliente} sub={p.ubicacion} />
        <Stat label="Presupuesto total" value={clp(res.presupuestado)} sub={`Avance físico ${p.avance}%`} />
        <Stat label="Presupuestado al avance" value={clp(res.presupuestadoAvance)} />
        <Stat label="Costo real acumulado" value={clp(res.real)} highlight />
      </div>

      <div className="grid gap-6 xl:grid-cols-[360px_1fr]">
        <Card>
          <CardHeader><CardTitle className="font-display uppercase">Registrar costo real</CardTitle></CardHeader>
          <CardContent>
            <form onSubmit={registrar} className="space-y-3">
              <div className="grid grid-cols-2 gap-3">
                <div className="space-y-1"><Label>Tipo</Label>
                  <Select value={f.tipo} onValueChange={(v) => setF({ ...f, tipo: v as typeof f.tipo })}>
                    <SelectTrigger><SelectValue /></SelectTrigger>
                    <SelectContent><SelectItem value="Compra">Compra</SelectItem><SelectItem value="Ejecución">Ejecución</SelectItem></SelectContent>
                  </Select></div>
                <div className="space-y-1"><Label>Fecha</Label><Input type="date" value={f.fecha} onChange={(e) => setF({ ...f, fecha: e.target.value })} /></div>
              </div>
              <div className="space-y-1"><Label>Partida</Label>
                <Select value={f.apuId} onValueChange={(v) => setF({ ...f, apuId: v })}>
                  <SelectTrigger><SelectValue placeholder="Seleccionar partida" /></SelectTrigger>
                  <SelectContent>{partidasApu.map((a) => <SelectItem key={a.id} value={a.id}>{a.codigo} · {a.nombre}</SelectItem>)}</SelectContent>
                </Select></div>
              <div className="space-y-1"><Label>Descripción</Label><Input value={f.descripcion} onChange={(e) => setF({ ...f, descripcion: e.target.value })} /></div>
              <div className="space-y-1"><Label>Monto (CLP)</Label><Input type="number" min={0} value={f.monto} onChange={(e) => setF({ ...f, monto: e.target.value })} /></div>
              <Button type="submit" className="w-full">Registrar costo</Button>
            </form>
            <div className="mt-5 max-h-64 space-y-2 overflow-y-auto">
              {[...p.costos].reverse().map((c) => (
                <div key={c.id} className="flex justify-between border-l-4 border-ink-muted bg-muted px-3 py-2 text-sm">
                  <div><p className="font-medium">{c.descripcion}</p><p className="text-xs text-muted-foreground">{c.tipo} · {c.fecha}</p></div>
                  <b>{clp(c.monto)}</b>
                </div>
              ))}
            </div>
          </CardContent>
        </Card>

        <div className="space-y-6">
          <Card>
            <CardHeader><CardTitle className="font-display uppercase">Presupuestado vs. real por partida</CardTitle></CardHeader>
            <CardContent>
              <Table>
                <TableHeader><TableRow>
                  <TableHead>Partida</TableHead><TableHead className="text-right">Presupuesto</TableHead>
                  <TableHead className="text-right">Esperado al {p.avance}%</TableHead><TableHead className="text-right">Real</TableHead><TableHead className="text-right">Desviación</TableHead>
                </TableRow></TableHeader>
                <TableBody>
                  {res.filas.map((r) => (
                    <TableRow key={r.apu.id}>
                      <TableCell><span className="font-mono text-xs">{r.apu.codigo}</span> {r.apu.nombre}</TableCell>
                      <TableCell className="text-right">{clp(r.presupuestado)}</TableCell>
                      <TableCell className="text-right">{clp(r.presupuestadoAvance)}</TableCell>
                      <TableCell className="text-right">{clp(r.real)}</TableCell>
                      <TableCell className={`text-right font-semibold ${r.desviacion > 0 ? "text-primary" : "text-success"}`}>
                        {r.desviacion > 0 ? "+" : ""}{clp(r.desviacion)} ({r.pct.toFixed(1)}%)
                      </TableCell>
                    </TableRow>
                  ))}
                </TableBody>
              </Table>
            </CardContent>
          </Card>

          <Card>
            <CardHeader className="flex-row items-center justify-between space-y-0">
              <CardTitle className="font-display uppercase">Estado de costos</CardTitle>
              <Button onClick={() => { setConsulta(true); toast.info("Estado de costos consultado"); }}><Search size={16} /> Consultar</Button>
            </CardHeader>
            <CardContent>
              {consulta ? (
                <div className="grid gap-4 md:grid-cols-3">
                  <div className="space-y-1"><p className="text-sm text-muted-foreground">Estado</p><EstadoBadge estado={res.estado} /></div>
                  <div><p className="text-sm text-muted-foreground">Desviación total</p>
                    <p className={`text-2xl font-bold ${res.desviacion > 0 ? "text-primary" : "text-success"}`}>{res.desviacion > 0 ? "+" : ""}{clp(res.desviacion)}</p></div>
                  <div><p className="text-sm text-muted-foreground">Desviación %</p><p className="text-2xl font-bold">{res.pct.toFixed(1)}%</p></div>
                  <p className="text-sm md:col-span-3">
                    {res.filas.filter((r) => r.desviacion > 0).length
                      ? <>Partidas con sobrecosto: <b>{res.filas.filter((r) => r.desviacion > 0).map((r) => r.apu.nombre).join(", ")}</b>.</>
                      : "Ninguna partida presenta sobrecosto."}
                    {" "}Saldo disponible del presupuesto: <b>{clp(res.presupuestado - res.real)}</b>.
                  </p>
                </div>
              ) : <p className="text-sm text-muted-foreground">Presiona «Consultar» para ver la situación financiera del proyecto.</p>}
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
}

function Stat({ label, value, sub, highlight }: { label: string; value: string; sub?: string; highlight?: boolean }) {
  return (
    <div className={`border-t-4 p-4 ${highlight ? "border-primary bg-ink text-ink-foreground" : "border-ink bg-card"}`}>
      <p className={`text-xs uppercase tracking-wider ${highlight ? "text-ink-muted" : "text-muted-foreground"}`}>{label}</p>
      <p className="font-display text-2xl font-bold">{value}</p>
      {sub && <p className={`text-xs ${highlight ? "text-ink-muted" : "text-muted-foreground"}`}>{sub}</p>}
    </div>
  );
}
