"""Track one person and publish the aim error over MQTT (if MQTT_HOST is set). Press q to quit the window.

Env: SOURCE (0 = device camera, or a stream URL / video file), SHOW (1/0), MQTT_HOST, MQTT_PORT, MQTT_TOPIC
"""
import json
import os
from ultralytics import YOLO

SOURCE = os.getenv("SOURCE", "0")
SOURCE = int(SOURCE) if SOURCE.isdigit() else SOURCE
SHOW = os.getenv("SHOW", "1") == "1"
MQTT_HOST = os.getenv("MQTT_HOST")
MQTT_TOPIC = os.getenv("MQTT_TOPIC", "gimbal/target")
AIM_Y = 0.2
LOST_FRAMES = 30  # frames without the locked person before picking a new target

publish = None
if MQTT_HOST:
    import paho.mqtt.client as mqtt
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.connect_async(MQTT_HOST, int(os.getenv("MQTT_PORT", "1883")))  # retries in the background if the broker is down
    client.loop_start()
    publish = lambda msg: client.publish(MQTT_TOPIC, msg)

model = YOLO("yolo11n.pt")  # baked into the image / auto-downloads on first run

target, lost = None, 0
for r in model.track(source=SOURCE, classes=[0], tracker="bytetrack.yaml", stream=True, show=SHOW, verbose=False):
    if r.boxes.id is None:
        ids, boxes = [], []
    else:
        ids, boxes = r.boxes.id.int().tolist(), r.boxes.xywh.tolist()

    if target not in ids:
        lost += 1
        if target is None or lost > LOST_FRAMES:  # (re)lock onto the largest person
            target, lost = (ids[max(range(len(ids)), key=lambda i: boxes[i][2] * boxes[i][3])], 0) if ids else (None, 0)
        continue
    lost = 0

    h, w = r.orig_shape
    x, y, bw, bh = boxes[ids.index(target)]
    ex = (x - w / 2) / (w / 2)
    ey = (y - bh / 2 + AIM_Y * bh - h / 2) / (h / 2)
    print(f"id={target} ex={ex:+.2f} ey={ey:+.2f}", flush=True)
    if publish:
        publish(json.dumps({"id": target, "ex": round(ex, 3), "ey": round(ey, 3)}))
