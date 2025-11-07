# Multi-stage build to minimize final image size
FROM ubuntu:22.04 AS builder

# Avoid interactive prompts
ARG DEBIAN_FRONTEND=noninteractive

# Install base dependencies and add LLVM repository
RUN apt-get update && apt-get install -y \
    build-essential \
    flex \
    bison \
    git \
    wget \
    gnupg \
    software-properties-common \
    lsb-release \
    ca-certificates \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-20 main" >> /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y \
    clang-20 \
    llvm-20-dev \
    llvm-20-tools \
    lld-20 \
    libclang-20-dev \
    cargo \
    && rm -rf /var/lib/apt/lists/*

# Install modern CMake from Kitware repository
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null \
    && echo 'deb https://apt.kitware.com/ubuntu/ jammy main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

# Create symbolic links for LLVM tools
RUN ln -sf /usr/bin/clang-20 /usr/bin/clang++ && \
    ln -sf /usr/bin/llvm-config-20 /usr/bin/llvm-config && \
    ln -sf /usr/bin/llvm-profdata-20 /usr/bin/llvm-profdata && \
    ln -sf /usr/bin/llvm-cov-20 /usr/bin/llvm-cov

WORKDIR /app

# Copy source code
COPY . .

# Configure CMake with same options as coverage.sh
RUN cmake -Bbuild \
    -DLLVM_ENABLE_ASSERTIONS=1 \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="X86" \
    -DENABLE_ASAN=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DFROM_SOURCE=0 \
    -DCMAKE_CXX_COMPILER=clang++

# Build the project
RUN cmake --build build -j$(nproc)

# Stage 2: Minimal runtime image
FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    lsb-release \
    && wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - \
    && echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-20 main" >> /etc/apt/sources.list.d/llvm.list \
    && apt-get update \
    && apt-get install -y \
    clang-20 \
    llvm-20-runtime \
    llvm-20-tools \
    libc6-dev \
    && rm -rf /var/lib/apt/lists/*

# Create symbolic links for runtime tools
RUN ln -sf /usr/bin/clang-20 /usr/bin/clang && \
    ln -sf /usr/bin/clang-20 /usr/bin/clang++ && \
    ln -sf /usr/bin/llvm-profdata-20 /usr/bin/llvm-profdata && \
    ln -sf /usr/bin/llvm-cov-20 /usr/bin/llvm-cov

# Create non-root user
RUN useradd -m -s /bin/bash glu

WORKDIR /app

# Copy compiled binaries from builder stage
COPY --from=builder /app/build/tools/gluc/gluc /usr/local/bin/
COPY --from=builder /app/build/tools/glu-demangle/glu-demangle /usr/local/bin/
COPY --from=builder /app/build/test/unit_tests /app/
# Copy shared libraries directly to /usr/local/lib
COPY --from=builder /app/build/lib/*/*.so /usr/local/lib/
COPY --from=builder /app/build/tools/lib/ /usr/local/lib/
COPY --from=builder /app/test/functional/ /app/test/functional/

# Update dynamic library cache
RUN ldconfig

# Set permissions
RUN chown -R glu:glu /app && \
    chmod +x /usr/local/bin/gluc

# Switch to non-root user
USER glu

# Default command
CMD ["gluc", "--help"]

# Metadata labels
LABEL maintainer="Glu Language Team"
LABEL description="Glu Programming Language - A modern glue language for connecting and interoperating with other languages via LLVM"
LABEL version="latest"
