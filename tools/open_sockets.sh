#!/bin/bash
os=$(uname -s)
case $os in
    Linux*)
        netstat -t | grep localhost ; netstat -tl
        ;;
    Darwin*)
        lsof -iTCP -P | grep -E '8080|8081'
        ;;
    *)
        echo "unknown os"
		exit 1
        ;;
esac
