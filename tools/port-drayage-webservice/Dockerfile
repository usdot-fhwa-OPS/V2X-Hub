FROM maven:3.8-jdk-11-slim AS maven_build
RUN mkdir /home/port-drayage-webservice
COPY src/ /home/port-drayage-webservice/src/
COPY pom.xml /home/port-drayage-webservice/
WORKDIR /home/port-drayage-webservice/
RUN mvn clean install
FROM alpine:20250108 
RUN mkdir /home/port-drayage-webservice/
RUN apk add openjdk11-jre --repository=http://dl-cdn.alpinelinux.org/alpine/edge/community
COPY --from=maven_build /home/port-drayage-webservice/target/port-drayage-webservice-0.0.1-SNAPSHOT.jar /home/port-drayage-webservice/
WORKDIR /home/port-drayage-webservice/
ENTRYPOINT ["java","-jar","port-drayage-webservice-0.0.1-SNAPSHOT.jar"]
