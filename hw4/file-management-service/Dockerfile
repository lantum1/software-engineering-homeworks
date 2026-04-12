FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    libssl-dev \
    libpoco-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /opt
RUN git clone https://github.com/microsoft/vcpkg.git
WORKDIR /opt/vcpkg
RUN ./bootstrap-vcpkg.sh

RUN ./vcpkg install mongo-cxx-driver

WORKDIR /app
COPY . .

RUN mkdir -p build && cd build && \
    cmake .. \
      -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=arm64-linux \
      -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . -j$(nproc)


FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libpoco-dev \
    libssl3 \
    ca-certificates \
    libmongoc-1.0-0 \
    libbson-1.0-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/file-management-service .

EXPOSE 8082

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD wget --quiet --tries=1 --spider http://localhost:8082/v1/folders || exit 1

CMD ["./file-management-service", "--port=8082", "--threads=4"]