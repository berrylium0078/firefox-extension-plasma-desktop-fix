# firefox-extension-plasma-desktop-fix

## Features

1. Move new tabs to a Firefox window on the current desktop & activity, and if there is none, create one.
2. Move restored windows (i.e. Ctrl+Shift+N) to where they used to be.

## Demo

To be uploaded...

## Architecture

This project consists of three components:
1. A KWin plugin that runs a DBus server, exposing necessary interfaces. (src/kwin)
2. A native messaging host that connects our extension to the server. (src/native)
3. A Firefox extension doing all sorts of stuff. (src/browser)

## Build & Install

Ensure that you have `git`, `npm`, `zip`, Qt6, and KWin installed.

```sh
git clone git@github.com:berrylium0078/firefox-extension-plasma-desktop-fix.git
cd firefox-extension-plasma-desktop-fix
npm install # install dependency for typescript
mkdir build && cmake -B build && cmake --build build
```

Then, run `sudo cmake --install build` to install the KWin plugin and native messaging host. Logout and login again and KWin should load the plugin automatically.

The extension can be installed from `build/extension.xpi` or [Firefox Addons](https://addons.mozilla.org/en-US/firefox/addon/plasma-desktop-fix/).

## Uninstall

`build/install_manifest.txt` holds a list of installed files, just delete all of them.

## How it works

Whenever a new window is created, the extension attachs a unique preface (a random UUID string) to the window's caption, and asks the KWin plugin for its internal ID, which is later used for monitoring and manipulating which desktops and activities the window is on.

As for session restoration, we use the sessions API to store infomation bounded with windows and tabs (which, unforturenately, is only supported by Firefox), such as the position of windows and whether a tab already exists before creation (i.e. because it's restored).
