FROM ubuntu:xenial-20190122

RUN apt-get update && apt-get install -y cmake gcc-5 g++-5 libboost1.58-dev libboost-thread1.58-dev libboost-regex1.58-dev libboost-log1.58-dev libboost-program-options1.58-dev libboost1.58-all-dev libxerces-c-dev libcurl4-openssl-dev libsnmp-dev libmysqlclient-dev libjsoncpp-dev uuid-dev libusb-dev libusb-1.0-0-dev libftdi-dev swig liboctave-dev gpsd libgps-dev portaudio19-dev libsndfile1-dev libglib2.0-dev libglibmm-2.4-dev libpcre3-dev libsigc++-2.0-dev libxml++2.6-dev libxml2-dev liblzma-dev dpkg-dev libmysqlcppconn-dev libev-dev libuv-dev git vim zip

ENV MYSQL_ROOT_PASSWORD ivpx1

RUN mkdir ~/V2X-Hub
COPY . /home/V2X-Hub
WORKDIR /home/V2X-Hub/src/tmx/
RUN cmake . -DCMAKE_BUILD_TYPE=DEBUG
RUN make
RUN make install

WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/library.sh
RUN /home/V2X-Hub/container/library.sh
RUN ldconfig

WORKDIR /home/V2X-Hub/ext/libwebsockets/
RUN cmake -DLWS_WITH_SHARED=OFF .
RUN make
RUN make install

WORKDIR /home/V2X-Hub/src/v2i-hub/
RUN cmake .
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

#ENV MYSQL_ROOT_PASSWORD ivpx
RUN echo "mysql-server mysql-server/root_password password $MYSQL_PWD" | debconf-set-selections
RUN echo "mysql-server mysql-server/root_password_again password $MYSQL_PWD" | debconf-set-selections
#RUN rm -rf /var/lib/apt/lists/* && apt-get update && apt-get install -y apache2 mysql-server php

WORKDIR /home/V2X-Hub/src/tmx/TmxCore/
RUN cp tmxcore.service /lib/systemd/system/
RUN cp tmxcore.service /usr/sbin/
WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/database.sh
RUN /home/V2X-Hub/container/database.sh
WORKDIR /home/V2X-Hub/container/
RUN chmod +x /home/V2X-Hub/container/service.sh

WORKDIR /var/www/
RUN mkdir ~/plugins
WORKDIR /home/V2X-Hub/src/v2i-hub/
RUN tmxctl --plugin-install CommandPlugin.zip
WORKDIR /var/www/plugins/
RUN mkdir /var/www/plugins/.ssl
RUN chown root .ssl
RUN chgrp www-data .ssl
WORKDIR /var/www/plugins/.ssl/
RUN openssl req -x509 -newkey rsa:4096 -sha256 -nodes -keyout tmxcmd.key -out tmxcmd.crt -subj "/CN= <your website url> " -days 3650
RUN chown root *
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

ENTRYPOINT ["/home/V2X-Hub/container/service.sh"]