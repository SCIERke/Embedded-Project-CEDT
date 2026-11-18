package main

import (
	"context"
	"errors"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/config"
	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/storage"
	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/telemetry"
)

func main() {
	cfg, err := config.Load()
	if err != nil {
		slog.Error("failed to load configuration", "error", err)
		os.Exit(1)
	}

	slog.SetDefault(slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{Level: cfg.LogLevel})))
	slog.Info("starting backend",
		"listen", cfg.Listen,
		"influx_url", cfg.InfluxURL,
		"influx_bucket", cfg.InfluxBucket,
		"env", cfg.ServerEnvironment,
	)

	// 1. Initialize InfluxDB Client
	influxClient, err := storage.NewInfluxClient(cfg)
	if err != nil {
		slog.Error("failed to initialize InfluxDB client", "error", err)
		os.Exit(1)
	}

	// 2. Initialize Telemetry Ingestion Service
	ingestionSvc := telemetry.NewIngestionService(influxClient)

	// 3. HTTP Server Routing
	mux := http.NewServeMux()
	mux.HandleFunc("/health", func(w http.ResponseWriter, r *http.Request) {
		w.WriteHeader(http.StatusOK)
		_, _ = w.Write([]byte(`{"status":"ok"}`))
	})
	mux.HandleFunc("/api/v1/telemetry", ingestionSvc.HTTPHandler())
	mux.HandleFunc("/api/v1/telemetry/latest", ingestionSvc.LatestHandler())

	// Stubs for frontend controls
	mux.HandleFunc("/api/v1/control/target", func(w http.ResponseWriter, r *http.Request) {
		slog.Info("received target control update from frontend")
		w.WriteHeader(http.StatusOK)
		_, _ = w.Write([]byte(`{"status":"target_received"}`))
	})
	mux.HandleFunc("/api/v1/control/pid", func(w http.ResponseWriter, r *http.Request) {
		slog.Info("received PID update from frontend")
		w.WriteHeader(http.StatusOK)
		_, _ = w.Write([]byte(`{"status":"pid_updated"}`))
	})
	mux.HandleFunc("/api/v1/control/reset", func(w http.ResponseWriter, r *http.Request) {
		slog.Info("received zero reset from frontend")
		w.WriteHeader(http.StatusOK)
		_, _ = w.Write([]byte(`{"status":"zeroed"}`))
	})

	server := &http.Server{
		Addr:    cfg.Listen,
		Handler: mux,
	}

	// 4. Start HTTP Server in background
	go func() {
		slog.Info("HTTP server listening", "addr", cfg.Listen)
		if err := server.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
			slog.Error("HTTP server failed", "error", err)
			os.Exit(1)
		}
	}()

	// 5. Graceful Shutdown on SIGINT/SIGTERM
	stopChan := make(chan os.Signal, 1)
	signal.Notify(stopChan, os.Interrupt, syscall.SIGTERM)
	<-stopChan

	slog.Info("shutting down gracefully...")
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		slog.Error("server shutdown failed", "error", err)
	}
	slog.Info("backend stopped cleanly")
}
