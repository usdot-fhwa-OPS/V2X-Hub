ARG UBUNTU_VERSION=focal-20220113

FROM ubuntu:$UBUNTU_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/install_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/install_dependencies.sh

# build out ext components
COPY ./ext /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN ./build.sh

COPY ./container /home/V2X-Hub/container
WORKDIR /home/V2X-Hub/container/
RUN ./database.sh
RUN ./library.sh
RUN ldconfig

# build internal components
COPY ./src /home/V2X-Hub/src/
WORKDIR /home/V2X-Hub/src/
RUN ./build.sh release
RUN ldconfig

# package plugins
WORKDIR /home/V2X-Hub/src/v2i-hub/
RUN ./package_plugins.sh

RUN /home/V2X-Hub/container/setup.sh
WORKDIR /home/V2X-Hub/src/v2i-hub/

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

ENTRYPOINT ["/home/V2X-Hub/container/service.sh"]
