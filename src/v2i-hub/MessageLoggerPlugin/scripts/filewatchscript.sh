#!/bin/bash

ODELINK="<username>@<server-address>:/home/ubuntu/ode/uploads/bsmlog"
echo $ODELINK
rsync -e 'ssh -i ./ODE.pem' -avhP --remove-source-files ./*.bin  $ODELINK
while inotifywait -r -e modify,create,move ./; do
   rsync -e 'ssh -i ./ODE.pem' -avhP --remove-source-files  ./*.bin  $ODELINK
done
