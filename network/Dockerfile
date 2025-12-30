# syntax=docker/dockerfile:1

############################
# 1) Builder stage
############################
FROM ubuntu:24.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG BOOST_VERSION=1.90.0
# BOOST_VERSION with underscores for tarball path
ARG BOOST_VERSION_U=1_90_0

# Install build tools + download tools + OpenSSL dev headers
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    git \
    curl \
    wget \
    tar \
    xz-utils \
    pkg-config \
    build-essential \
    cmake \
    ninja-build \
    libssl-dev \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /deps

# Download and extract Boost sources (only used in builder stage)
RUN wget -q https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_U}.tar.gz \
 && tar -xzf boost_${BOOST_VERSION_U}.tar.gz \
 && rm boost_${BOOST_VERSION_U}.tar.gz

# Copy project
WORKDIR /src
COPY . .

# Configure & build
# Pass BOOST_ROOT so CMake can find headers for our boost_charconv build
RUN cmake -S . -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DBOOST_ROOT=/deps/boost_${BOOST_VERSION_U} \
 && cmake --build build --config Release

############################
# 2) Runtime stage (small)
############################
FROM ubuntu:24.04 AS runtime

ARG DEBIAN_FRONTEND=noninteractive

# Only runtime libs (OpenSSL runtime). If your binary is fully static, you may not need these.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libssl3 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the built binary
COPY --from=builder /src/build/lynks_backend /usr/local/bin/lynks_backend

# (Optional) if your app needs runtime files/configs, copy them too:
# COPY --from=builder /src/secret /app/secret

# Expose port if relevant (change/remove)
ENV PORT=60000
EXPOSE 60000

# Start server
ENTRYPOINT ["/usr/local/bin/lynks_backend"]