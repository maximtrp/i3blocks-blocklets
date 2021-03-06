# Weather blocklet

[![Go Report Card](https://goreportcard.com/badge/github.com/maximtrp/i3blocks-blocklets)](https://goreportcard.com/report/github.com/maximtrp/i3blocks-blocklets)

This blocklet retrieves and shows current weather and forecast (6 hours) from OpenWeatherMap.

![Screenshot](screen.png)

## Requirements

* Golang compiler
* API key for [OpenWeatherMap](https://openweathermap.org/api)

## Installation

1. Compile the blocklet using `make`:

```bash
make
```

2. Copy the compiled file to your blockets directory (`/usr/lib/i3blocks`, in my case):

```bash
sudo cp weather /usr/lib/i3blocks
```

3. Add `weather` configuration block to your `i3blocks.conf` file. Fill in `APIKEY` and `CITY` parameters:

```
[weather]
interval=1200
CITY=
APIKEY=
UNITS=metric
```
