# firefox-extension-plasma-desktop-fix

## Features

1. Moves new tabs to a Firefox window on the current desktop & activity, and creates a new window if none exists.
2. Restores windows (e.g. those reopened with Ctrl+Shift+N) to their original positions.

## Demo

Opening a link when then last focused window is on another desktop:


https://github.com/user-attachments/assets/a5ed718c-a686-450c-aade-1fdfd9079281

Extension off:


https://github.com/user-attachments/assets/c61a045e-dbac-44d7-a0d1-c2d9439a1c9d

Restore a window:


https://github.com/user-attachments/assets/b4d6f18a-439e-4ff5-8810-df74476a9efb

## Architecture

This project consists of three components:
1. A KWin plugin that runs a DBus server, exposing necessary interfaces. (src/kwin)
2. A native messaging host that connects our extension to the server. (src/native)
3. A Firefox extension that handles the core functionality. (src/browser)

## Build & Install

Ensure that you have `git`, `npm`, TypeScript, `zip`, `cmake`, Qt6, and KWin installed.

```sh
git clone git@github.com:berrylium0078/firefox-extension-plasma-desktop-fix.git
cd firefox-extension-plasma-desktop-fix
npm install # install dependencies for typescript
mkdir build && cmake -B build && cmake --build build
```

Then, run `sudo cmake --install build` to install the KWin plugin and native messaging host. Log out and log in again and KWin should load the plugin automatically.

The extension can be installed from either `build/extension.xpi` or [Firefox Addons](https://addons.mozilla.org/en-US/firefox/addon/plasma-desktop-fix/).

## Uninstall

Besides the extension itself, `build/install_manifest.txt` holds a list of installed files, just delete all of them.

## How it works

Whenever a new window is created, the extension attaches a unique preface (the implementation uses a random UUID string) to its caption in order to get its KWin internal ID, which is then used to track and control the window's desktop and activity placement.

For window restoration, we use the sessions API to store window-related data across restoration cycles (a Firefox-exclusive feature). This includes window positions and tab existence status (i.e. whether a tab is being restored).
