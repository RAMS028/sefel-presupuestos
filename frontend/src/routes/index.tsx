import { createFileRoute, useNavigate } from "@tanstack/react-router";
import { useEffect, useState } from "react";
import { toast } from "sonner";
import { HardHat } from "lucide-react";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { ROL_LABEL, USUARIOS, useStore, type Rol } from "@/lib/store";

export const Route = createFileRoute("/")({
  head: () => ({
    meta: [
      { title: "Iniciar sesión — Sistema de Presupuestos Constructora" },
      { name: "description", content: "Acceso al sistema de presupuestos, APU y control de costos de obra." },
      { property: "og:title", content: "Iniciar sesión — Sistema de Presupuestos" },
      { property: "og:description", content: "Acceso por tipo de usuario: presupuestos, jefe de proyecto y gerencia." },
    ],
  }),
  component: Login,
});

const INICIO: Record<Rol, "/recursos" | "/proyectos" | "/indicadores"> = {
  presupuestos: "/recursos", jefe: "/proyectos", gerencia: "/indicadores",
};

function Login() {
  const { login, usuario, ready } = useStore();
  const navigate = useNavigate();
  const [u, setU] = useState("");
  const [c, setC] = useState("");

  useEffect(() => { if (ready && usuario) navigate({ to: INICIO[usuario.rol] }); }, [ready, usuario, navigate]);

  const submit = (e: React.FormEvent) => {
    e.preventDefault();
    const found = login(u, c);
    if (!found) { toast.error("Credenciales incorrectas"); return; }
    toast.success(`Bienvenido/a, ${found.nombre}`);
  };

  return (
    <div className="grid min-h-screen lg:grid-cols-2">
      <div className="relative hidden flex-col justify-between bg-ink p-12 text-ink-foreground lg:flex">
        <div className="flex items-center gap-3">
          <div className="grid h-12 w-12 place-items-center bg-primary text-primary-foreground"><HardHat /></div>
          <span className="font-display text-2xl font-bold uppercase">Constructora</span>
        </div>
        <div>
          <h1 className="text-6xl font-extrabold uppercase leading-none">Presupuestos<br /><span className="text-primary">y control</span><br />de costos</h1>
          <p className="mt-6 max-w-md text-ink-muted">Recursos, APU, presupuestos y seguimiento de costos reales de cada obra en un solo lugar.</p>
        </div>
        <div className="hazard-stripes h-3 w-full" />
      </div>
      <div className="flex items-center justify-center p-6">
        <div className="w-full max-w-sm">
          <h2 className="text-3xl font-bold uppercase">Iniciar sesión</h2>
          <p className="mb-6 text-sm text-muted-foreground">Ingresa tus credenciales para acceder al sistema.</p>
          <form onSubmit={submit} className="space-y-4">
            <div className="space-y-2"><Label htmlFor="u">Usuario</Label><Input id="u" value={u} onChange={(e) => setU(e.target.value)} /></div>
            <div className="space-y-2"><Label htmlFor="c">Contraseña</Label><Input id="c" type="password" value={c} onChange={(e) => setC(e.target.value)} /></div>
            <Button type="submit" className="w-full">Ingresar</Button>
          </form>
          <div className="mt-8 border border-dashed p-4 text-sm">
            <p className="mb-2 font-semibold">Usuarios de prueba (clave: demo123)</p>
            {USUARIOS.map((x) => (
              <button key={x.usuario} type="button" onClick={() => { setU(x.usuario); setC(x.clave); }}
                className="flex w-full justify-between py-1 text-left hover:text-primary">
                <span className="font-mono">{x.usuario}</span><span className="text-muted-foreground">{ROL_LABEL[x.rol]}</span>
              </button>
            ))}
          </div>
        </div>
      </div>
    </div>
  );
}
