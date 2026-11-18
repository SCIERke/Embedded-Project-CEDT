package config

import (
	"log/slog"
	"time"

	"github.com/caarlos0/env/v11"
	"github.com/joho/godotenv"
)

type Config struct {
	Listen      string `env:"SERVER_LISTEN" envDefault:":8000"`
	DatabaseURL string `env:"SERVER_DATABASE_URL"`

	LogLevel          slog.Level `env:"SERVER_LOG_LEVEL" envDefault:"info"`
	ServerEnvironment string     `env:"SERVER_ENVIRONMENT" envDefault:"dev"`

	// InfluxDB Configuration
	InfluxURL    string `env:"INFLUX_URL" envDefault:"http://localhost:8181"`
	InfluxToken  string `env:"INFLUX_TOKEN"`
	InfluxOrg    string `env:"INFLUX_ORG" envDefault:"gimbal-org"`
	InfluxBucket string `env:"INFLUX_BUCKET" envDefault:"gimbal"`

	// MQTT Configuration
	MQTTBroker   string `env:"MQTT_BROKER" envDefault:"tcp://localhost:1883"`
	MQTTClientID string `env:"MQTT_CLIENT_ID" envDefault:"go-backend"`
	MQTTUser     string `env:"MQTT_USER"`
	MQTTPassword string `env:"MQTT_PASSWORD"`
	MQTTTopic    string `env:"MQTT_SENSOR_TOPIC" envDefault:"nodes/+/data"`

	JWTSecret  string        `env:"SERVER_JWT_SECRET"`
	AccessTTL  time.Duration `env:"SERVER_ACCESS_TTL" envDefault:"15m"`
	RefreshTTL time.Duration `env:"SERVER_REFRESH_TTL" envDefault:"168h"`

	AgentURL    string `env:"AGENT_URL"`
	AgentModel  string `env:"AGENT_MODEL"`
	AgentAPIKey string `env:"AGENT_API_KEY"`
}

func Load() (Config, error) {
	// Best-effort load of .env file
	_ = godotenv.Load()

	return env.ParseAs[Config]()
}
