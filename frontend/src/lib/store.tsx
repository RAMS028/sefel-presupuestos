import { createContext, useContext, useEffect, useState, type ReactNode } from "react";

export type Rol = "presupuestos" | "jefe" | "gerencia";
export type Prioridad = "Alta" | "Media" | "Baja";

export interface Usuario { usuario: string; clave: string; nombre: string; rol: Rol }
export interface PrecioHist { fecha: string; precio: number }
export interface Recurso {
  id: string; codigo: string; nombre: string; unidad: string; categoria: string;
  prioridad: Prioridad; precio: number; historial: PrecioHist[];
}
export interface ApuItem { recursoId: string; cantidad: number }
export interface Apu { id: string; codigo: string; nombre: string; unidad: string; items: ApuItem[] }
export interface Partida { apuId: string; cantidad: number }
export interface CostoReal {
  id: string; fecha: string; tipo: "Ejecución" | "Compra"; descripcion: string; apuId: string; monto: number;
}
export interface Proyecto {
  id: string; nombre: string; cliente: string; ubicacion: string; avance: number;
  partidas: Partida[]; costos: CostoReal[];
}

export const ROL_LABEL: Record<Rol, string> = {
  presupuestos: "Encargado de presupuestos",
  jefe: "Jefe de proyecto",
  gerencia: "Gerencia",
};

export const USUARIOS: Usuario[] = [
  { usuario: "presupuestos", clave: "demo123", nombre: "Carla Muñoz", rol: "presupuestos" },
  { usuario: "jefe", clave: "demo123", nombre: "Rodrigo Soto", rol: "jefe" },
  { usuario: "gerencia", clave: "demo123", nombre: "Andrea Vidal", rol: "gerencia" },
];

const hist = (p: number): PrecioHist[] => [
  { fecha: "2026-03-01", precio: Math.round(p * 0.9) },
  { fecha: "2026-06-01", precio: Math.round(p * 0.96) },
  { fecha: "2026-09-01", precio: p },
];
const r = (id: string, codigo: string, nombre: string, unidad: string, categoria: string, prioridad: Prioridad, precio: number): Recurso =>
  ({ id, codigo, nombre, unidad, categoria, prioridad, precio, historial: hist(precio) });

const SEED_RECURSOS: Recurso[] = [
  r("r1", "MAT-001", "Cemento Portland saco 25 kg", "saco", "Material", "Alta", 5490),
  r("r2", "MAT-002", "Arena gruesa", "m³", "Material", "Media", 18900),
  r("r3", "MAT-003", "Grava 3/4\"", "m³", "Material", "Media", 21500),
  r("r4", "MAT-004", "Fierro estriado 10 mm x 6 m", "un", "Material", "Alta", 6890),
  r("r5", "MAT-005", "Ladrillo fiscal", "un", "Material", "Baja", 390),
  r("r6", "MAT-006", "Placa yeso-cartón 15 mm", "un", "Material", "Media", 8990),
  r("r7", "MAT-007", "Pintura látex blanca 1 gl", "gl", "Material", "Baja", 15990),
  r("r8", "MO-001", "Maestro albañil", "hh", "Mano de obra", "Alta", 9500),
  r("r9", "MO-002", "Ayudante", "hh", "Mano de obra", "Media", 6200),
  r("r10", "MO-003", "Enfierrador", "hh", "Mano de obra", "Alta", 9000),
  r("r11", "EQ-001", "Betonera 150 L", "día", "Equipo", "Media", 25000),
  r("r12", "EQ-002", "Andamio (cuerpo)", "día", "Equipo", "Baja", 3500),
];

const SEED_APU: Apu[] = [
  { id: "a1", codigo: "APU-01", nombre: "Hormigón H25 elaborado en obra", unidad: "m³", items: [
    { recursoId: "r1", cantidad: 14 }, { recursoId: "r2", cantidad: 0.5 }, { recursoId: "r3", cantidad: 0.8 },
    { recursoId: "r8", cantidad: 4 }, { recursoId: "r9", cantidad: 8 }, { recursoId: "r11", cantidad: 0.5 } ] },
  { id: "a2", codigo: "APU-02", nombre: "Albañilería ladrillo fiscal", unidad: "m²", items: [
    { recursoId: "r5", cantidad: 38 }, { recursoId: "r1", cantidad: 0.6 }, { recursoId: "r2", cantidad: 0.03 },
    { recursoId: "r8", cantidad: 1.2 }, { recursoId: "r9", cantidad: 1.2 } ] },
  { id: "a3", codigo: "APU-03", nombre: "Enfierradura losa", unidad: "kg", items: [
    { recursoId: "r4", cantidad: 0.27 }, { recursoId: "r10", cantidad: 0.05 }, { recursoId: "r9", cantidad: 0.03 } ] },
  { id: "a4", codigo: "APU-04", nombre: "Tabique yeso-cartón pintado", unidad: "m²", items: [
    { recursoId: "r6", cantidad: 0.7 }, { recursoId: "r7", cantidad: 0.08 }, { recursoId: "r8", cantidad: 0.6 },
    { recursoId: "r12", cantidad: 0.2 } ] },
];

const SEED_PROYECTOS: Proyecto[] = [
  { id: "p1", nombre: "Edificio Los Aromos", cliente: "Inmobiliaria Andes", ubicacion: "Ñuñoa, Santiago", avance: 55,
    partidas: [{ apuId: "a1", cantidad: 120 }, { apuId: "a2", cantidad: 800 }, { apuId: "a3", cantidad: 9000 }],
    costos: [
      { id: "c1", fecha: "2026-08-10", tipo: "Compra", descripcion: "Cemento y áridos", apuId: "a1", monto: 9800000 },
      { id: "c2", fecha: "2026-09-05", tipo: "Ejecución", descripcion: "Muros 1er y 2do piso", apuId: "a2", monto: 21500000 },
      { id: "c3", fecha: "2026-09-20", tipo: "Compra", descripcion: "Fierro losas", apuId: "a3", monto: 12400000 },
    ] },
  { id: "p2", nombre: "Bodega Industrial Quilicura", cliente: "Logística Sur", ubicacion: "Quilicura", avance: 30,
    partidas: [{ apuId: "a1", cantidad: 300 }, { apuId: "a3", cantidad: 15000 }],
    costos: [
      { id: "c4", fecha: "2026-09-12", tipo: "Ejecución", descripcion: "Fundaciones", apuId: "a1", monto: 14200000 },
    ] },
  { id: "p3", nombre: "Remodelación Oficinas Centro", cliente: "Estudio Legal Ríos", ubicacion: "Santiago Centro", avance: 80,
    partidas: [{ apuId: "a4", cantidad: 450 }, { apuId: "a2", cantidad: 60 }],
    costos: [
      { id: "c5", fecha: "2026-07-02", tipo: "Compra", descripcion: "Placas y perfiles", apuId: "a4", monto: 4300000 },
      { id: "c6", fecha: "2026-08-15", tipo: "Ejecución", descripcion: "Instalación tabiques", apuId: "a4", monto: 7900000 },
      { id: "c7", fecha: "2026-08-30", tipo: "Ejecución", descripcion: "Muros baño", apuId: "a2", monto: 2100000 },
    ] },
];

interface Data { recursos: Recurso[]; apus: Apu[]; proyectos: Proyecto[] }
const SEED: Data = { recursos: SEED_RECURSOS, apus: SEED_APU, proyectos: SEED_PROYECTOS };
const KEY = "constructora-demo-v1";
const SESSION_KEY = "constructora-demo-sesion";

export const uid = () => Math.random().toString(36).slice(2, 9);
export const hoy = () => new Date().toISOString().slice(0, 10);
export const clp = (n: number) =>
  new Intl.NumberFormat("es-CL", { style: "currency", currency: "CLP", maximumFractionDigits: 0 }).format(n);

export function costoUnitarioApu(apu: Apu, recursos: Recurso[]) {
  return apu.items.reduce((s, it) => s + (recursos.find((x) => x.id === it.recursoId)?.precio ?? 0) * it.cantidad, 0);
}
export function resumenProyecto(p: Proyecto, apus: Apu[], recursos: Recurso[]) {
  const filas = p.partidas.map((pa) => {
    const apu = apus.find((a) => a.id === pa.apuId)!;
    const presupuestado = apu ? costoUnitarioApu(apu, recursos) * pa.cantidad : 0;
    const real = p.costos.filter((c) => c.apuId === pa.apuId).reduce((s, c) => s + c.monto, 0);
    const presupuestadoAvance = presupuestado * (p.avance / 100);
    const desviacion = real - presupuestadoAvance;
    return { apu, cantidad: pa.cantidad, presupuestado, presupuestadoAvance, real, desviacion,
      pct: presupuestadoAvance ? (desviacion / presupuestadoAvance) * 100 : 0 };
  });
  const presupuestado = filas.reduce((s, f) => s + f.presupuestado, 0);
  const presupuestadoAvance = presupuestado * (p.avance / 100);
  const real = p.costos.reduce((s, c) => s + c.monto, 0);
  const desviacion = real - presupuestadoAvance;
  const pct = presupuestadoAvance ? (desviacion / presupuestadoAvance) * 100 : 0;
  const estado = pct > 10 ? "Sobrecosto" : pct > 0 ? "En alerta" : "Dentro de presupuesto";
  return { filas, presupuestado, presupuestadoAvance, real, desviacion, pct, estado };
}

interface Ctx extends Data {
  ready: boolean;
  usuario: Usuario | null;
  login: (u: string, c: string) => Usuario | null;
  logout: () => void;
  addRecurso: (r: Omit<Recurso, "id" | "historial">) => void;
  actualizarPrecios: (cambios: Record<string, number>) => number;
  addApu: (a: Omit<Apu, "id">) => Apu;
  addCosto: (proyectoId: string, c: Omit<CostoReal, "id">) => void;
  reset: () => void;
}
const StoreCtx = createContext<Ctx | null>(null);

export function StoreProvider({ children }: { children: ReactNode }) {
  const [data, setData] = useState<Data>(SEED);
  const [usuario, setUsuario] = useState<Usuario | null>(null);
  const [ready, setReady] = useState(false);

  useEffect(() => {
    try {
      const d = localStorage.getItem(KEY);
      if (d) setData(JSON.parse(d));
      const s = localStorage.getItem(SESSION_KEY);
      if (s) setUsuario(USUARIOS.find((u) => u.usuario === s) ?? null);
    } catch { /* ignore */ }
    setReady(true);
  }, []);
  useEffect(() => { if (ready) localStorage.setItem(KEY, JSON.stringify(data)); }, [data, ready]);

  const value: Ctx = {
    ...data, ready, usuario,
    login: (u, c) => {
      const found = USUARIOS.find((x) => x.usuario === u.trim().toLowerCase() && x.clave === c) ?? null;
      if (found) { setUsuario(found); localStorage.setItem(SESSION_KEY, found.usuario); }
      return found;
    },
    logout: () => { setUsuario(null); localStorage.removeItem(SESSION_KEY); },
    addRecurso: (rec) => setData((d) => ({ ...d, recursos: [...d.recursos,
      { ...rec, id: uid(), historial: [{ fecha: hoy(), precio: rec.precio }] }] })),
    actualizarPrecios: (cambios) => {
      const n = Object.keys(cambios).length;
      setData((d) => ({ ...d, recursos: d.recursos.map((x) => {
        const np = cambios[x.id];
        return np !== undefined && np !== x.precio
          ? { ...x, precio: np, historial: [...x.historial, { fecha: hoy(), precio: np }] } : x;
      }) }));
      return n;
    },
    addApu: (a) => { const apu = { ...a, id: uid() }; setData((d) => ({ ...d, apus: [...d.apus, apu] })); return apu; },
    addCosto: (pid, c) => setData((d) => ({ ...d, proyectos: d.proyectos.map((p) => p.id === pid
      ? { ...p, costos: [...p.costos, { ...c, id: uid() }] } : p) })),
    reset: () => setData(SEED),
  };
  return <StoreCtx.Provider value={value}>{children}</StoreCtx.Provider>;
}

export function useStore() {
  const c = useContext(StoreCtx);
  if (!c) throw new Error("useStore fuera de StoreProvider");
  return c;
}
