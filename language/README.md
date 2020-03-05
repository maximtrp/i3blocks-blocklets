# Language blocklet

This is a blocklet, which displays an active keyboard layout. It also supports layout switching.

![Screenshot](screen.png)

## Requirements

* Font Awesome 4.7: [aur](https://aur.archlinux.org/packages/ttf-font-awesome-4/)

* [xkb-switch](https://github.com/ierton/xkb-switch).

## Installation

1. Copy `language` file to your blocklets directory (`/usr/lib/i3blocks`, in my case).

```bash
sudo cp language /usr/lib/i3blocks
```

2. Add language configuration block to your `i3blocks.conf` file:

```
[language]
interval=once
signal=8
markup=pango
```

3. Add a keyboard shortcut for sending a signal to i3blocks. Example string for Alt+Shift combination in `i3`:

```
bindcode Mod1+50 exec --no-startup-id pkill -RTMIN+8 i3blocks
```
