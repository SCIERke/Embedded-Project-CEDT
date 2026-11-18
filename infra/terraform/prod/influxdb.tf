module "influxdb" {
  source  = "influxdata/influxdb/aws"
  version = "1.0.0"

  data_instances = 1
  # Set meta_instances to zero for OSS usage :)
  meta_instances  = 0
  ami             = "ami-6d48500b"
  tags            = { Environment = "production" }
  subnet_ids      = ["subnet-9d4a7b6c", "subnet-7d4a9b6c"]
  vpc_id          = "vpc-6589ba03"
  instance_type   = "t3.small"
  key_name        = "deployer"
  zone_id         = "Z4RIKDGDIXUWGT"
  security_groups = ["sg-fffa2187"]
}
