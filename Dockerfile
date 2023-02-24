ARG UBUNTU_VERSION=jammy-20230126

FROM ubuntu:$UBUNTU_VERSION

ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/install_dependencies.sh /usr/local/bin/
# key for for "ports" for jammy
RUN apt-get -y install gpgv && apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 871920D1991BC93C
RUN sed -i 's|http://archive.ubuntu.com|http://us.archive.ubuntu.com|g' /etc/apt/sources.list && \
    /usr/local/bin/install_dependencies.sh

# build out ext components
COPY ./ext /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN ./build.sh

ADD container/wait-for-it.sh /usr/local/bin/
ADD container/service.sh /usr/local/bin/
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

WORKDIR /home/V2X-Hub/
RUN /home/V2X-Hub/container/setup.sh
WORKDIR /var/log/tmx

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
