#!/bin/sh
#updating to the latest repository package List...
echo "updating ubuntu package list from repository..."
cd /usr/src
apt-get update
#installing prerequisite"
echo "installing freeswitch prerequisite..."
apt-get -y install build-essential subversion automake autoconf wget libtool libncurses5-dev xvfb libx11-dev libasound2-dev xfs xfonts-100dpi xfonts-75dpi xfonts-scalable
echo "checking out and compiling  freeswitch from SVN..."
cd /usr/src
wget http://www.freeswitch.org/eg/Makefile
make
echo "installing freeswitch..."
make install
echo "downloading Sounds files..."
cd freeswitch.trunk
make cd-sounds-install
echo "downloading Freeswitch Music on hold..."
make cd-moh-install
echo "Install Skype?y/n"
read skype
if [[ "$skype" == "o" ]]
echo "Configuring freeswitch..."
echo "adding skypiax configuration..."
cp src/mod/endpoints/mod_skypiax/configs/skypiax.conf.xml /usr/local/freeswitch/conf/autoload_configs/
echo "installing skype..."
cd /
apt-get -y install ia32-libs lib32asound2 libc6-i386 lib32gcc1 \
lib32stdc++6 lib32ncurses5 lib32z1 libasound2-plugins
wget -c wget http://www.skype.com/go/getskype-linux-ubuntu
#Configuring skype..."
cp /mnt/root/configskypenew.tgz ./
tar xzf configskypenew.tgz
chown root.root .Skype
fi
echo "finished!"
