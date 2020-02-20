package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"strings"
	"time"
)

var icons = map[string]string{"clear": "", "cloud": "", "rain": "", "storm": "", "snow": "", "mist": "", "fog": "", "smoke": ""}

type openWeather struct {
	Weather []struct {
		ID          int    `json:"id"`
		Main        string `json:"main"`
		Description string `json:"description"`
		Icon        string `json:"icon"`
	} `json:"weather"`
	Base string `json:"base"`
	Main struct {
		Temp     float64 `json:"temp"`
		Pressure int     `json:"pressure"`
		Humidity int     `json:"humidity"`
		TempMin  float64 `json:"temp_min"`
		TempMax  float64 `json:"temp_max"`
	} `json:"main"`
	Visibility int `json:"visibility"`
	Wind       struct {
		Speed float64 `json:"speed"`
		Deg   float64 `json:"deg"`
	} `json:"wind"`
	Clouds struct {
		All int `json:"all"`
	} `json:"clouds"`
	Dt  int `json:"dt"`
	Sys struct {
		Type    int     `json:"type"`
		ID      int     `json:"id"`
		Message float64 `json:"message"`
		Country string  `json:"country"`
		Sunrise int     `json:"sunrise"`
		Sunset  int     `json:"sunset"`
	} `json:"sys"`
}

func (w openWeather) getWeatherIcon() (weatherIcon string) {
	weatherDesc := strings.ToLower(w.Weather[0].Main)
	weatherIcon = ""

	for k, v := range icons {
		if strings.Contains(weatherDesc, k) {
			weatherIcon = v
			break
		}
	}
	return
}

func (w openWeather) String() string {
	weatherIcon := w.getWeatherIcon()
	return fmt.Sprintf("%s %.0f°C", weatherIcon, w.Main.Temp)
}

func getUnixTime() int64 {
	secs := time.Now().Unix()
	return secs
}

type forecast struct {
	List []struct {
		Dt   int64 `json:"dt"`
		Main struct {
			Temp      float64 `json:"temp"`
			TempMin   float64 `json:"temp_min"`
			TempMax   float64 `json:"temp_max"`
			Pressure  float64 `json:"pressure"`
			SeaLevel  float64 `json:"sea_level"`
			GrndLevel float64 `json:"grnd_level"`
			Humidity  int     `json:"humidity"`
			TempKf    float64 `json:"temp_kf"`
		} `json:"main"`
		Weather []struct {
			ID          int    `json:"id"`
			Main        string `json:"main"`
			Description string `json:"description"`
			Icon        string `json:"icon"`
		} `json:"weather"`
		Wind struct {
			Speed float64 `json:"speed"`
			Deg   float64 `json:"deg"`
		} `json:"wind"`
	} `json:"list"`
}

func (f forecast) getForecastIcon(i int) (forecastIcon string) {
	forecastDesc := strings.ToLower(f.List[i].Weather[0].Main)
	forecastIcon = ""

	for k, v := range icons {
		if strings.Contains(forecastDesc, k) {
			forecastIcon = v
			break
		}
	}
	return
}

func (f forecast) String() string {
	var forecastIcon string
	var temp float64
	secs := getUnixTime()

	for i, fcl := range f.List {
		if fcl.Dt-secs > 60*60*6 {
			forecastIcon = f.getForecastIcon(i)
			temp = fcl.Main.Temp
			break
		}
	}
	return fmt.Sprintf("%s %.0f°C", forecastIcon, temp)
}

func getData(url string, c1 chan *http.Response, c2 chan error) {
	resp, err := http.Get(url)
	c1 <- resp
	c2 <- err
}

func main() {
	cityID := os.Getenv("CITY")
	apiKey := os.Getenv("APIKEY")
	units := os.Getenv("UNITS")

	chan1 := make(chan *http.Response)
	chan1_err := make(chan error)
	chan2 := make(chan *http.Response)
	chan2_err := make(chan error)

	ow := openWeather{}
	fc := forecast{}

	weatherURL := fmt.Sprintf("http://api.openweathermap.org/data/2.5/weather?id=%s&units=%s&appid=%s", cityID, units, apiKey)
	forecastURL := fmt.Sprintf("http://api.openweathermap.org/data/2.5/forecast?id=%s&units=%s&appid=%s", cityID, units, apiKey)

	go getData(weatherURL, chan1, chan1_err)
	go getData(forecastURL, chan2, chan2_err)

	weatherResp := <-chan1
	weatherErr := <-chan1_err
	forecastResp := <-chan2
	forecastErr := <-chan2_err

	defer weatherResp.Body.Close()
	defer forecastResp.Body.Close()

	if weatherErr != nil {
		errInt := fmt.Errorf("Error connecting to OpenWeatherMap weather API via GET request")
		fmt.Println(errInt.Error())
		os.Exit(1)
	}

	if forecastErr != nil {
		errInt := fmt.Errorf("Error connecting to OpenWeatherMap forecast API via GET request")
		fmt.Println(errInt.Error())
		os.Exit(1)
	}

	weatherBody, err := ioutil.ReadAll(weatherResp.Body)
	err = json.Unmarshal(weatherBody, &ow)
	if err != nil {
		errInt := fmt.Errorf("Error parsing weather JSON response")
		fmt.Println(errInt.Error(), err)
		os.Exit(1)
	}

	forecastBody, err := ioutil.ReadAll(forecastResp.Body)
	err = json.Unmarshal(forecastBody, &fc)
	if err != nil {
		errInt := fmt.Errorf("Error parsing forecast JSON response")
		fmt.Println(errInt.Error())
		os.Exit(1)
	}

	fmt.Println(ow, "→", fc)
}
