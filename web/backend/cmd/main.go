package main

import (
	"log/slog"
	"os"

	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/config"
)
func main() {
	cfg, err := config.Load();
	if err != nil {
		slog.Error("config", "error", err)
		os.Exit(1)
	}

	slog.SetDefault(slog.New(slog.NewTextHandler(os.Stderr, &slog.HandlerOptions{Level: cfg.LogLevel})))

}
