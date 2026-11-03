import { createFileRoute } from "@tanstack/react-router";
import { useState } from "react";
import { toast } from "sonner";
import { History } from "lucide-react";
import { PageHeader } from "@/components/PageHeader";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from "@/components/ui/select";
import { RadioGroup, RadioGroupItem } from "@/components/ui/radio-group";
import { Checkbox } from "@/components/ui/checkbox";
import { Badge } from "@/components/ui/badge";
import { Dialog, DialogContent, DialogHeader, DialogTitle } from "@/components/ui/dialog";
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from "@/components/ui/table";
import { clp, useStore, type Prioridad, type Recurso } from "@/lib/store";
import { HistorialDialog } from "@/components/shared";

export const Route = createFileRoute("/_panel/recursos")({
  head: () => ({ meta: [
    { title: "Recursos — Sistema de Presupuestos" },
    { name: "description", content: "Registro, filtro e historial de precios de recursos de obra." },
    { property: "og:title", content: "Recursos — Sistema de Presupuestos" },
    { property: "og:description", content: "Registro, filtro e historial de precios de recursos." },
  ] }),
  component: RecursosPage,
});

const ORDEN_P: Record<Prioridad, number> = { Alta: 0, Media: 1, Baja: 2 };
const vacio = { codigo: "", nombre: "", unidad: "", categoria: "Material", prioridad: "Media" as Prioridad, precio: "" };

function RecursosPage() {
  const { recursos, addRecurso } = useStore();
  const [f, setF] = useState(vacio);
  const [texto, setTexto] = useState("");
  const [tipo, setTipo] = useState<"alfabetico" | "prioridad">("alfabetico");
  const [resultado, setResultado] = useState<Recurso[] | null>(null);
  const [sel, setSel] = useState<string[]>([]);
  const [verHist, setVerHist] = useState(false);

  const registrar = (e: React.FormEvent) => {
    e.preventDefault();
    if (!f.codigo || !f.nombre || !f.unidad || !Number(f.precio)) { toast.error("Completa todos los campos con valores válidos"); return; }
    addRecurso({ ...f, precio: Number(f.precio) });
    toast.success(`Recurso "${f.nombre}" registrado correctamente`);
    setF(vacio);
    setResultado(null);
  };

  const filtrar = () => {
    const q = texto.trim().toLowerCase();
    const r = recursos.filter((x) => !q || [x.nombre, x.codigo, x.categoria].some((v) => v.toLowerCase().includes(q)));
    r.sort((a, b) => tipo === "alfabetico" ? a.nombre.localeCompare(b.nombre, "es")
      : ORDEN_P[a.prioridad] - ORDEN_P[b.prioridad] || a.nombre.localeCompare(b.nombre, "es"));
    setResultado(r);
    toast.info(`${r.length} resultado(s) encontrados`);
  };

  const lista = resultado ?? recursos;
  const toggle = (id: string) => setSel((s) => s.includes(id) ? s.filter((x) => x !== id) : [...s, id]);
  const seleccionados = recursos.filter((x) => sel.includes(x.id));

  return (
    <div>
      <PageHeader hu="HU-01 · HU-02 · HU-04" title="Recursos" />
      <div className="grid gap-6 xl:grid-cols-[380px_1fr]">
        <Card>
          <CardHeader><CardTitle className="font-display uppercase">Registrar nuevo recurso</CardTitle></CardHeader>
          <CardContent>
            <form onSubmit={registrar} className="space-y-3">
              <div className="grid grid-cols-2 gap-3">
                <div className="space-y-1"><Label>Código</Label><Input value={f.codigo} onChange={(e) => setF({ ...f, codigo: e.target.value })} placeholder="MAT-010" /></div>
                <div className="space-y-1"><Label>Unidad</Label><Input value={f.unidad} onChange={(e) => setF({ ...f, unidad: e.target.value })} placeholder="m³, un, kg" /></div>
              </div>
              <div className="space-y-1"><Label>Nombre</Label><Input value={f.nombre} onChange={(e) => setF({ ...f, nombre: e.target.value })} /></div>
              <div className="grid grid-cols-2 gap-3">
                <div className="space-y-1"><Label>Categoría</Label>
                  <Select value={f.categoria} onValueChange={(v) => setF({ ...f, categoria: v })}>
                    <SelectTrigger><SelectValue /></SelectTrigger>
                    <SelectContent>{["Material", "Mano de obra", "Equipo"].map((c) => <SelectItem key={c} value={c}>{c}</SelectItem>)}</SelectContent>
                  </Select></div>
                <div className="space-y-1"><Label>Prioridad</Label>
                  <Select value={f.prioridad} onValueChange={(v) => setF({ ...f, prioridad: v as Prioridad })}>
                    <SelectTrigger><SelectValue /></SelectTrigger>
                    <SelectContent>{["Alta", "Media", "Baja"].map((c) => <SelectItem key={c} value={c}>{c}</SelectItem>)}</SelectContent>
                  </Select></div>
              </div>
              <div className="space-y-1"><Label>Precio unitario (CLP)</Label><Input type="number" min={0} value={f.precio} onChange={(e) => setF({ ...f, precio: e.target.value })} /></div>
              <Button type="submit" className="w-full">Registrar</Button>
            </form>
          </CardContent>
        </Card>

        <Card>
          <CardHeader><CardTitle className="font-display uppercase">Consultar recursos registrados</CardTitle></CardHeader>
          <CardContent className="space-y-4">
            <div className="flex flex-wrap items-end gap-4 border bg-muted p-4">
              <div className="min-w-48 flex-1 space-y-1"><Label>Buscar</Label><Input value={texto} onChange={(e) => setTexto(e.target.value)} placeholder="Nombre, código o categoría" /></div>
              <div className="space-y-1"><Label>Tipo de filtro</Label>
                <RadioGroup value={tipo} onValueChange={(v) => setTipo(v as typeof tipo)} className="flex gap-4 py-2">
                  <label className="flex items-center gap-2 text-sm"><RadioGroupItem value="alfabetico" /> Alfabético</label>
                  <label className="flex items-center gap-2 text-sm"><RadioGroupItem value="prioridad" /> Prioridad</label>
                </RadioGroup></div>
              <Button onClick={filtrar}>Filtrar</Button>
            </div>
            <div className="flex items-center justify-between">
              <p className="text-sm">{resultado ? <><b>{resultado.length}</b> resultado(s) encontrados</> : `${recursos.length} recursos registrados`}</p>
              <Button variant="secondary" disabled={!sel.length} onClick={() => setVerHist(true)}>
                <History size={16} /> Historial de precios {sel.length > 0 && `(${sel.length})`}
              </Button>
            </div>
            <Table>
              <TableHeader><TableRow>
                <TableHead className="w-8" /><TableHead>N°</TableHead><TableHead>Código</TableHead><TableHead>Nombre</TableHead>
                <TableHead>Categoría</TableHead><TableHead>Prioridad</TableHead><TableHead className="text-right">Precio</TableHead>
              </TableRow></TableHeader>
              <TableBody>
                {lista.map((x, i) => (
                  <TableRow key={x.id}>
                    <TableCell><Checkbox checked={sel.includes(x.id)} onCheckedChange={() => toggle(x.id)} /></TableCell>
                    <TableCell className="font-semibold">{i + 1}</TableCell>
                    <TableCell className="whitespace-nowrap font-mono text-xs">{x.codigo}</TableCell>
                    <TableCell>{x.nombre}</TableCell>
                    <TableCell>{x.categoria}</TableCell>
                    <TableCell><Badge variant={x.prioridad === "Alta" ? "default" : x.prioridad === "Media" ? "secondary" : "outline"}>{x.prioridad}</Badge></TableCell>
                    <TableCell className="text-right">{clp(x.precio)} / {x.unidad}</TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          </CardContent>
        </Card>
      </div>
      <HistorialDialog open={verHist} onOpenChange={setVerHist} recursos={seleccionados} />
    </div>
  );
}

