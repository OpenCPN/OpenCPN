FROM debian:bookworm-slim

COPY ci/control /tmp
RUN apt update && \
    apt install -y devscripts equivs ninja-build && \
    yes | mk-build-deps -i -r tmp/control && \
    apt-get --allow-unauthenticated install -f -y && \
    apt clean && \
    rm -f /tmp/control && \
    mkdir /src
WORKDIR /src
