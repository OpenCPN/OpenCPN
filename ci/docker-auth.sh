#!/bin/bash

if [ -z "$DOCKER_USER" ]; then
    echo 'warning: $DOCKER_USER missing, using unauthenticated docker pulls.'
    exit 0
elif [ -z "$DOCKER_PW" ]; then
    echo 'warning: $DOCKER_PW missing, using unauthenticated docker pulls.'
    exit 0
fi

echo $DOCKER_PW | docker login --username $DOCKER_USER --password-stdin
