# CPU usage

This blocklet is based on `mpstat` output.

## Requirements

* Font Awesome 4.7: [aur](https://aur.archlinux.org/packages/ttf-font-awesome-4/)

## Installation

1. Copy `cpu_usage` file to your blocklets directory (`/usr/lib/i3blocks`, in my case).

```bash
sudo cp cpu_usage /usr/lib/i3blocks
```

2. Add a configuration block to your `i3blocks.conf` file:

```
[cpu_usage]
interval=5
markup=pango
```
