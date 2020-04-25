#!/usr/bin/env bash

#
# Build the travis flatpak artifacts. Uses docker to run Fedora
# in the Travis ubuntu VM; the actual build is done in the Fedora
# container.
#

# bailout on errors and echo commands.
set -xe

sudo apt-key adv \
    --keyserver keyserver.ubuntu.com --recv-keys 78BD65473CB3BD13

sudo apt-get -qq update
if [ -z "$CIRCLE_BUILD_NUM" ]; then
    sudo apt install -y docker docker.io
fi

DOCKER_SOCK="unix:///var/run/docker.sock"

echo "DOCKER_OPTS=\"-H tcp://127.0.0.1:2375 -H $DOCKER_SOCK -s devicemapper\"" \
    | sudo tee /etc/default/docker > /dev/null
sudo service docker restart
sleep 5;
sudo docker pull fedora:31;

set +x
echo "FLATPAK_KEY=$FLATPAK_KEY" > envvars
set -x

docker run --privileged -d -ti -e "container=docker"  \
    --env-file envvars \
    -v /sys/fs/cgroup:/sys/fs/cgroup \
    -v $(pwd):/opencpn-ci:rw \
    fedora:31   /usr/sbin/init
DOCKER_CONTAINER_ID=$(docker ps | grep fedora | awk '{print $1}')
docker logs $DOCKER_CONTAINER_ID
docker exec -ti $DOCKER_CONTAINER_ID /bin/bash -xec \
    "bash -xe /opencpn-ci/ci/generic-build-flatpak.sh 31;
         echo -ne \"------\nEND OPENCPN-CI BUILD\n\";"
docker ps -a
docker stop $DOCKER_CONTAINER_ID
docker rm -v $DOCKER_CONTAINER_ID
