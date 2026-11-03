import { createFileRoute } from "@tanstack/react-router";
import { useState } from "react";
import { toast } from "sonner";
import { FileSpreadsheet } from "lucide-react";
import { PageHeader } from "@/components/PageHeader";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Checkbox } from "@/components/ui/checkbox";
import { Progress } from "@/components/ui/progress";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Dialog, DialogContent, DialogHeader, DialogTitle } from "@/components/ui/dialog";
import { clp, costoUnitarioApu, useStore } from "@/lib/store";
import { exportarPresupuesto } from "@/lib/excel";

export const Route = createFileRoute("/_panel/apu")({
  head: () => ({ meta: [
    { title: "APU registradas — Sistema de Presupuestos" },
    { name: "description", content: "Generación de presupuestos en Excel a partir de APU registradas." },
    { property: "og:title", content: "APU registradas — Sistema de Presupuestos" },
    { property: "og:description", content: "Generación de presupuestos a partir de APU." },
  ] }),
  component: ApuPage,
});

function ApuPage() {
  const { apus, recursos } = useStore();
  const [sel, setSel] = useState<Record<string, number>>({});
  const [nombre, setNombre] = useState("Presupuesto obra");
  const [progreso, setProgreso] = useState<number | null>(null);

  const elegidas = apus.filter((a) => sel[a.id] !== undefined);
  const total = elegidas.reduce((s, a) => s + costoUnitarioApu(a, recursos) * (sel[a.id] ?? 0), 0);

  const generar = () => {
    if (!elegidas.length) { toast.error("Selecciona al menos una APU"); return; }
    if (elegidas.some((a) => !((sel[a.id] ?? 0) > 0))) { toast.error("Ingresa una cantidad mayor a 0 en cada APU seleccionada"); return; }
    setProgreso(0);
    let p = 0;
    const t = setInterval(() => {
      p += 8 + Math.random() * 12;
      if (p >= 100) {
        clearInterval(t);
        setProgreso(100);
        setTimeout(() => {
          exportarPresupuesto(nombre || "Presupuesto", elegidas.map((a) => ({ apu: a, cantidad: sel[a.id] ?? 0 })), recursos);
          setProgreso(null);
          toast.success("Presupuesto generado y descargado en Excel");
        }, 400);
      } else setProgreso(p);
    }, 200);
  };

  return (
    <div>
      <PageHeader hu="HU-05" title="APU registradas">
        <div className="flex items-end gap-3">
          <div className="space-y-1"><Label>Nombre del presupuesto</Label><Input value={nombre} onChange={(e) => setNombre(e.target.value)} /></div>
          <Button onClick={generar}><FileSpreadsheet size={16} /> Generar Presupuesto</Button>
        </div>
      </PageHeader>
      <div className="grid gap-4 md:grid-cols-2">
        {apus.map((a) => {
          const on = sel[a.id] !== undefined;
          return (
            <Card key={a.id} className={on ? "border-2 border-primary" : ""}>
              <CardHeader className="flex-row items-start gap-3 space-y-0">
                <Checkbox checked={on} onCheckedChange={() => setSel((s) => {
                  const n = { ...s }; if (on) delete n[a.id]; else n[a.id] = 1; return n; })} />
                <div className="flex-1">
                  <span className="font-mono text-xs text-primary">{a.codigo}</span>
                  <CardTitle className="font-display text-xl uppercase">{a.nombre}</CardTitle>
                </div>
                <div className="text-right"><p className="text-xs text-muted-foreground">Costo unitario</p><p className="font-bold">{clp(costoUnitarioApu(a, recursos))} / {a.unidad}</p></div>
              </CardHeader>
              <CardContent>
                <p className="text-sm text-muted-foreground">{a.items.length} recursos: {a.items.map((i) => recursos.find((r) => r.id === i.recursoId)?.nombre).filter(Boolean).join(", ")}</p>
                {on && (
                  <div className="mt-3 flex items-center gap-2">
                    <Label>Cantidad ({a.unidad})</Label>
                    <Input type="number" min={0} className="w-32" value={sel[a.id] || ""} onChange={(e) => setSel({ ...sel, [a.id]: Number(e.target.value) })} />
                  </div>
                )}
              </CardContent>
            </Card>
          );
        })}
      </div>
      {elegidas.length > 0 && (
        <div className="mt-6 flex justify-end bg-ink p-4 text-ink-foreground">
          <span>{elegidas.length} APU seleccionadas · Costo directo estimado: <b className="text-lg">{clp(total)}</b></span>
        </div>
      )}
      <Dialog open={progreso !== null}>
        <DialogContent className="[&>button]:hidden">
          <DialogHeader><DialogTitle className="font-display uppercase">Generando presupuesto…</DialogTitle></DialogHeader>
          <Progress value={progreso ?? 0} />
          <p className="text-right text-2xl font-bold">{Math.round(progreso ?? 0)}%</p>
        </DialogContent>
      </Dialog>
    </div>
  );
}
