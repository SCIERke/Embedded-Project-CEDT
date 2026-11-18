import React, { useRef, useState } from 'react';
import { Target, Crosshair } from 'lucide-react';

interface Props {
  onTargetClick: (ex: number, ey: number) => void;
  streamUrl?: string;
}

export const CameraView: React.FC<Props> = ({ onTargetClick, streamUrl }) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [clickCoord, setClickCoord] = useState<{ x: number; y: number; ex: number; ey: number } | null>(null);

  const handleClick = (e: React.MouseEvent<HTMLDivElement>) => {
    if (!containerRef.current) return;
    const rect = containerRef.current.getBoundingClientRect();

    // Click relative to container in pixels
    const px = e.clientX - rect.left;
    const py = e.clientY - rect.top;

    // Normalize error ex, ey from -1.0 (left/top) to +1.0 (right/bottom)
    const ex = ((px - rect.width / 2) / (rect.width / 2));
    const ey = ((py - rect.height / 2) / (rect.height / 2));

    setClickCoord({ x: px, y: py, ex, ey });
    onTargetClick(Number(ex.toFixed(3)), Number(ey.toFixed(3)));
  };

  return (
    <div className="bg-slate-900/80 border border-slate-800 rounded-xl p-5 shadow-lg flex flex-col">
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center gap-2">
          <Crosshair className="w-4 h-4 text-rose-400" />
          <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">Click-to-Track Camera View</h3>
        </div>
        <span className="text-xs px-2 py-0.5 rounded bg-rose-500/10 text-rose-400 border border-rose-500/20 font-mono">
          INTERACTIVE
        </span>
      </div>

      {/* Video Viewport / Click Canvas */}
      <div
        ref={containerRef}
        onClick={handleClick}
        className="relative w-full aspect-video bg-slate-950 rounded-lg overflow-hidden border border-slate-800 cursor-crosshair group flex items-center justify-center select-none shadow-inner"
      >
        {streamUrl ? (
          <img src={streamUrl} alt="ESP32 Stream" className="w-full h-full object-contain" />
        ) : (
          <div className="text-center p-6 text-slate-600 flex flex-col items-center">
            <Target className="w-10 h-10 mb-2 opacity-40 group-hover:scale-110 transition-transform text-slate-400" />
            <p className="text-xs font-mono text-slate-500">Camera stream placeholder / No feed</p>
            <p className="text-[11px] text-slate-600 mt-1">Click anywhere inside frame to test target coordinates</p>
          </div>
        )}

        {/* Center Crosshair Overlay */}
        <div className="absolute inset-0 pointer-events-none flex items-center justify-center">
          <div className="w-8 h-[1px] bg-white/20" />
          <div className="h-8 w-[1px] bg-white/20 absolute" />
        </div>

        {/* Target Reticle where clicked */}
        {clickCoord && (
          <div
            className="absolute pointer-events-none transition-all duration-75"
            style={{ left: clickCoord.x, top: clickCoord.y }}
          >
            <div className="relative -left-4 -top-4 w-8 h-8 rounded-full border-2 border-rose-500 animate-ping opacity-75" />
            <div className="absolute -left-3 -top-3 w-6 h-6 rounded-full border border-rose-400 flex items-center justify-center">
              <div className="w-1.5 h-1.5 bg-rose-500 rounded-full" />
            </div>
          </div>
        )}
      </div>

      {/* Coordinate Telemetry readout */}
      <div className="mt-3 flex items-center justify-between text-xs font-mono text-slate-400 bg-slate-950/60 px-3 py-2 rounded-lg border border-slate-800">
        <span>Click Target Error:</span>
        {clickCoord ? (
          <span className="text-rose-400 font-bold">
            ex: {clickCoord.ex > 0 ? `+${clickCoord.ex.toFixed(2)}` : clickCoord.ex.toFixed(2)} | ey: {clickCoord.ey > 0 ? `+${clickCoord.ey.toFixed(2)}` : clickCoord.ey.toFixed(2)}
          </span>
        ) : (
          <span className="text-slate-600">None selected</span>
        )}
      </div>
    </div>
  );
};
