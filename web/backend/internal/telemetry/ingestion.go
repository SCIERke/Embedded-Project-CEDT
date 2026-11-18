package telemetry

import (
	"context"
	"encoding/json"
	"fmt"
	"log/slog"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/SCIERke/Embedded-Project-CEDT/web/backend/internal/storage"
)

// IngestionService processes incoming raw telemetry and stores it into InfluxDB
type IngestionService struct {
	influx       *storage.InfluxClient
	mu           sync.RWMutex
	latestPoint  *SensorPayload
	latestUpdate time.Time
}

// SensorPayload represents standard payload received from node or gateway
type SensorPayload struct {
	NodeID      string                 `json:"node_id"`
	Seq         int64                  `json:"seq,omitempty"`
	Timestamp   *int64                 `json:"ts,omitempty"`
	Orientation *OrientationData       `json:"orientation,omitempty"`
	RawSensors  map[string]interface{} `json:"sensors,omitempty"`

	// Direct flat fields supported for easy ESP32 JSON serialization
	Roll   *float64 `json:"roll,omitempty"`
	Pitch  *float64 `json:"pitch,omitempty"`
	Yaw    *float64 `json:"yaw,omitempty"`
	GyroX  *float64 `json:"gx,omitempty"`
	GyroY  *float64 `json:"gy,omitempty"`
	GyroZ  *float64 `json:"gz,omitempty"`
	AccelX *float64 `json:"ax,omitempty"`
	AccelY *float64 `json:"ay,omitempty"`
	AccelZ *float64 `json:"az,omitempty"`
	Temp   *float64 `json:"temp,omitempty"`
}

type OrientationData struct {
	Roll  float64 `json:"roll"`
	Pitch float64 `json:"pitch"`
	Yaw   float64 `json:"yaw"`
}

func NewIngestionService(influx *storage.InfluxClient) *IngestionService {
	return &IngestionService{
		influx: influx,
	}
}

// ProcessPayload parses a sensor message and writes to InfluxDB
func (s *IngestionService) ProcessPayload(ctx context.Context, defaultNodeID string, data []byte) error {
	var payload SensorPayload
	if err := json.Unmarshal(data, &payload); err != nil {
		return fmt.Errorf("unmarshal telemetry json: %w", err)
	}

	nodeID := payload.NodeID
	if nodeID == "" {
		nodeID = defaultNodeID
	}
	if nodeID == "" {
		nodeID = "unknown_node"
	}
	payload.NodeID = nodeID

	ts := time.Now()
	if payload.Timestamp != nil && *payload.Timestamp > 0 {
		ts = time.UnixMilli(*payload.Timestamp)
	}

	fields := make(map[string]interface{})
	if payload.Seq > 0 {
		fields["seq"] = payload.Seq
	}

	if payload.Orientation != nil {
		fields["roll"] = payload.Orientation.Roll
		fields["pitch"] = payload.Orientation.Pitch
		fields["yaw"] = payload.Orientation.Yaw
	}
	if payload.Roll != nil {
		fields["roll"] = *payload.Roll
	}
	if payload.Pitch != nil {
		fields["pitch"] = *payload.Pitch
	}
	if payload.Yaw != nil {
		fields["yaw"] = *payload.Yaw
	}
	if payload.GyroX != nil {
		fields["gx"] = *payload.GyroX
	}
	if payload.GyroY != nil {
		fields["gy"] = *payload.GyroY
	}
	if payload.GyroZ != nil {
		fields["gz"] = *payload.GyroZ
	}
	if payload.AccelX != nil {
		fields["ax"] = *payload.AccelX
	}
	if payload.AccelY != nil {
		fields["ay"] = *payload.AccelY
	}
	if payload.AccelZ != nil {
		fields["az"] = *payload.AccelZ
	}
	if payload.Temp != nil {
		fields["temp"] = *payload.Temp
	}

	// Dynamic custom sensor attributes
	for k, v := range payload.RawSensors {
		fields[k] = v
	}

	pt := storage.SensorPoint{
		Measurement: "gimbal_telemetry",
		Tags: map[string]string{
			"node_id": nodeID,
		},
		Fields:    fields,
		Timestamp: ts,
	}

	// Save latest in memory for fast frontend polling
	s.mu.Lock()
	s.latestPoint = &payload
	s.latestUpdate = ts
	s.mu.Unlock()

	return s.influx.WritePoint(ctx, pt)
}

// HTTPHandler returns an http.HandlerFunc to ingest telemetry via POST /api/v1/telemetry
func (s *IngestionService) HTTPHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method != http.MethodPost {
			http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
			return
		}

		var payload SensorPayload
		if err := json.NewDecoder(r.Body).Decode(&payload); err != nil {
			http.Error(w, "Invalid JSON: "+err.Error(), http.StatusBadRequest)
			return
		}

		nodeID := r.URL.Query().Get("node_id")
		if payload.NodeID != "" {
			nodeID = payload.NodeID
		}

		rawJSON, _ := json.Marshal(payload)
		if err := s.ProcessPayload(r.Context(), nodeID, rawJSON); err != nil {
			slog.Error("failed to write telemetry to InfluxDB", "error", err)
			http.Error(w, "Failed to persist telemetry: "+err.Error(), http.StatusInternalServerError)
			return
		}

		w.WriteHeader(http.StatusAccepted)
		_, _ = w.Write([]byte(`{"status":"accepted"}`))
		slog.Info("telemetry saved to InfluxDB", "node_id", nodeID, "seq", payload.Seq)
	}
}

// LatestHandler returns the most recent telemetry point for the frontend
func (s *IngestionService) LatestHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		s.mu.RLock()
		latest := s.latestPoint
		s.mu.RUnlock()

		if latest == nil {
			http.Error(w, "No telemetry received yet", http.StatusNotFound)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		_ = json.NewEncoder(w).Encode(latest)
	}
}

// ExtractNodeIDFromTopic extracts node id from topic like 'nodes/n1/data' -> 'n1'
func ExtractNodeIDFromTopic(topic string) string {
	parts := strings.Split(topic, "/")
	if len(parts) >= 2 && parts[0] == "nodes" {
		return parts[1]
	}
	return "node_default"
}
