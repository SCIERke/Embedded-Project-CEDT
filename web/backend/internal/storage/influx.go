package storage

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"net/url"
	"strings"
	"sync"
	"time"

	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/config"
)

// InfluxClient handles writing points via InfluxDB Line Protocol HTTP API (compatible with InfluxDB v2 and v3 Core)
type InfluxClient struct {
	cfg        config.Config
	httpClient *http.Client
	writeURL   string
	closeOnce  sync.Once
}

// SensorPoint represents a parsed sensor telemetry reading
type SensorPoint struct {
	Measurement string
	Tags        map[string]string
	Fields      map[string]interface{}
	Timestamp   time.Time
}

// NewInfluxClient creates a new InfluxDB client
func NewInfluxClient(cfg config.Config) (*InfluxClient, error) {
	baseURL := strings.TrimRight(cfg.InfluxURL, "/")

	// InfluxDB v2/v3 write endpoint: /api/v2/write?bucket=<bucket>&org=<org>&precision=ns
	u, err := url.Parse(baseURL + "/api/v2/write")
	if err != nil {
		return nil, fmt.Errorf("invalid influx url: %w", err)
	}

	q := u.Query()
	q.Set("bucket", cfg.InfluxBucket)
	if cfg.InfluxOrg != "" {
		q.Set("org", cfg.InfluxOrg)
	}
	q.Set("precision", "ns")
	u.RawQuery = q.Encode()

	return &InfluxClient{
		cfg: cfg,
		httpClient: &http.Client{
			Timeout: 5 * time.Second,
		},
		writeURL: u.String(),
	}, nil
}

// WriteLineProtocol sends raw InfluxDB Line Protocol string
func (c *InfluxClient) WriteLineProtocol(ctx context.Context, lineProtocol string) error {
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, c.writeURL, bytes.NewBufferString(lineProtocol))
	if err != nil {
		return fmt.Errorf("create influx write request: %w", err)
	}

	req.Header.Set("Content-Type", "text/plain; charset=utf-8")
	if c.cfg.InfluxToken != "" {
		req.Header.Set("Authorization", "Token "+c.cfg.InfluxToken)
	}

	resp, err := c.httpClient.Do(req)
	if err != nil {
		return fmt.Errorf("execute influx write: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		body, _ := io.ReadAll(resp.Body)
		return fmt.Errorf("influx write failed (status %d): %s", resp.StatusCode, string(body))
	}

	return nil
}

// WritePoint formats and sends a single sensor data point
func (c *InfluxClient) WritePoint(ctx context.Context, pt SensorPoint) error {
	if pt.Measurement == "" {
		pt.Measurement = "sensor_data"
	}
	if pt.Timestamp.IsZero() {
		pt.Timestamp = time.Now()
	}

	// Format: measurement,tag1=val1,tag2=val2 field1=val1,field2=val2 timestamp_ns
	var tagParts []string
	for k, v := range pt.Tags {
		if k != "" && v != "" {
			tagParts = append(tagParts, fmt.Sprintf("%s=%s", escapeKey(k), escapeKey(v)))
		}
	}

	var fieldParts []string
	for k, v := range pt.Fields {
		switch val := v.(type) {
		case float64:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%f", escapeKey(k), val))
		case float32:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%f", escapeKey(k), val))
		case int, int32, int64:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%di", escapeKey(k), val))
		case uint, uint32, uint64:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%di", escapeKey(k), val))
		case bool:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%t", escapeKey(k), val))
		case string:
			fieldParts = append(fieldParts, fmt.Sprintf("%s=%q", escapeKey(k), val))
		}
	}

	if len(fieldParts) == 0 {
		return fmt.Errorf("point has no fields to write")
	}

	measurementAndTags := escapeKey(pt.Measurement)
	if len(tagParts) > 0 {
		measurementAndTags += "," + strings.Join(tagParts, ",")
	}

	line := fmt.Sprintf("%s %s %d\n", measurementAndTags, strings.Join(fieldParts, ","), pt.Timestamp.UnixNano())
	slog.Debug("writing line protocol to InfluxDB", "line", strings.TrimSpace(line))

	return c.WriteLineProtocol(ctx, line)
}

func escapeKey(s string) string {
	s = strings.ReplaceAll(s, ",", `\,`)
	s = strings.ReplaceAll(s, " ", `\ `)
	s = strings.ReplaceAll(s, "=", `\=`)
	return s
}
