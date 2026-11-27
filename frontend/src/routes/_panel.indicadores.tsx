import { createFileRoute } from "@tanstack/react-router";
import { PageHeader } from "@/components/PageHeader";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Progress } from "@/components/ui/progress";
import { clp, resumenProyecto, useStore } from "@/lib/store";
import { EstadoBadge } from "@/components/shared";

export const Route = createFileRoute("/_panel/indicadores")({
  head: () => ({ meta: [
    { title: "Proyectos e indicadores — Sistema de Presupuestos" },
    { name: "description", content: "Indicadores de costos de todos los proyectos para gerencia." },
    { property: "og:title", content: "Proyectos e indicadores — Sistema de Presupuestos" },
    { property: "og:description", content: "Indicadores de costos de todos los proyectos." },
  ] }),
  component: IndicadoresPage,
});

function IndicadoresPage() {
  const { proyectos, apus, recursos } = useStore();
  const datos = proyectos.map((p) => ({ p, r: resumenProyecto(p, apus, recursos) }));
  const totP = datos.reduce((s, d) => s + d.r.presupuestado, 0);
  const totR = datos.reduce((s, d) => s + d.r.real, 0);

  return (
    <div>
      <PageHeader hu="HU-10" title="Proyectos" />
      <div className="mb-6 grid gap-4 md:grid-cols-3">
        <div className="border-t-4 border-ink bg-card p-4"><p className="text-xs uppercase text-muted-foreground">Proyectos activos</p><p className="font-display text-3xl font-bold">{proyectos.length}</p></div>
        <div className="border-t-4 border-ink bg-card p-4"><p className="text-xs uppercase text-muted-foreground">Presupuesto cartera</p><p className="font-display text-3xl font-bold">{clp(totP)}</p></div>
        <div className="border-t-4 border-primary bg-ink p-4 text-ink-foreground"><p className="text-xs uppercase text-ink-muted">Costo real cartera</p><p className="font-display text-3xl font-bold">{clp(totR)}</p></div>
      </div>
      <div className="grid gap-5 lg:grid-cols-3">
        {datos.map(({ p, r }) => {
          const cpi = r.real ? r.presupuestadoAvance / r.real : 1;
          return (
            <Card key={p.id}>
              <CardHeader>
                <div className="flex items-start justify-between gap-2">
                  <CardTitle className="font-display text-xl uppercase">{p.nombre}</CardTitle>
                  <EstadoBadge estado={r.estado} />
                </div>
                <p className="text-sm text-muted-foreground">{p.cliente} · {p.ubicacion}</p>
              </CardHeader>
              <CardContent className="space-y-3 text-sm">
                <div><div className="mb-1 flex justify-between"><span>Avance físico</span><b>{p.avance}%</b></div><Progress value={p.avance} /></div>
                <div><div className="mb-1 flex justify-between"><span>Presupuesto consumido</span><b>{((r.real / r.presupuestado) * 100).toFixed(1)}%</b></div><Progress value={Math.min(100, (r.real / r.presupuestado) * 100)} /></div>
                <Row k="Presupuesto" v={clp(r.presupuestado)} />
                <Row k="Costo real" v={clp(r.real)} />
                <Row k="Desviación" v={`${r.desviacion > 0 ? "+" : ""}${clp(r.desviacion)} (${r.pct.toFixed(1)}%)`} danger={r.desviacion > 0} />
                <Row k="Índice de costo (CPI)" v={cpi.toFixed(2)} danger={cpi < 1} />
              </CardContent>
            </Card>
          );
        })}
      </div>
    </div>
  );
}

function Row({ k, v, danger }: { k: string; v: string; danger?: boolean }) {
  return <div className="flex justify-between border-b border-dashed pb-1"><span className="text-muted-foreground">{k}</span><b className={danger ? "text-primary" : ""}>{v}</b></div>;
}
