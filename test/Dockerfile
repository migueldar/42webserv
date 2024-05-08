FROM debian:bullseye

WORKDIR /code

RUN apt update && apt install -y build-essential make
RUN echo 'alias l="ls -la"' >> ~/.bashrc

RUN git clone https://go.googlesource.com/go goroot