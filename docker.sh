#!/bin/bash

set -e

IMAGE_NAME="gluc-local"
CONTAINER_NAME="gluc-container"

function show_help() {
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  build     Build the Docker image"
    echo "  run       Run the Docker container interactively"
    echo "  compile   Compile a Glu source file"
    echo "  test      Run unit tests in container"
    echo "  clean     Remove Docker image and containers"
    echo "  help      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 build                    # Build the Docker image"
    echo "  $0 run                      # Start interactive shell in container"
    echo "  $0 compile file.glu         # Compile a Glu file"
    echo "  $0 test                     # Run unit tests"
}

function build_image() {
    echo "Building Docker image: $IMAGE_NAME"
    docker build -t "$IMAGE_NAME" .
    echo "Build completed successfully!"
}

function run_container() {
    echo "Starting interactive container..."
    docker run -it --rm \
        --name "$CONTAINER_NAME" \
        -v "$(pwd):/workspace" \
        -w /workspace \
        "$IMAGE_NAME" \
        /bin/bash
}

function compile_file() {
    if [ -z "$1" ]; then
        echo "Error: Please specify a Glu source file to compile"
        echo "Usage: $0 compile <file.glu>"
        exit 1
    fi

    if [ ! -f "$1" ]; then
        echo "Error: File '$1' not found"
        exit 1
    fi

    echo "Compiling $1 using Glu compiler in Docker..."
    docker run --rm \
        -v "$(pwd):/workspace" \
        -w /workspace \
        "$IMAGE_NAME" \
        gluc "$1"
}

function run_tests() {
    echo "Running unit tests in Docker container..."
    docker run --rm \
        "$IMAGE_NAME" \
        lit -v /app/test/functional
}

function clean_docker() {
    echo "Cleaning up Docker containers and images..."

    # Stop and remove container if running
    if docker ps -q -f name="$CONTAINER_NAME" | grep -q .; then
        echo "Stopping container $CONTAINER_NAME..."
        docker stop "$CONTAINER_NAME"
    fi

    # Remove container if exists
    if docker ps -aq -f name="$CONTAINER_NAME" | grep -q .; then
        echo "Removing container $CONTAINER_NAME..."
        docker rm "$CONTAINER_NAME"
    fi

    # Remove image if exists
    if docker images -q "$IMAGE_NAME" | grep -q .; then
        echo "Removing image $IMAGE_NAME..."
        docker rmi "$IMAGE_NAME"
    fi

    echo "Cleanup completed!"
}

# Main script logic
case "${1:-help}" in
    build)
        build_image
        ;;
    run)
        run_container
        ;;
    compile)
        compile_file "$2"
        ;;
    test)
        run_tests
        ;;
    clean)
        clean_docker
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        echo "Error: Unknown command '$1'"
        echo ""
        show_help
        exit 1
        ;;
esac
