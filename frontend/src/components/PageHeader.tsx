import type { ReactNode } from "react";

export function PageHeader({ hu, title, children }: { hu: string; title: string; children?: ReactNode }) {
  return (
    <div className="mb-6 flex flex-wrap items-end justify-between gap-4 border-b-2 border-ink pb-4">
      <div>
        <span className="text-xs font-semibold uppercase tracking-widest text-primary">{hu}</span>
        <h1 className="text-3xl font-bold uppercase text-foreground">{title}</h1>
      </div>
      {children}
    </div>
  );
}
