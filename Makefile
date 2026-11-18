-include .env
PROJECT ?= $(GOOGLE_CLOUD_PROJECT)
ZONE    ?= asia-southeast1-a
TF      := terraform -chdir=infra/terraform/dev
TFVARS  := -var project_id=$(PROJECT) -var zone=$(ZONE)

.PHONY: check-env tf-plan tf-apply tf-destroy firmware-secrets gimbal tracker-dev

# ---- Development Commands ----
dev:

# Tracker dev mode: laptop webcam, runs on the host (Docker on macOS can't reach the camera).
# Starts the MQTT broker, then tracks and publishes to it. make tracker-dev SOURCE=1 for another camera, or a video file.
SOURCE ?= 0
tracker-dev:
	docker compose up -d mqtt-broker
	cd tracker && SOURCE=$(SOURCE) MQTT_HOST=localhost uv run track.py


# ---- Production Commands ----

check-env:
	@case "$(PROJECT)" in ""|"<your-project-id>") echo "Set GOOGLE_CLOUD_PROJECT in .env (cp .env.example .env)"; exit 1;; esac

tf-plan: check-env
	$(TF) plan $(TFVARS)

tf-apply: check-env
	$(TF) apply $(TFVARS)

tf-destroy: check-env ## deletes the VM AND the data disk
	$(TF) destroy $(TFVARS)

# ---- Firmware ----
# The ESP32 can't read .env at runtime, so bake WIFI_* and MQTT_* from .env into a gitignored header.
firmware-secrets:
	@set -a; . ./.env; set +a; \
	[ -n "$$WIFI_SSID" ] || { echo "Set WIFI_SSID and WIFI_PASSWORD in .env"; exit 1; }; \
	[ -n "$$MQTT_HOST" ] || { echo "Set MQTT_HOST in .env"; exit 1; }; \
	printf '#pragma once\n#define WIFI_SSID "%s"\n#define WIFI_PASSWORD "%s"\n#define MQTT_HOST "%s"\n#define MQTT_PORT %s\n#define MQTT_USER "%s"\n#define MQTT_PASSWORD "%s"\n' \
		"$$WIFI_SSID" "$$WIFI_PASSWORD" "$$MQTT_HOST" "$${MQTT_PORT:-1883}" "$$MQTT_USER" "$$MQTT_PASSWORD" > firmware/common/src/secrets.h

BOARD ?= esp32:esp32:esp32
gimbal: firmware-secrets
	arduino-cli compile --library firmware/common -b $(BOARD) firmware/gimbal
	@[ -z "$(PORT)" ] || arduino-cli upload -p $(PORT) -b $(BOARD) firmware/gimbal   # make gimbal PORT=/dev/cu.usbserial-xxx

# start-backend:
  # Some code here
