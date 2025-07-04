ARG UBUNTU_VERSION=jammy-20230126

FROM ubuntu:$UBUNTU_VERSION AS build-environment
ARG J2735_VERSION=2016
ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/install_dependencies.sh /usr/local/bin/
RUN sed -i 's|http://archive.ubuntu.com|http://us.archive.ubuntu.com|g' /etc/apt/sources.list
RUN /usr/local/bin/install_dependencies.sh

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
FROM build-environment AS dependencies
RUN ./build.sh release --j2735-version $J2735_VERSION
RUN ldconfig

# run final image
FROM ubuntu:$UBUNTU_VERSION AS v2xhub
ENV DEBIAN_FRONTEND=noninteractive
ADD scripts/deployment_dependencies.sh /usr/local/bin/
RUN /usr/local/bin/deployment_dependencies.sh

COPY ./container /home/V2X-Hub/container/
WORKDIR /home/V2X-Hub/container/
RUN ./database.sh
RUN ./library.sh
RUN ldconfig

COPY --from=dependencies /usr/local/plugins/ /usr/local/plugins/
COPY --from=dependencies /usr/local/include/ /usr/local/include/
COPY --from=dependencies /usr/local/lib/ /usr/local/lib/
COPY --from=dependencies /usr/local/bin/ /usr/local/bin/
COPY --from=dependencies /usr/lib/ /usr/lib/
COPY --from=dependencies /usr/bin/ /usr/bin/
COPY --from=dependencies /usr/local/share/ /usr/local/share/
COPY --from=dependencies /var/www/plugins/ /var/www/plugins/
COPY --from=dependencies /var/log/tmx/ /var/log/tmx/
COPY --from=dependencies /opt/ /opt/
ADD src/tmx/TmxCore/tmxcore.service /lib/systemd/system/
ADD src/tmx/TmxCore/tmxcore.service /usr/sbin/
RUN ldconfig

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
