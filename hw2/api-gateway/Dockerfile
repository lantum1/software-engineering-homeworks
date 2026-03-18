FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libpoco-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN mkdir build
WORKDIR /app/build

RUN cmake ..
RUN cmake --build . --config Release

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpoco-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/max-api-gateway .

COPY --from=builder /app/config ./config

EXPOSE 8080

CMD ["./max-api-gateway"]