# Stopwatch blocklet

This is a simple stopwatch blocklet which I use as a pomodoro timer. It displays the elapsed time in minutes. Blocklet saves unix timestamp to `/tmp/pomodoro` file when you left-click on it, and then updates it every minute (by comparing with current time). Right click removes the file and resets the stopwatch to a waiting state.


## Installation

1. Copy `stopwatch` file to your blocklets directory (`/usr/lib/i3blocks`, in my case).

```bash
sudo chmod +x stopwatch
sudo cp stopwatch /usr/lib/i3blocks
```

2. Add a configuration block to your `i3blocks.conf` file:

```
[stopwatch]
interval=60
```

