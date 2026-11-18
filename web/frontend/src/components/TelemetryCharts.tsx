import React from 'react';
import { ResponsiveContainer, LineChart, Line, XAxis, YAxis, Tooltip, CartesianGrid } from 'recharts';
import { Activity } from 'lucide-react';

export interface TelemetryPoint {
  time: string;
  roll: number;
  pitch: number;
  yaw: number;
  temp: number;
}

interface Props {
  data: TelemetryPoint[];
}

export const TelemetryCharts: React.FC<Props> = ({ data }) => {
  return (
    <div className="bg-slate-900/80 border border-slate-800 rounded-xl p-5 shadow-lg flex flex-col h-full">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Activity className="w-4 h-4 text-sky-400" />
          <h3 className="text-sm font-semibold uppercase tracking-wider text-slate-400">Live Telemetry History</h3>
        </div>
        <div className="flex items-center gap-4 text-xs font-mono">
          <span className="flex items-center gap-1.5 text-sky-400">
            <span className="w-2.5 h-2.5 rounded-full bg-sky-400 inline-block" /> Roll (°)
          </span>
          <span className="flex items-center gap-1.5 text-amber-400">
            <span className="w-2.5 h-2.5 rounded-full bg-amber-400 inline-block" /> Pitch (°)
          </span>
        </div>
      </div>

      <div className="w-full h-64">
        <ResponsiveContainer width="100%" height="100%">
          <LineChart data={data}>
            <CartesianGrid strokeDasharray="3 3" stroke="#1e293b" />
            <XAxis dataKey="time" stroke="#64748b" tick={{ fontSize: 10 }} />
            <YAxis domain={[-45, 45]} stroke="#64748b" tick={{ fontSize: 10 }} unit="°" />
            <Tooltip
              contentStyle={{ backgroundColor: '#0f172a', borderColor: '#334155', borderRadius: '0.5rem' }}
              labelStyle={{ color: '#94a3b8', fontSize: '11px' }}
              itemStyle={{ fontSize: '12px' }}
            />
            <Line type="monotone" dataKey="roll" stroke="#38bdf8" strokeWidth={2} dot={false} isAnimationActive={false} />
            <Line type="monotone" dataKey="pitch" stroke="#fbbf24" strokeWidth={2} dot={false} isAnimationActive={false} />
          </LineChart>
        </ResponsiveContainer>
      </div>
    </div>
  );
};
