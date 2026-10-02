# Grab git dependencies
git submodule init
git submodule update --init --recursive

set -e

sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y git-lfs

DEB_FILE=$(find . -name "libalp43_0.0.2-beta_amd64.deb" -print -quit)
if [ -z "$DEB_FILE" ]; then
    echo "Error: libalp43 .deb installer package not found in this repository!"
    echo "Please place the file 'libalp43_0.0.2-beta_amd64.deb' somewhere in the project tree."
    echo "If you've done this step at some point and libalph43 is installed, ignore this."
else
    # These will only run if DEB_FILE is NOT empty
    echo "alph43: $DEB_FILE"
    sudo dpkg -i "$DEB_FILE"

    CURRENT_USER=$(id -un)
    sudo usermod -aG vialux_alp "$CURRENT_USER"    
fi

# Create build directory if it doesn't exist
mkdir -p build

echo "Bootstrap complete. You can now run build-all.sh, build-debug.sh or build-release.sh."