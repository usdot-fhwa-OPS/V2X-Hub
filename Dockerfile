FROM ubuntu:bionic-20190807


RUN apt-get update  && apt-get install -y cmake gcc-7 g++-7 libboost1.65-dev libboost-thread1.65-dev libboost-regex1.65-dev libboost-log1.65-dev libboost-program-options1.65-dev libboost1.65-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev gpsd libgps-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv-dev git vim zip build-essential libssl-dev qtbase5-dev qtbase5-dev-tools curl libqhttpengine-dev libgtest-dev libcpprest-dev

ENV MYSQL_ROOT_PASSWORD ivp

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
RUN mkdir -p ~/ext
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

WORKDIR /home/V2X-Hub/ext/server
RUN cmake .
RUN make
RUN make install

WORKDIR /home/V2X-Hub/ext/ccserver
RUN cmake . 
RUN make
RUN make install 

### setup and install v2x-hub core and plugins 

WORKDIR cd /usr/src/googletest/googletest
RUN mkdir ~/build
WORKDIR /usr/src/googletest/googletest/build
RUN cmake ..
RUN make
RUN cp libgtest* /usr/lib/
WORKDIR cd /usr/src/googletest/googletest
RUN rm -rf build
RUN mkdir /usr/local/lib/googletest
RUN ln -s /usr/lib/libgtest.a /usr/local/lib/googletest/libgtest.a
RUN ln -s /usr/lib/libgtest_main.a /usr/local/lib/googletest/libgtest_main.a
RUN ldconfig

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
RUN ln -s ../bin BsmLoggerPlugin/bin
RUN zip BsmLoggerPlugin.zip BsmLoggerPlugin/bin/BsmLoggerPlugin BsmLoggerPlugin/manifest.json
RUN ln -s ../bin PedestrianPlugin/bin
RUN zip PedestrianPlugin.zip PedestrianPlugin/bin/PedestrianPlugin PedestrianPlugin/manifest.json
RUN ln -s ../bin TimPlugin/bin
RUN zip TimPlugin.zip TimPlugin/bin/TimPlugin TimPlugin/manifest.json
RUN ln -s ../bin CARMACloudPlugin/bin
RUN zip CARMACloudPlugin.zip CARMACloudPlugin/bin/CARMACloudPlugin CARMACloudPlugin/manifest.json

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
RUN tmxctl --plugin-install CommandPlugin.zip
WORKDIR /var/www/plugins/
RUN mkdir /var/www/plugins/.ssl
RUN chown plugin .ssl
RUN chgrp www-data .ssl
WORKDIR /var/www/plugins/.ssl/
RUN openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout tmxcmd.key -out tmxcmd.crt -subj "/CN= <your website url> " -days 3650
RUN chown plugin *
RUN chgrp www-data *
WORKDIR /home/V2X-Hub/src/v2i-hub/
RUN tmxctl --plugin-install CswPlugin.zip
RUN tmxctl --plugin-install DmsPlugin.zip
RUN tmxctl --plugin-install DsrcImmediateForwardPlugin.zip
RUN tmxctl --plugin-install LocationPlugin.zip
RUN tmxctl --plugin-install MapPlugin.zip
RUN tmxctl --plugin-install MessageReceiverPlugin.zip
RUN tmxctl --plugin-install ODEPlugin.zip
RUN tmxctl --plugin-install RtcmPlugin.zip
RUN tmxctl --plugin-install SpatPlugin.zip
RUN tmxctl --plugin-install PreemptionPlugin.zip
RUN tmxctl --plugin-install SPaTLoggerPlugin.zip
RUN tmxctl --plugin-install BsmLoggerPlugin.zip
RUN tmxctl --plugin-install PedestrianPlugin.zip
RUN tmxctl --plugin-install TimPlugin.zip
RUN tmxctl --plugin-install CARMACloudPlugin.zip


ENTRYPOINT ["/home/V2X-Hub/container/service.sh"]
