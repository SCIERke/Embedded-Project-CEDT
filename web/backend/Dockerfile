FROM golang:1.23-alpine AS build
WORKDIR /src
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN CGO_ENABLED=0 go build -trimpath -ldflags "-s -w" -o /web/backend ./cmd/main.go

FROM gcr.io/distroless/static-debian12:nonroot
COPY --from=build /web/backend /web/backend
EXPOSE 8000
ENTRYPOINT ["/web/backend"]
