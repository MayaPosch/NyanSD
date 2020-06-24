#!/bin/sh

# Build & installation script for NyanSD.
#
# This will:
# * Build the NyanSD library and install it.
# * Build the NyanSD Daemon (nyansdd) and install it.
# * Install the Systemd or OpenRC service script.
# * Build the Nyansd utility client (nyansd-browse) and install it.

# --- Fetch and install dependencies --
if [ -x "$(command -v apt)" ]; then
	sudo apt update
	sudo apt -y install libpoco-dev
elif [ -x "$(command -v apk)" ]; then
	sudo apk update
	sudo apk add poco-dev openssl-dev
elif [ -x "$(command -v pacman)" ]; then
	sudo pacman -Syy 
	sudo pacman -S --noconfirm --needed poco
fi


# --- NyanSD library ---
echo "Building NyanSD library..."
make lib
echo "Installing NyanSD library..."
make install


# --- NyanSD daemon ---
echo "Building daemon..."
make daemon
echo "Installing daemon..."
make install-daemon


# --- Service script ---
echo "Installing service..."
if [ -d "/run/systemd/system" ]; then
	sudo make -C daemon install-systemd
	sudo ln -s /etc/systemd/system/nyansd.service /etc/systemd/system/multi-user.target.wants/nyansd.service
else
	sudo make -C daemon install-openrc
fi


# --- NyanSD utility client ---
echo "Build utility client..."
make browse
echo "Installing utility client..."
make install-browse

echo "Done."
