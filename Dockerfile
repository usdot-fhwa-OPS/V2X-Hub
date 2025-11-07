ARG UBUNTU_VERSION=jammy

FROM ubuntu:$UBUNTU_VERSION AS build-environment
ARG SKIP_PLUGINS=""
ARG J2735_VERSION=2024
ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/install_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/install_dependencies.sh

# build out ext components
COPY ./ext /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN ./build.sh

ADD container/wait-for-it.sh /usr/local/bin/
ADD container/service.sh /usr/local/bin/

COPY ./container /home/V2X-Hub/container

# build internal components

COPY ./src /home/V2X-Hub/src/
WORKDIR /home/V2X-Hub/src/
FROM build-environment AS dependencies
RUN /home/V2X-Hub/container/database.sh
RUN /home/V2X-Hub/container/library.sh
RUN ldconfig
RUN ./build.sh release --j2735-version $J2735_VERSION --skip-plugins "${SKIP_PLUGINS}"
RUN ldconfig

# run final image
FROM ubuntu:$UBUNTU_VERSION AS v2xhub
ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/deployment_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/deployment_dependencies.sh

COPY --chown=plugin:adm ./container /home/V2X-Hub/container/
WORKDIR /home/V2X-Hub/container/
RUN ./database.sh
RUN ./library.sh
RUN ldconfig
# Built Plugins
COPY --from=dependencies --chown=plugin:adm /usr/local/plugins/ /usr/local/plugins/
# Headers
COPY --from=dependencies  --chown=plugin:adm /usr/local/include/ /usr/local/include/
# Built Libraries for V2X Hub (tmx services) and ext/ (snmp, etc)
COPY --from=dependencies  --chown=plugin:adm /usr/local/lib/ /usr/local/lib/
# Built Binaries for V2X Hub (tmx cli ) and ext/ (snmpget cli, etc)
COPY --from=dependencies  --chown=plugin:adm /usr/local/bin/ /usr/local/bin/
# CMake config iles
COPY --from=dependencies --chown=plugin:adm /usr/local/share/ /usr/local/share/
COPY --from=dependencies --chown=plugin:adm /var/www/plugins/ /var/www/plugins/
# Installed STOL debian packages like (stol-j2735, timesync, etc)
COPY --from=dependencies /opt/ /opt/
ADD src/tmx/TmxCore/tmxcore.service /lib/systemd/system/
ADD src/tmx/TmxCore/tmxcore.service /usr/sbin/
RUN ldconfig


WORKDIR /var/log/tmx
# TODO: create a non-root user to run the services. Currently 
# PluginMonitor needs user with root permissions
USER plugin

# Set metadata labels
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.name="V2X-Hub-Deployment"
LABEL org.label-schema.description="Image V2X-Hub Deployment"
LABEL org.label-schema.vendor="Leidos"
LABEL org.label-schema.version="${VERSION}"
LABEL org.label-schema.url="https://highways.dot.gov/research/research-programs/operations"
LABEL org.label-schema.vcs-url="https://github.com/usdot-fhwa-ops/V2X-HUB"
LABEL org.label-schema.vcs-ref=${VCS_REF}
LABEL org.label-schema.build-date=${BUILD_DATE}

ENTRYPOINT ["/usr/local/bin/service.sh"]
