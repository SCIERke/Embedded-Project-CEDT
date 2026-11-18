import React from 'react';

interface Props {
  roll: number;
  pitch: number;
  yaw: number;
}

export const AttitudeIndicator: React.FC<Props> = ({ roll, pitch, yaw }) => {
  // Clamp pitch visual displacement (-45 to 45 deg maps to -40px to +40px)
  const pitchOffset = Math.max(-45, Math.min(45, pitch)) * 1.2;

  return (
    <div className="flex flex-col items-center bg-slate-900/80 border border-slate-800 rounded-xl p-5 shadow-lg">
      <div className="flex items-center justify-between w-full mb-3">
        <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">Attitude & Orientation</h3>
        <span className="text-xs px-2 py-0.5 rounded bg-emerald-500/10 text-emerald-400 border border-emerald-500/20 font-mono">
          LIVE IMU
        </span>
      </div>

      {/* Artificial Horizon Circle */}
      <div className="relative w-48 h-48 rounded-full border-4 border-slate-700 overflow-hidden bg-sky-600 shadow-inner">
        {/* Sky / Ground rotating disc */}
        <div
          className="absolute inset-[-50%] transition-transform duration-100 ease-out flex flex-col"
          style={{
            transform: `translateY(${pitchOffset}px) rotate(${-roll}deg)`,
          }}
        >
          {/* Sky */}
          <div className="w-full h-1/2 bg-sky-500 flex items-end justify-center pb-2">
            <span className="text-[10px] text-sky-200 font-mono tracking-widest">+10°</span>
          </div>
          {/* Horizon Line */}
          <div className="w-full h-[2px] bg-white shadow-sm" />
          {/* Ground */}
          <div className="w-full h-1/2 bg-amber-800/90 flex items-start justify-center pt-2">
            <span className="text-[10px] text-amber-200 font-mono tracking-widest">-10°</span>
          </div>
        </div>

        {/* Static Airplane Crosshair */}
        <div className="absolute inset-0 flex items-center justify-center pointer-events-none">
          <div className="relative flex items-center justify-center">
            {/* Left wing */}
            <div className="w-8 h-1 bg-amber-400 rounded-sm shadow-sm" />
            {/* Center dot */}
            <div className="w-3 h-3 rounded-full border-2 border-amber-400 mx-1 bg-slate-900" />
            {/* Right wing */}
            <div className="w-8 h-1 bg-amber-400 rounded-sm shadow-sm" />
          </div>
        </div>
      </div>

      {/* Angle Readings */}
      <div className="grid grid-cols-3 gap-3 w-full mt-4 text-center">
        <div className="bg-slate-800/60 p-2 rounded-lg border border-slate-700/50">
          <div className="text-[10px] uppercase text-slate-400 font-bold">Roll</div>
          <div className="text-base font-mono font-bold text-sky-400">{roll.toFixed(1)}°</div>
        </div>
        <div className="bg-slate-800/60 p-2 rounded-lg border border-slate-700/50">
          <div className="text-[10px] uppercase text-slate-400 font-bold">Pitch</div>
          <div className="text-base font-mono font-bold text-amber-400">{pitch.toFixed(1)}°</div>
        </div>
        <div className="bg-slate-800/60 p-2 rounded-lg border border-slate-700/50">
          <div className="text-[10px] uppercase text-slate-400 font-bold">Yaw</div>
          <div className="text-base font-mono font-bold text-emerald-400">{yaw.toFixed(1)}°</div>
        </div>
      </div>
    </div>
  );
};
