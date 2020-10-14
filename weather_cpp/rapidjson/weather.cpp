#include <iostream>
#include <string>
#include <cmath>
#include <ctime>
#include <map>
#include <algorithm>
#include "curl/curl.h"
#include "rapidjson/document.h"

using namespace rapidjson;

std::map<std::string, std::string> weather_icons = {
    {"clear", "☀️"},
    {"cloud", "☁️"},
    {"rain", "🌧️"},
    {"storm", "⛈️"},
    {"snow", "❄️"},
    {"mist", "🌫️"},
    {"fog", "🌫️"},
    {"smoke", "💨"},
    {"drizzle", "⛆"}
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *buf) {

    ((std::string*)buf)->append((char*)contents, size * nmemb);
    return size * nmemb;

}

Document get_data(std::string *url) {

    std::string response_buffer;
    CURL *curl_handle;
    CURLcode curl_result;
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url->c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_buffer);
        curl_result = curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
    }

    Document jsondoc;
    if (response_buffer != "") jsondoc.Parse(response_buffer.c_str());
    return jsondoc;

}

std::string get_icon(std::string weather) {
    std::string weather_lower = weather;
    std::for_each(weather_lower.begin(), weather_lower.end(), [](char &c){
	    c = ::tolower(c);
    });

    std::string icon = "🌡️";
    for (auto const& weather_item : weather_icons) {
        if (weather_lower.find(weather_item.first) != std::string::npos) {
            icon = weather_item.second;
            break;
        }
    }
    return icon;
}

std::string get_weather_string(Document &weather) {
    std::string weather_main = weather["weather"][0]["main"].GetString();
    std::string icon = get_icon(weather_main);
    std::string temp = std::to_string(std::lrint(weather["main"]["temp"].GetDouble()));
    return icon + " " + temp + "°";
}

std::string get_forecast_string(Document &forecast) {

    Document forecast_item;
    std::string forecast_str = "";
    long int now = static_cast<long int>(std::time(nullptr));

    for (auto &item : forecast["list"].GetArray()) {
        if (item.GetObject()["dt"].GetInt64() - now > 60*60*6) {
            forecast_item.CopyFrom(item, forecast_item.GetAllocator());
            forecast_str = get_weather_string(forecast_item);
            break;
        }
    }
    return forecast_str;
}

int main(void) {

    char *city = std::getenv("CITY");
    char *apikey = std::getenv("APIKEY");
    char *units = std::getenv("UNITS");

    if (city == nullptr || apikey == nullptr || units == nullptr) {
        return 1;
    }

    std::string city_str(city);
    std::string apikey_str(apikey);
    std::string units_str(units);

    std::string weather_url = "http://api.openweathermap.org/data/2.5/weather";
    weather_url += "?id=" + city_str + "&appid=" + apikey_str + "&units=" + units_str;
    
    std::string forecast_url = "http://api.openweathermap.org/data/2.5/forecast";
    forecast_url += "?id=" + city_str + "&appid=" + apikey_str + "&units=" + units_str;

    Document weather = get_data(&weather_url);
    Document forecast = get_data(&forecast_url);
    
    if (weather.IsNull() || forecast.IsNull()) {
        return 1;
    }

    std::string weather_str = get_weather_string(weather);
    std::string forecast_str = get_forecast_string(forecast);

    std::cout << weather_str;
    std::cout << " → ";
    std::cout << forecast_str << std::endl;
    return 0;
}
