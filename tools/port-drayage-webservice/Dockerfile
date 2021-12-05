FROM adoptopenjdk/openjdk11:alpine-jre
RUN mkdir ~/port-drayage-webservice/
COPY ./target/port-drayage-webservice-0.0.1-SNAPSHOT.jar ~/port-drayage-webservice/
WORKDIR ~/port-drayage-webservice/
ENTRYPOINT ["java","-jar","port-drayage-webservice-0.0.1-SNAPSHOT.jar"]