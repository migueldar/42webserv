#!/bin/bash

docker build $(dirname $0) -t pruebas
docker run -it --rm -p 9090:9090 -v $PWD/$(dirname $0)/conf:/conf pruebas bash