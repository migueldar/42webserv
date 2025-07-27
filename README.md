# webserv

A single-threaded HTTP/1.1 web server written in C++, developed as part of the 42 School curriculum. This project focuses on building a compliant, efficient and configurable server from scratch, without relying on external libraries for networking or HTTP parsing. It supports multiple simultaneous connections using the `poll()` system call.

## Features

- HTTP/1.1 support with persistent connections
- Static file serving
- CGI execution (e.g. Python scripts)
- Configurable via `.conf` file (ports, routes, error pages, etc.)
- Proper handling of HTTP methods: `GET`, `POST`, `DELETE`
- Autoindexing for directories
- File uploading
