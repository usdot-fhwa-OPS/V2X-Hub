#!/bin/bash
/home/V2X-Hub/container/database.sh
/home/V2X-Hub/container/library.sh
ldconfig
# Install development tools 

apt update 
apt install -y valgrind gdb net-tools vim

# Install Node 22 for SonarLint
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.40.4/install.sh | bash
export NVM_DIR="$HOME/.nvm"
[ -s "$NVM_DIR/nvm.sh" ] && \. "$NVM_DIR/nvm.sh"  # This loads nvm
[ -s "$NVM_DIR/bash_completion" ] && \. "$NVM_DIR/bash_completion"  # This loads nvm bash_completion
nvm install 22.22.0