import { createFileRoute } from "@tanstack/react-router";
import { useState } from "react";
import { toast } from "sonner";
import { History } from "lucide-react";
import { PageHeader } from "@/components/PageHeader";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Card, CardContent } from "@/components/ui/card";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { clp, useStore, type Recurso } from "@/lib/store";
import { HistorialDialog } from "@/components/shared";

export const Route = createFileRoute("/_panel/precios")({
  head: () => ({ meta: [
    { title: "Actualizar precios — Sistema de Presupuestos" },
    { name: "description", content: "Modificación de precios actuales de los recursos." },
    { property: "og:title", content: "Actualizar precios — Sistema de Presupuestos" },
    { property: "og:description", content: "Modificación de precios actuales de los recursos." },
  ] }),
  component: PreciosPage,
});

function PreciosPage() {
  const { recursos, actualizarPrecios } = useStore();
  const [edit, setEdit] = useState<Record<string, string>>({});
  const [hist, setHist] = useState<Recurso[]>([]);

  const cambios = Object.fromEntries(Object.entries(edit)
    .filter(([id, v]) => v !== "" && Number(v) > 0 && Number(v) !== recursos.find((x) => x.id === id)?.precio)
    .map(([id, v]) => [id, Number(v)]));
  const n = Object.keys(cambios).length;

  const actualizar = () => {
    if (!n) { toast.error("No hay precios modificados para actualizar"); return; }
    actualizarPrecios(cambios);
    setEdit({});
    toast.success(`Se actualizaron correctamente ${n} precio(s)`);
  };

  return (
    <div>
      <PageHeader hu="HU-03" title="Actualizar precios">
        <Button onClick={actualizar} disabled={!n}>Actualizar {n > 0 && `(${n})`}</Button>
      </PageHeader>
      <Card><CardContent className="pt-6">
        <Table>
          <TableHeader><TableRow>
            <TableHead>Código</TableHead><TableHead>Recurso</TableHead><TableHead>Unidad</TableHead>
            <TableHead className="text-right">Precio actual</TableHead><TableHead className="w-48">Nuevo precio</TableHead><TableHead />
          </TableRow></TableHeader>
          <TableBody>
            {recursos.map((x) => (
              <TableRow key={x.id} className={cambios[x.id] ? "bg-accent" : ""}>
                <TableCell className="whitespace-nowrap font-mono text-xs">{x.codigo}</TableCell>
                <TableCell>{x.nombre}</TableCell>
                <TableCell>{x.unidad}</TableCell>
                <TableCell className="text-right font-semibold">{clp(x.precio)}</TableCell>
                <TableCell><Input type="number" min={0} placeholder={String(x.precio)} value={edit[x.id] ?? ""}
                  onChange={(e) => setEdit({ ...edit, [x.id]: e.target.value })} /></TableCell>
                <TableCell><Button size="sm" variant="ghost" onClick={() => setHist([x])}><History size={16} /> Historial de precios</Button></TableCell>
              </TableRow>
            ))}
          </TableBody>
        </Table>
      </CardContent></Card>
      <HistorialDialog open={hist.length > 0} onOpenChange={(v) => !v && setHist([])} recursos={hist} />
    </div>
  );
}
