import React, { useState } from 'react';
import { Sliders, RotateCcw, AlertTriangle } from 'lucide-react';

interface Props {
  onUpdatePID: (kp: number, ki: number, kd: number) => void;
  onReset: () => void;
}

export const ControlPanel: React.FC<Props> = ({ onUpdatePID, onReset }) => {
  const [kp, setKp] = useState(1.5);
  const [ki, setKi] = useState(0.0);
  const [kd, setKd] = useState(0.05);

  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    onUpdatePID(kp, ki, kd);
  };

  return (
    <div className="bg-slate-900/80 border border-slate-800 rounded-xl p-5 shadow-lg flex flex-col justify-between">
      <div>
        <div className="flex items-center gap-2 mb-4">
          <Sliders className="w-4 h-4 text-purple-400" />
          <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">Gimbal Controller & PID</h3>
        </div>

        <form onSubmit={handleSubmit} className="space-y-3">
          <div>
            <div className="flex justify-between text-xs font-mono text-slate-300 mb-1">
              <span>Proportional (Kp)</span>
              <span className="text-purple-400 font-bold">{kp.toFixed(2)}</span>
            </div>
            <input
              type="range"
              min="0"
              max="5"
              step="0.05"
              value={kp}
              onChange={(e) => setKp(parseFloat(e.target.value))}
              className="w-full h-1.5 bg-slate-700 rounded-lg appearance-none cursor-pointer accent-purple-500"
            />
          </div>

          <div>
            <div className="flex justify-between text-xs font-mono text-slate-300 mb-1">
              <span>Integral (Ki)</span>
              <span className="text-purple-400 font-bold">{ki.toFixed(3)}</span>
            </div>
            <input
              type="range"
              min="0"
              max="0.5"
              step="0.005"
              value={ki}
              onChange={(e) => setKi(parseFloat(e.target.value))}
              className="w-full h-1.5 bg-slate-700 rounded-lg appearance-none cursor-pointer accent-purple-500"
            />
          </div>

          <div>
            <div className="flex justify-between text-xs font-mono text-slate-300 mb-1">
              <span>Derivative (Kd)</span>
              <span className="text-purple-400 font-bold">{kd.toFixed(3)}</span>
            </div>
            <input
              type="range"
              min="0"
              max="0.5"
              step="0.005"
              value={kd}
              onChange={(e) => setKd(parseFloat(e.target.value))}
              className="w-full h-1.5 bg-slate-700 rounded-lg appearance-none cursor-pointer accent-purple-500"
            />
          </div>

          <button
            type="submit"
            className="w-full mt-2 py-2 px-4 bg-purple-600 hover:bg-purple-500 text-white text-xs font-bold rounded-lg transition-colors shadow"
          >
            Apply PID Gains
          </button>
        </form>
      </div>

      <div className="pt-4 border-t border-slate-800 flex gap-2 mt-4">
        <button
          onClick={onReset}
          className="flex-1 flex items-center justify-center gap-1.5 py-2 px-3 bg-slate-800 hover:bg-slate-700 text-slate-200 text-xs font-semibold rounded-lg transition-colors border border-slate-700"
        >
          <RotateCcw className="w-3.5 h-3.5" />
          Zero Calibrate
        </button>
        <button
          onClick={() => onUpdatePID(0, 0, 0)}
          className="flex items-center justify-center gap-1 py-2 px-3 bg-rose-950/60 hover:bg-rose-900/80 text-rose-300 text-xs font-semibold rounded-lg transition-colors border border-rose-900"
        >
          <AlertTriangle className="w-3.5 h-3.5" />
          Stop
        </button>
      </div>
    </div>
  );
};
