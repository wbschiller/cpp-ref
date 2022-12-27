# syntax=docker/dockerfile:1
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    build-essential \
    clang-format \
    clang-tidy \
    cmake \
    git \
    && apt-get clean
CMD ["bash"]
