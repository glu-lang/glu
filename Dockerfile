# Multi-stage build to minimize final image size
FROM ubuntu:25.04 AS builder

# Avoid interactive prompts
ARG DEBIAN_FRONTEND=noninteractive
ARG RUST_VERSION=1.90.0

ENV RUSTUP_HOME=/opt/rustup \
    CARGO_HOME=/opt/cargo \
    PATH=/opt/cargo/bin:$PATH

# Install base dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    flex \
    bison \
    git \
    wget \
    curl \
    gnupg \
    software-properties-common \
    lsb-release \
    ca-certificates

# Install LLVM 20 via official script
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 20 all \
    && rm -rf /var/lib/apt/lists/*

# Install pinned Rust toolchain via rustup for compatibility with LLVM 20
RUN curl -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain ${RUST_VERSION} --profile minimal && \
    if [ -d /root/.cargo ]; then \
        mv /root/.cargo /opt/cargo && \
        mv /root/.rustup /opt/rustup; \
    fi && \
    /opt/cargo/bin/rustc --version

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
    -DDOCKER_BUILD=ON \
    -DCMAKE_CXX_COMPILER=clang++

# Build the project
RUN cmake --build build -j$(nproc) --target gluc glu-demangle stdlib

# Stage 2: Minimal runtime image
FROM ubuntu:25.04

ARG DEBIAN_FRONTEND=noninteractive
ARG RUST_VERSION=1.90.0

ENV RUSTUP_HOME=/opt/rustup \
    CARGO_HOME=/opt/cargo \
    PATH=/opt/cargo/bin:$PATH

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    wget \
    curl \
    python3-pip \
    gnupg \
    software-properties-common \
    lsb-release

# Install LLVM 20 via official script
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 20 all && \
    rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -s /bin/bash glu

# Install pinned Rust toolchain via rustup for compatibility with LLVM 20
RUN curl -sSf https://sh.rustup.rs | sh -s -- -y --default-toolchain ${RUST_VERSION} --profile minimal && \
    if [ -d /root/.cargo ]; then \
        mv /root/.cargo /home/glu/.cargo && \
        mv /root/.rustup /home/glu/.rustup; \
    fi && \
    /opt/cargo/bin/rustc --version

# Install lit testing tool
RUN pip install lit --break-system-packages

# Create symbolic links for runtime tools
RUN ln -sf /usr/bin/clang-20 /usr/bin/clang && \
    ln -sf /usr/bin/clang-20 /usr/bin/clang++ && \
    ln -sf /usr/bin/llvm-profdata-20 /usr/bin/llvm-profdata && \
    ln -sf /usr/bin/llvm-cov-20 /usr/bin/llvm-cov && \
    ln -sf /usr/bin/lldb-20 /usr/bin/lldb

# Create symbolic links for Rust tools to make them globally available
RUN ln -sf /opt/cargo/bin/rustc /usr/local/bin/rustc && \
    ln -sf /opt/cargo/bin/cargo /usr/local/bin/cargo && \
    ln -sf /opt/cargo/bin/rustup /usr/local/bin/rustup

WORKDIR /app

# Copy compiled binaries from builder stage
COPY --from=builder /app/build/tools/gluc/gluc /usr/local/bin/
COPY --from=builder /app/build/tools/glu-demangle/glu-demangle /usr/local/bin/

# Copy shared libraries directly to /usr/local/lib
COPY --from=builder /app/build/lib/*/*.so /usr/local/lib/
COPY --from=builder /app/build/tools/lib/ /usr/local/lib/
COPY --from=builder /app/test/functional/ /app/test/functional/

# Update dynamic library cache
RUN ldconfig

# Set permissions
RUN chown -R glu:glu /app /opt/cargo /opt/rustup && \
    chmod +x /usr/local/bin/gluc

# Switch to non-root user
USER glu

# Default command
CMD ["gluc", "--help"]

# Metadata labels
LABEL maintainer="Glu Language Team"
LABEL description="Glu Programming Language - A modern glue language for connecting and interoperating with other languages via LLVM"
LABEL version="latest"
