#include <iostream>
#include <string>
#include <ctime>
#include <map>
#include <filesystem>
#include <chrono>
#include <fstream>
#include <streambuf>
#include <sys/stat.h>
#include "curl/curl.h"
#include "rapidjson/document.h"

namespace fs = std::filesystem;
using namespace rapidjson;
using namespace std::chrono_literals;

std::map<std::string, std::string> SUN_ICONS = {
    {"day", "☀️"},
    {"night", "🌑"},
    {"sunset", "🌇"},
    {"sunrise", "🌅"},
    {"evening", "🌆"}
};

const char FILENAME[] = "/tmp/sunset_sunrise";

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *buf) {
    ((std::string*)buf)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

Document call_api(std::string url) {
    std::string response_buffer;
    CURL *curl_handle;
    CURLcode curl_result;
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response_buffer);
        curl_result = curl_easy_perform(curl_handle);
        curl_easy_cleanup(curl_handle);
    }

    std::ofstream file(FILENAME);
    file << response_buffer;
    file.close();

    Document jsondoc;
    jsondoc.Parse(response_buffer.c_str());
    return jsondoc;
}

time_t process_time(std::string time_str) {
    int hour, min, sec, year, month, day;
    sscanf(time_str.c_str(), "%d-%d-%dT%d:%d:%d+00:00", &year, &month, &day, &hour, &min, &sec);

    struct tm dt = {};
    dt.tm_hour = hour;
    dt.tm_min = min;
    dt.tm_sec = sec;
    dt.tm_year = year - 1900;
    dt.tm_mon = month - 1;
    dt.tm_mday = day;

    return mktime(&dt);
}

std::string get_icon(Document &sundata) {
    std::string sunrise_str = sundata["results"]["sunrise"].GetString();
    std::string sunset_str = sundata["results"]["sunset"].GetString();

    time_t sunrise_time = process_time(sunrise_str);
    time_t sunset_time = process_time(sunset_str);
    
    time_t now;
    time(&now);
    time_t now_utc = mktime(gmtime(&now));

    double sunrise_diff = difftime(now_utc, sunrise_time);
    double sunset_diff = difftime(now_utc, sunset_time);

    if (sunrise_diff <= 300 && sunrise_diff >= -900) {
        return SUN_ICONS["sunrise"];
    }
    else if (sunrise_diff >= 300 && sunset_diff <= -900) {
        return SUN_ICONS["day"];
    }
    else if (sunset_diff <= 300 && sunset_diff >= -900) {
        return SUN_ICONS["sunset"];
    }
    else if (sunset_diff >= 300 && sunset_diff <= 10800) {
        return SUN_ICONS["evening"];
    }
    else return SUN_ICONS["night"];
}

std::string get_time() {
    time_t now;
    struct tm *timeinfo;
    char buffer[13];

    time(&now);
    timeinfo = localtime(&now);

    strftime(buffer, 16, "%d-%b %H:%M", timeinfo);
    return std::string(buffer);
}

Document get_data(std::string url) {
    fs::path path = FILENAME;
    struct tm tm_cf;
    if (fs::exists(path)) { 
        // Getting file write date+time
        //auto f_time = fs::last_write_time(path);
        //time_t cf_time = decltype(f_time)::clock::to_time_t(f_time);
        //struct tm *tm_cf = localtime(&cf_time);
        struct stat filestat;
        if(stat(path.c_str(), &filestat) == 0) {
            const time_t file_mtime = filestat.st_mtime;
            localtime_r(&file_mtime, &tm_cf);

            // Getting current date+time
            time_t now;
            time(&now);
            struct tm tm_now;
            localtime_r(&now, &tm_now);
            
            if (tm_cf.tm_year == tm_now.tm_year
            && tm_cf.tm_mon == tm_now.tm_mon
            && tm_cf.tm_mday == tm_now.tm_mday) {
                std::ifstream file(FILENAME);
                std::stringstream file_stream;
                file_stream << file.rdbuf();
                std::string file_str = file_stream.str();

                if (file_str != "") {
                    Document jsondoc;
                    jsondoc.Parse(file_str.c_str());
                    return jsondoc;
                }
            }
        }
    }
    return call_api(url);
}

int main(void) {

    char *lat = std::getenv("LAT");
    char *lng = std::getenv("LNG");
    char *c1bg = std::getenv("COLOR1BG");
    char *c2bg = std::getenv("COLOR2BG");
    char *c2fg = std::getenv("COLOR2FG");

    if (lat == nullptr || lng == nullptr) {
        return 1;
    }

    if (c1bg == nullptr) c1bg = (char *)"#4F2850";
    if (c2bg == nullptr) c2bg = (char *)"#9E509F";
    if (c2fg == nullptr) c2fg = (char *)"white";

    std::string lat_str(lat, strlen(lat));
    std::string lng_str(lng, strlen(lng));

    std::string c1bg_str(c1bg, strlen(c1bg));
    std::string c2bg_str(c2bg, strlen(c2bg));
    std::string c2fg_str(c2fg, strlen(c2fg));

    std::string url = "https://api.sunrise-sunset.org/json?date=today&formatted=0&lat=" + lat_str + "&lng=" + lng_str;
    Document sundata = get_data(url);

    std::string icon = get_icon(sundata);
    std::string current_time = get_time();
    
    std::cout << "<span background=\""
        + c1bg_str + "\"> "
        + icon + " </span><span font-weight=\"bold\" foreground=\""
        + c2fg_str + "\" background=\""
        + c2bg_str + "\"> " + current_time + " </span>" << std::endl;

    return 0;
}
