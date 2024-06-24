FROM debian:bullseye

WORKDIR /code

RUN apt update && apt install -y build-essential make git golang \
    && rm -rf /var/lib/apt/lists/*

RUN echo 'alias l="ls -la"' >> ~/.bashrc