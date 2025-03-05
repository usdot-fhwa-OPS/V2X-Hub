FROM maven:3.8-jdk-11-slim AS maven_build
RUN mkdir /home/port-drayage-webservice
COPY src/ /home/port-drayage-webservice/src/
COPY pom.xml /home/port-drayage-webservice/
WORKDIR /home/port-drayage-webservice/
RUN mvn clean install
FROM alpine:latest 
RUN mkdir /home/port-drayage-webservice/
RUN apk add openjdk11-jre --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
COPY --from=maven_build /home/port-drayage-webservice/target/port-drayage-webservice-0.0.1-SNAPSHOT.jar /home/port-drayage-webservice/
WORKDIR /home/port-drayage-webservice/

ARG BUILD_DATE="NULL"
ARG VCS_REF="NULL"
ARG VERSION="NULL"
# Set metadata labels
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.name="Port Drayage Web Service"
LABEL org.label-schema.description="Port Drayage Web Service"
LABEL org.label-schema.vendor="Leidos"
LABEL org.label-schema.version="${VERSION}"
LABEL org.label-schema.url="https://highways.dot.gov/research/research-programs/operations"
LABEL org.label-schema.vcs-url="https://github.com/usdot-fhwa-ops/V2X-HUB"
LABEL org.label-schema.vcs-ref=${VCS_REF}
LABEL org.label-schema.build-date=${BUILD_DATE}
ENTRYPOINT ["java","-jar","port-drayage-webservice-0.0.1-SNAPSHOT.jar"]