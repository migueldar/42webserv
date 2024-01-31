FROM debian:bullseye

RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN apt-get update && apt-get install -y nginx procps 

EXPOSE 8080

# CMD ["tail", "-f", "/dev/null"]
CMD sleep infinity