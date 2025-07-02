REMOTE_USER="ubuntu"; 
REMOTE_ADDR="www.carma-cloud.com"; 
KEY_FILE="carma-cloud-test-1.pem";
HOST_PORT="33333"; # This port is forwarded to remote host (carma-cloud) and port: 
REMOTE_PORT="22222"; # This port is forwarded to local host (v2xhub) and port: 22222 sudo ./open_tunnels.sh -u $REMOTE_USER -a $REMOTE_ADDR -r $HOST_PORT -k $KEY_FILE -p 
sudo ./open_tunnels.sh -u $REMOTE_USER -a $REMOTE_ADDR -r $HOST_PORT -k $KEY_FILE -p $REMOTE_PORT
