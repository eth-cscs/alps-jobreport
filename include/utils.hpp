#ifndef JOBREPORT_ERROR_HPP
#define JOBREPORT_ERROR_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <limits.h>
#include <filesystem>

// Debugging macro
#ifdef JOBREPORT_DEBUG
    #define LOG(x) std::cerr << "DEBUG LOG: " << x << std::endl;
#else
    #define LOG(x)
#endif

std::string get_hostname() {
    char hostname[HOST_NAME_MAX + 1]; // HOST_NAME_MAX doesn't include the null terminator
    // gethostname returns 0 on success, and -1 on failure
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return std::string(hostname);
    } else {
        // Handle the error appropriately, here we just return an empty string
        std::cout << "WARNING: Unable to read hostname." << std::endl;
        return std::string("unknown_host");
    }
}

std::filesystem::path get_home_directory() {
    const char* home_dir = std::getenv("HOME");
    if (home_dir) {
        return std::filesystem::path(home_dir);
    } else {
        std::cerr << "Error: HOME environment variable is not set." << std::endl;
        return std::filesystem::path();
    }
}

void raise_error(const std::string &msg)
{
    std::cerr << msg << std::endl;
    std::exit(EXIT_FAILURE);
}

// Function used to parse the time string format DD-HH:MM:SS to seconds
int parse_time(const std::string &time_str)
{
    int days = 0, hours = 0, minutes = 0, seconds = 0;
    std::vector<int> time_components;

    // Check for the presence of the '-' specifier at the beginning
    std::string remaining_time = time_str;
    if (time_str.find('-') != std::string::npos)
    {
        size_t dash_pos = time_str.find('-');
        try{
            days = std::stoi(time_str.substr(0, dash_pos));
        } catch (std::exception &e) {
            raise_error("Invalid value for -t, --max_time\n"
                        "Expected a time string in the format DD-HH:MM:SS, got: \"" + time_str + "\""
            );
        }
        
        remaining_time = time_str.substr(dash_pos + 1);
    }

    // Create a stringstream from the remaining time string
    std::stringstream ss(remaining_time);
    std::string component;

    // Split the input string based on the ':' delimiter
    while (std::getline(ss, component, ':'))
    {
        try{
            time_components.push_back(std::stoi(component));
        } catch (std::exception &e) {
            raise_error("Invalid value for -t, --max_time\n"
                        "Expected a time string in the format DD-HH:MM:SS, got: \"" + time_str + "\"" 
            );
        }
    }

    // Determine the format based on the number of components
    int n = time_components.size();
    if (n == 1)
    {
        // Only minutes provided
        minutes = time_components[0];
    }
    else if (n == 2)
    {
        // Format is minutes:seconds
        minutes = time_components[0];
        seconds = time_components[1];
    }
    else if (n == 3)
    {
        // Format is hours:minutes:seconds
        hours = time_components[0];
        minutes = time_components[1];
        seconds = time_components[2];
    }

    // Calculate the total time in seconds
    return (days * 86400) + (hours * 3600) + (minutes * 60) + seconds;
}

#endif // JOBREPORT_ERROR_HPP
