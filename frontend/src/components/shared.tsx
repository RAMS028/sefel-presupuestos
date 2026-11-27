import { Badge } from "@/components/ui/badge";
import { Dialog, DialogContent, DialogHeader, DialogTitle } from "@/components/ui/dialog";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { clp, type Recurso } from "@/lib/store";

export function EstadoBadge({ estado }: { estado: string }) {
  const cls = estado === "Sobrecosto" ? "bg-primary text-primary-foreground" : estado === "En alerta" ? "bg-warning text-warning-foreground" : "bg-success text-success-foreground";
  return <Badge className={cls}>{estado}</Badge>;
}

export function HistorialDialog({ open, onOpenChange, recursos }: { open: boolean; onOpenChange: (v: boolean) => void; recursos: Recurso[] }) {
  return (
    <Dialog open={open} onOpenChange={onOpenChange}>
      <DialogContent className="max-h-[80vh] max-w-2xl overflow-y-auto">
        <DialogHeader><DialogTitle className="font-display uppercase">Historial de precios</DialogTitle></DialogHeader>
        <div className="space-y-5">
          {recursos.map((x) => (
            <div key={x.id}>
              <p className="mb-1 font-semibold">{x.codigo} · {x.nombre}</p>
              <Table>
                <TableHeader><TableRow><TableHead>Fecha</TableHead><TableHead className="text-right">Precio</TableHead><TableHead className="text-right">Variación</TableHead></TableRow></TableHeader>
                <TableBody>
                  {x.historial.map((h, i) => {
                    const prev = x.historial[i - 1]?.precio;
                    const v = prev ? ((h.precio - prev) / prev) * 100 : 0;
                    return (
                      <TableRow key={i}>
                        <TableCell>{new Date(h.fecha + "T12:00").toLocaleDateString("es-CL")}</TableCell>
                        <TableCell className="text-right">{clp(h.precio)}</TableCell>
                        <TableCell className={`text-right ${v > 0 ? "text-primary" : v < 0 ? "text-success" : "text-muted-foreground"}`}>{prev ? `${v > 0 ? "+" : ""}${v.toFixed(1)}%` : "—"}</TableCell>
                      </TableRow>
                    );
                  })}
                </TableBody>
              </Table>
            </div>
          ))}
        </div>
      </DialogContent>
    </Dialog>
  );
}
