ARG UBUNTU_VERSION=jammy

FROM ubuntu:$UBUNTU_VERSION AS build-environment
ARG SKIP_PLUGINS=""
ARG J2735_VERSION=2024
ENV DEBIAN_FRONTEND=noninteractive
COPY scripts/install_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/install_dependencies.sh

# build out ext components
COPY ./ext /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN ./build.sh

COPY container/wait-for-it.sh /usr/local/bin/
COPY container/service.sh /usr/local/bin/

COPY ./container /home/V2X-Hub/container

# build internal components

COPY ./src /home/V2X-Hub/src/
WORKDIR /home/V2X-Hub/src/
FROM build-environment AS dependencies
RUN /home/V2X-Hub/container/database.sh
RUN /home/V2X-Hub/container/library.sh
RUN ldconfig
RUN ./build.sh release --j2735-version "$J2735_VERSION" --skip-plugins "${SKIP_PLUGINS}"
RUN ldconfig

# run final image
FROM ubuntu:$UBUNTU_VERSION AS v2xhub
ENV DEBIAN_FRONTEND=noninteractive
COPY scripts/deployment_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/deployment_dependencies.sh

COPY --chown=plugin:adm --chmod=644 ./container /home/V2X-Hub/container/
WORKDIR /home/V2X-Hub/container/
RUN ./database.sh && ./library.sh && ldconfig
# Built Plugins
COPY --from=dependencies --chown=plugin:adm --chmod=644 /usr/local/plugins/ /usr/local/plugins/
# Headers
COPY --from=dependencies  --chown=plugin:adm --chmod=644 /usr/local/include/ /usr/local/include/
# Built Libraries for V2X Hub (tmx services) and ext/ (snmp, etc)
COPY --from=dependencies  --chown=plugin:adm --chmod=644 /usr/local/lib/ /usr/local/lib/
# Built Binaries for V2X Hub (tmx cli ) and ext/ (snmpget cli, etc)
COPY --from=dependencies  --chown=plugin:adm --chmod=644 /usr/local/bin/ /usr/local/bin/
# CMake config iles
COPY --from=dependencies --chown=plugin:adm --chmod=644 /usr/local/share/ /usr/local/share/
COPY --from=dependencies --chown=plugin:adm --chmod=644 /var/www/plugins/ /var/www/plugins/
# Installed STOL debian packages like (stol-j2735, timesync, etc)
COPY --from=dependencies /opt/ /opt/
COPY src/tmx/TmxCore/tmxcore.service /lib/systemd/system/
COPY src/tmx/TmxCore/tmxcore.service /usr/sbin/
RUN ldconfig


WORKDIR /var/log/tmx
# Non root default user
USER plugin
# Build arges for Open Containers Annotations (https://specs.opencontainers.org/image-spec/annotations/)
ARG VCS_REF
ARG BUILD_DATE
ARG VERSION
ARG UBUNTU_VERSION

# Set metadata labels
LABEL org.opencontainers.image.title="V2X-Hub"
LABEL org.opencontainers.image.description="Image V2X-Hub"
LABEL org.opencontainers.image.vendor="Leidos"
LABEL org.opencontainers.image.version=${VERSION}
LABEL org.opencontainers.image.url="https://highways.dot.gov/research/research-programs/operations"
LABEL org.opencontainers.image.source="https://github.com/usdot-fhwa-ops/V2X-HUB"
LABEL org.opencontainers.image.revision=${VCS_REF}
LABEL org.opencontainers.image.created=${BUILD_DATE}
LABEL org.opencontainers.image.base.name="docker.io/ubuntu:${UBUNTU_VERSION}"

ENTRYPOINT ["/usr/local/bin/service.sh"]