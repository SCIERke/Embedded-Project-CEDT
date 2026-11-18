import { useState, useEffect } from 'react';
import { AttitudeIndicator } from './components/AttitudeIndicator';
import { CameraView } from './components/CameraView';
import { TelemetryCharts } from './components/TelemetryCharts';
import type { TelemetryPoint } from './components/TelemetryCharts';
import { ControlPanel } from './components/ControlPanel';
import { Radio, Wifi } from 'lucide-react';

export function App() {
  const [currentAttitude, setCurrentAttitude] = useState({ roll: 0, pitch: 0, yaw: 0 });
  const [history, setHistory] = useState<TelemetryPoint[]>([]);
  const [lastUpdate, setLastUpdate] = useState<string>('Waiting for connection...');
  const [connected, setConnected] = useState<boolean>(false);

  // Poll latest telemetry from Go backend (or connect WebSocket)
  useEffect(() => {
    const fetchLatest = async () => {
      try {
        const res = await fetch('/api/v1/telemetry/latest');
        if (res.ok) {
          const pt = await res.json();
          setCurrentAttitude({
            roll: pt.roll || 0,
            pitch: pt.pitch || 0,
            yaw: pt.yaw || 0,
          });
          setConnected(true);
          const timeStr = new Date().toLocaleTimeString();
          setLastUpdate(timeStr);

          setHistory((prev) => {
            const next = [...prev, {
              time: timeStr,
              roll: Number((pt.roll || 0).toFixed(1)),
              pitch: Number((pt.pitch || 0).toFixed(1)),
              yaw: Number((pt.yaw || 0).toFixed(1)),
              temp: pt.temp || 25,
            }];
            return next.slice(-25); // keep latest 25 readings
          });
        }
      } catch {
        setConnected(false);
      }
    };

    const interval = setInterval(fetchLatest, 1000);
    return () => clearInterval(interval);
  }, []);

  const handleTargetClick = async (ex: number, ey: number) => {
    try {
      await fetch('/api/v1/control/target', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ex, ey }),
      });
    } catch (err) {
      console.error('Failed to send target click:', err);
    }
  };

  const handleUpdatePID = async (kp: number, ki: number, kd: number) => {
    try {
      await fetch('/api/v1/control/pid', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ kp, ki, kd }),
      });
    } catch (err) {
      console.error('Failed to update PID:', err);
    }
  };

  const handleReset = async () => {
    try {
      await fetch('/api/v1/control/reset', { method: 'POST' });
    } catch (err) {
      console.error('Failed to reset gimbal:', err);
    }
  };

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 flex flex-col p-4 md:p-8">
      {/* Top Navigation / Status Header */}
      <header className="flex flex-col md:flex-row items-start md:items-center justify-between pb-6 border-b border-slate-800 gap-4 mb-6">
        <div>
          <div className="flex items-center gap-3">
            <div className="p-2 bg-indigo-600/20 border border-indigo-500/30 rounded-lg text-indigo-400">
              <Radio className="w-6 h-6 animate-pulse" />
            </div>
            <div>
              <h1 className="text-xl md:text-2xl font-bold tracking-tight text-white flex items-center gap-2">
                Chicken Gimbal <span className="text-xs px-2 py-0.5 rounded-full bg-indigo-500/10 text-indigo-400 border border-indigo-500/20 font-mono">CEDT 2026</span>
              </h1>
              <p className="text-xs text-slate-400 font-mono">3-Axis Active Stabilization & Target Tracking Dashboard</p>
            </div>
          </div>
        </div>

        <div className="flex items-center gap-4 text-xs font-mono">
          <div className="flex items-center gap-2 px-3 py-1.5 rounded-lg bg-slate-900 border border-slate-800">
            <Wifi className={`w-3.5 h-3.5 ${connected ? 'text-emerald-400' : 'text-slate-500'}`} />
            <span className={connected ? 'text-emerald-400' : 'text-slate-500'}>
              {connected ? 'ONLINE' : 'WAITING TELEMETRY'}
            </span>
          </div>
          <div className="text-slate-500 hidden sm:block">
            Updated: <span className="text-slate-300">{lastUpdate}</span>
          </div>
        </div>
      </header>

      {/* Main Grid Content */}
      <main className="grid grid-cols-1 lg:grid-cols-3 gap-6 flex-1">
        {/* Left Column: Artificial Horizon & PID Controls */}
        <div className="flex flex-col gap-6">
          <AttitudeIndicator
            roll={currentAttitude.roll}
            pitch={currentAttitude.pitch}
            yaw={currentAttitude.yaw}
          />
          <ControlPanel
            onUpdatePID={handleUpdatePID}
            onReset={handleReset}
          />
        </div>

        {/* Center & Right Column: Camera View + History Chart */}
        <div className="lg:col-span-2 flex flex-col gap-6">
          <CameraView onTargetClick={handleTargetClick} />
          <TelemetryCharts data={history} />
        </div>
      </main>
    </div>
  );
}

export default App;
