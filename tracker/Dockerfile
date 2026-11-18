FROM python:3.13-slim
RUN apt-get update && apt-get install -y --no-install-recommends libgl1 libglib2.0-0 \
    && rm -rf /var/lib/apt/lists/*
COPY --from=ghcr.io/astral-sh/uv:0.8 /uv /usr/local/bin/uv
WORKDIR /app
COPY pyproject.toml .
# CPU-only torch: the default Linux wheel drags in ~2 GB of CUDA libs
RUN uv pip install --system --no-cache --torch-backend=cpu -r pyproject.toml
RUN python -c "from ultralytics import YOLO; YOLO('yolo11n.pt')"
COPY track.py .
ENV SHOW=0
CMD ["python", "track.py"]
