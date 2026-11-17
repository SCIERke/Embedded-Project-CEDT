terraform {
  required_providers {
    google = {
      source = "hashicorp/google"
      version = "~> 8.11.0"
    }
  }
}

provider "google" {
  project = var.project_id
  region = var.region
  zone = var.zone
}

resource "google_compute_disk" "data_disk" {
  name = "app-data-disk"
  type = "pd-balanced"
  zone = var.zone
  size = 20 # GB, holds InfluxDB data
}

resource "google_service_account" "default" {
  account_id   = "db-instance"
  display_name = "db-instance"
}

resource "google_compute_instance" "vm" {
  name         = "db-instance"
  machine_type = "e2-medium" # 2 vCPUs, 2GB RAM
  zone         = var.zone # asia-southeast1-a (Singapore)

  boot_disk {
    initialize_params {
      image = "debian-cloud/debian-11"
    }
  }

  attached_disk {
    source      = google_compute_disk.data_disk.id
    device_name = "data"
  }

  network_interface {
    network = "default"

    access_config {
      // Ephemeral public IP
    }
  }

  service_account {
    # Google recommends custom service accounts that have cloud-platform scope and permissions granted via IAM Roles.
    email  = google_service_account.default.email
    scopes = ["cloud-platform"]
  }
}
