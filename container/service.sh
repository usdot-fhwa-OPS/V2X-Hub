#!/bin/bash
wait-for-it.sh 127.0.0.1:3306

cd /var/log/tmx

for plugin in /usr/local/plugins/*.zip; do
    echo "Installing plugin $plugin"
    tmxctl --plugin-install $plugin
done

# command plugin must always be enabled
tmxctl --plugin CommandPlugin --enable
tmxcore
