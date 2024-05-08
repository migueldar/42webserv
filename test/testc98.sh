#!/bin/bash

docker build . -t pruebas
docker run -it --network=host --rm -v="$PWD:/code" pruebas
