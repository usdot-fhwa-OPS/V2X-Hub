ARG UBUNTU_VERSION=jammy

FROM ubuntu:$UBUNTU_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/install_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/install_dependencies.sh

# build out ext components
COPY ./ext /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN ./build.sh
