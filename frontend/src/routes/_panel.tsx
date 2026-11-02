import { createFileRoute, Link, Outlet, useNavigate } from "@tanstack/react-router";
import { useEffect } from "react";
import { BarChart3, Calculator, FileSpreadsheet, HardHat, LogOut, Package, Tags, Building2 } from "lucide-react";
import { ROL_LABEL, useStore, type Rol } from "@/lib/store";
import { Button } from "@/components/ui/button";

export const Route = createFileRoute("/_panel")({ component: PanelLayout });

const MENU: { to: "/recursos" | "/precios" | "/apu" | "/calcular" | "/proyectos" | "/indicadores"; label: string; icon: typeof Package; roles: Rol[] }[] = [
  { to: "/recursos", label: "Recursos", icon: Package, roles: ["presupuestos"] },
  { to: "/precios", label: "Actualizar precios", icon: Tags, roles: ["presupuestos"] },
  { to: "/apu", label: "APU registradas", icon: FileSpreadsheet, roles: ["presupuestos"] },
  { to: "/calcular", label: "Calcular APU", icon: Calculator, roles: ["presupuestos"] },
  { to: "/proyectos", label: "Costos de proyecto", icon: Building2, roles: ["jefe"] },
  { to: "/indicadores", label: "Proyectos", icon: BarChart3, roles: ["gerencia"] },
];

function PanelLayout() {
  const { usuario, ready, logout } = useStore();
  const navigate = useNavigate();
  useEffect(() => { if (ready && !usuario) navigate({ to: "/" }); }, [ready, usuario, navigate]);
  if (!ready || !usuario) return null;

  return (
    <div className="flex min-h-screen">
      <aside className="flex w-64 shrink-0 flex-col bg-sidebar text-sidebar-foreground">
        <div className="flex items-center gap-3 p-5">
          <div className="grid h-10 w-10 place-items-center bg-sidebar-primary text-sidebar-primary-foreground"><HardHat size={20} /></div>
          <span className="font-display text-xl font-bold uppercase text-sidebar-accent-foreground">Constructora</span>
        </div>
        <div className="hazard-stripes h-1.5" />
        <nav className="flex-1 space-y-1 p-3">
          {MENU.filter((m) => m.roles.includes(usuario.rol)).map((m) => (
            <Link key={m.to} to={m.to}
              className="flex items-center gap-3 border-l-4 border-transparent px-3 py-2.5 text-sm font-medium hover:bg-sidebar-accent hover:text-sidebar-accent-foreground"
              activeProps={{ className: "border-sidebar-primary bg-sidebar-accent text-sidebar-accent-foreground" }}>
              <m.icon size={18} /> {m.label}
            </Link>
          ))}
        </nav>
        <div className="border-t border-sidebar-border p-4">
          <p className="font-semibold text-sidebar-accent-foreground">{usuario.nombre}</p>
          <p className="mb-3 text-xs">{ROL_LABEL[usuario.rol]}</p>
          <Button variant="secondary" size="sm" className="w-full" onClick={logout}><LogOut size={16} /> Cerrar sesión</Button>
        </div>
      </aside>
      <main className="min-w-0 flex-1 p-8"><Outlet /></main>
    </div>
  );
}
