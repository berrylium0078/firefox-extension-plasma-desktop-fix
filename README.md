# firefox-extension-plasma-desktop-fix

## Features

1. Move new tabs to a Firefox window on the current desktop & activity, and if there is none, create one.
2. Move restored windows (i.e. Ctrl+Shift+N) to where they used to be.

## Demo

uploading...

## Architecture

This project consists of three components:
1. A KWin plugin that runs a DBus server, exposing necessary interfaces.
2. A Firefox extension doing all sorts of stuff.
3. A native messaging host that connects our plugin to the server.

## Build & Install

```sh
git clone git@github.com:berrylium0078/firefox-extension-plasma-desktop-fix.git
cd firefox-extension-plasma-desktop-fix
mkdir build && cmake -B build
cmake --build build
cmake --install build # may require root privileges
```

You can install the extension from `/build/extension.xpi` or [Firefox Addons](https://addons.mozilla.org/en-US/firefox/addon/plasma-desktop-fix/).

## Uninstall

`build/install_manifest.txt` holds a list of installed files, just delete all of them.
