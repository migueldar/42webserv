#!/bin/bash

docker build $(dirname $0) -t pruebas
docker run -it --rm -v=$PWD:/code pruebas
