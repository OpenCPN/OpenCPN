# Container images for OpenCPN

## Builder image

Can be used to build OpenCPN in a container. All the dependencies are installed in the image.

### Building the image

From the root directory of OpenCPN source tree execute `podman build -f docker/Dockerfile.builder -t opencpn-builder .`

To build the image for a set of different architectures

```sh
podman build -f docker/Dockerfile.builder -t opencpn-builder --platform=linux/amd64,linux/arm64 .
```

### Using the image

Clone the OpenCPN source to your machine and mount it to the container as a volume

```sh
podman run -v <Path to your OpenCPN source tree>:/src:Z -it opencpn-builder:latest bash
```

To use the image for builds targetting other architecture, use the `--arch` parameter

```sh
podman run --arch arm64 -v <Path to your OpenCPN source tree>:/src:Z -it opencpn-builder:latest bash
```

Now you can follow the normal Linux build instructions
