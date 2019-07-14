sudo ./fips clean sapp-wayland-linux-make-debug &&
sudo ./fips gen sapp-wayland-linux-make-debug &&
sudo ./fips make viewer-sapp-ui sapp-wayland-linux-make-debug &&
./fips run viewer-sapp-ui sapp-wayland-linux-make-debug