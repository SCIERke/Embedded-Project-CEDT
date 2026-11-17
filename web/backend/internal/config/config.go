package config

import (
	"fmt"
	"log/slog"
	"time"

	"github.com/caarlos0/env/v11"
	"github.com/joho/godotenv"
)


type Config struct {
	Listen      string `env:"SERVER_LISTEN" envDefault:":8000"`
	DatabaseURL string `env:"SERVER_DATABASE_URL,notEmpty"`

	LogLevel          slog.Level    `env:"SERVER_LOG_LEVEL" envDefault:"info"`
	ServerEnvironment string        `env:"SERVER_ENVIRONMENT" envDefault:"dev"`

	JWTSecret  string        `env:"SERVER_JWT_SECRET"`
	AccessTTL  time.Duration `env:"SERVER_ACCESS_TTL" envDefault:"15m"`
	RefreshTTL time.Duration `env:"SERVER_REFRESH_TTL" envDefault:"168h"`

	AgentURL    string `env:"AGENT_URL"`
	AgentModel  string `env:"AGENT_MODEL"`
	AgentAPIKey string `env:"AGENT_API_KEY"`
}

func Load() (Config , error){
	if err := godotenv.Load(); err != nil {
		return Config{}, fmt.Errorf("load .env: %w", err)
	}

	return env.ParseAs[Config]()
}