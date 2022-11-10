FROM ubuntu:focal-20220113

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update  && apt-get install -y sudo cmake gcc-7 g++-7 libboost1.71-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev gpsd libgps-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv1-dev git vim zip build-essential zlib1g libssl-dev qtbase5-dev qtbase5-dev-tools curl libqhttpengine-dev libgtest-dev libcpprest-dev librdkafka-dev

WORKDIR cd /usr/src/googletest/googletest
RUN mkdir ~/build
WORKDIR /usr/src/googletest/googletest/build
RUN cmake ..
RUN make
RUN cd lib && cp libgtest* /usr/lib/
WORKDIR cd /usr/src/googletest/googletest
RUN rm -rf build
RUN mkdir /usr/local/lib/googletest
RUN ln -s /usr/lib/libgtest.a /usr/local/lib/googletest/libgtest.a
RUN ln -s /usr/lib/libgtest_main.a /usr/local/lib/googletest/libgtest_main.a
RUN ldconfig

RUN mkdir ~/V2X-Hub
COPY . /home/V2X-Hub
WORKDIR /home/V2X-Hub/src/tmx/
RUN cmake .
RUN make
RUN make install

WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/library.sh
RUN /home/V2X-Hub/container/library.sh
RUN ldconfig

WORKDIR /home/V2X-Hub/
RUN mkdir -p /home/V2X-Hub/ext
WORKDIR /home/V2X-Hub/ext/
RUN git clone https://github.com/usdot-fhwa-OPS/libwebsockets.git
WORKDIR /home/V2X-Hub/ext/libwebsockets/
RUN cmake -DLWS_WITH_SHARED=OFF .
RUN make
RUN make install

WORKDIR /home/V2X-Hub/ext
RUN git clone https://github.com/usdot-fhwa-OPS/qhttpengine.git
WORKDIR /home/V2X-Hub/ext/qhttpengine
RUN cmake .
RUN make 
RUN make install

WORKDIR /home/V2X-Hub/ext/ 
RUN git clone https://github.com/HowardHinnant/date.git
WORKDIR /home/V2X-Hub/ext/date
RUN cmake .
RUN make
RUN make install
RUN ldconfig 

WORKDIR /home/V2X-Hub/ext/server
RUN cmake .
RUN make
RUN make install

WORKDIR /home/V2X-Hub/ext/ccserver
RUN cmake . 
RUN make
RUN make install

WORKDIR /home/V2X-Hub/ext/pdclient
RUN cmake .
RUN make
RUN make install 

### setup and install v2x-hub core and plugins 

WORKDIR /home/V2X-Hub/src/v2i-hub/
RUN cmake . -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake -Dv2xhubWebAPI_DIR=/usr/local/share/v2xhubWebAPI/cmake/
RUN make


RUN ln -s ../bin CommandPlugin/bin
RUN zip CommandPlugin.zip CommandPlugin/bin/CommandPlugin CommandPlugin/manifest.json
RUN ln -s ../bin CswPlugin/bin
RUN zip CswPlugin.zip CswPlugin/bin/CswPlugin CswPlugin/manifest.json
RUN ln -s ../bin DmsPlugin/bin
RUN zip DmsPlugin.zip DmsPlugin/bin/DmsPlugin DmsPlugin/manifest.json
RUN ln -s ../bin DsrcImmediateForwardPlugin/bin
RUN zip DsrcImmediateForwardPlugin.zip DsrcImmediateForwardPlugin/bin/DsrcImmediateForwardPlugin DsrcImmediateForwardPlugin/manifest.json
RUN ln -s ../bin LocationPlugin/bin
RUN zip LocationPlugin.zip LocationPlugin/bin/LocationPlugin LocationPlugin/manifest.json
RUN ln -s ../bin MapPlugin/bin
RUN zip MapPlugin.zip MapPlugin/bin/MapPlugin MapPlugin/manifest.json
RUN ln -s ../bin MessageReceiverPlugin/bin
RUN zip MessageReceiverPlugin.zip MessageReceiverPlugin/bin/MessageReceiverPlugin MessageReceiverPlugin/manifest.json
RUN ln -s ../bin ODEPlugin/bin
RUN zip ODEPlugin.zip ODEPlugin/bin/ODEPlugin ODEPlugin/manifest.json
RUN ln -s ../bin RtcmPlugin/bin
RUN zip RtcmPlugin.zip RtcmPlugin/bin/RtcmPlugin RtcmPlugin/manifest.json
RUN ln -s ../bin SpatPlugin/bin
RUN zip SpatPlugin.zip SpatPlugin/bin/SpatPlugin SpatPlugin/manifest.json
RUN ln -s ../bin PreemptionPlugin/bin
RUN zip PreemptionPlugin.zip PreemptionPlugin/bin/PreemptionPlugin PreemptionPlugin/manifest.json
RUN ln -s ../bin SPaTLoggerPlugin/bin
RUN zip SPaTLoggerPlugin.zip SPaTLoggerPlugin/bin/SPaTLoggerPlugin SPaTLoggerPlugin/manifest.json
RUN ln -s ../bin MessageLoggerPlugin/bin
RUN zip MessageLoggerPlugin.zip MessageLoggerPlugin/bin/MessageLoggerPlugin MessageLoggerPlugin/manifest.json
RUN ln -s ../bin PedestrianPlugin/bin
RUN zip PedestrianPlugin.zip PedestrianPlugin/bin/PedestrianPlugin PedestrianPlugin/manifest.json
RUN ln -s ../bin TimPlugin/bin
RUN zip TimPlugin.zip TimPlugin/bin/TimPlugin TimPlugin/manifest.json
RUN ln -s ../bin CARMACloudPlugin/bin
RUN zip CARMACloudPlugin.zip CARMACloudPlugin/bin/CARMACloudPlugin CARMACloudPlugin/manifest.json
RUN ln -s ../bin PortDrayagePlugin/bin
RUN zip PortDrayagePlugin.zip PortDrayagePlugin/bin/PortDrayagePlugin PortDrayagePlugin/manifest.json
RUN ln -s ../bin ODELoggerPlugin/bin
RUN zip ODELoggerPlugin.zip ODELoggerPlugin/bin/ODELoggerPlugin ODELoggerPlugin/manifest.json
RUN ln -s ../bin CARMAStreetsPlugin/bin
RUN zip CARMAStreetsPlugin.zip CARMAStreetsPlugin/bin/CARMAStreetsPlugin CARMAStreetsPlugin/manifest.json


WORKDIR /home/V2X-Hub/src/tmx/TmxCore/
RUN cp tmxcore.service /lib/systemd/system/
RUN cp tmxcore.service /usr/sbin/
WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/database.sh
RUN /home/V2X-Hub/container/database.sh
WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/service.sh
RUN chmod +x /home/V2X-Hub/container/wait-for-it.sh

WORKDIR /var/www/
RUN mkdir ~/plugins
WORKDIR /home/V2X-Hub/src/v2i-hub/
WORKDIR /var/www/plugins/
RUN mkdir /var/www/plugins/MAP
RUN mkdir /var/www/plugins/.ssl
RUN chown plugin .ssl
RUN chgrp www-data .ssl
WORKDIR /var/www/plugins/.ssl/
RUN openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout tmxcmd.key -out tmxcmd.crt -subj "/CN= <127.0.0.1> " -days 3650
RUN chown plugin *
RUN chgrp www-data *
WORKDIR /home/V2X-Hub/src/v2i-hub/

RUN sudo mkdir /home/V2X-Hub/.base-image 

ENV SONAR_DIR=/opt/sonarqube

# Pull scanner from internet
RUN sudo mkdir $SONAR_DIR && \
        sudo curl -o $SONAR_DIR/sonar-scanner.zip https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-4.4.0.2170-linux.zip && \
        sudo curl -o $SONAR_DIR/build-wrapper.zip https://sonarcloud.io/static/cpp/build-wrapper-linux-x86.zip && \
        # Install Dependancy of NodeJs 6+
        sudo curl -sL https://deb.nodesource.com/setup_10.x | sudo bash - && \
        # Install JQ Json Parser Tool
        sudo mkdir /opt/jq && \
        sudo curl -L "https://github.com/stedolan/jq/releases/download/jq-1.5/jq-linux64" -o /opt/jq/jq && \
        sudo chmod +x /opt/jq/jq

# Unzip scanner
RUN cd $SONAR_DIR && \ 
        sudo unzip $SONAR_DIR/sonar-scanner.zip -d . && \
        sudo unzip $SONAR_DIR/build-wrapper.zip -d . && \
        # Remove zip files 
        sudo rm $SONAR_DIR/sonar-scanner.zip && \
        sudo rm $SONAR_DIR/build-wrapper.zip && \
        # Rename files 
        sudo mv $(ls $SONAR_DIR | grep "sonar-scanner-") $SONAR_DIR/sonar-scanner/ && \
        sudo mv $(ls $SONAR_DIR | grep "build-wrapper-") $SONAR_DIR/build-wrapper/ && \
        # Add scanner, wrapper, and jq to PATH
        sudo echo 'export PATH=$PATH:/opt/jq/:$SONAR_DIR/sonar-scanner/bin/:$SONAR_DIR/build-wrapper/' >> /home/V2X-Hub/.base-image/init-env.sh

# Install gcovr for code coverage tests and add code_coverage script folder to path
RUN sudo apt-get -y install gcovr && \
        sudo echo 'export PATH=$PATH:/home/V2X-Hub/.ci-image/engineering_tools/code_coverage/' >> /home/V2X-Hub/.base-image/init-env.sh


# Set metadata labels
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.name="V2X-Hub-SonarCloud"
LABEL org.label-schema.description="Base image for CARMA CI testing using SonarCloud"
LABEL org.label-schema.vendor="Leidos"
LABEL org.label-schema.version="${VERSION}"
LABEL org.label-schema.url="https://highways.dot.gov/research/research-programs/operations"
LABEL org.label-schema.vcs-url="https://github.com/usdot-fhwa-ops/V2X-HUB"
LABEL org.label-schema.vcs-ref=${VCS_REF}
LABEL org.label-schema.build-date=${BUILD_DATE}

ENTRYPOINT ["/home/V2X-Hub/container/service.sh"]
