#ifndef JOBREPORT_ERROR_HPP
#define JOBREPORT_ERROR_HPP

#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <sstream>

// Debugging macro
#ifdef JOBREPORT_DEBUG
    #define LOG(x) std::cerr << "DEBUG LOG: " << x << std::endl;
#else
    #define LOG(x)
#endif


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
        days = std::stoi(time_str.substr(0, dash_pos));
        remaining_time = time_str.substr(dash_pos + 1);
    }

    // Create a stringstream from the remaining time string
    std::stringstream ss(remaining_time);
    std::string component;

    // Split the input string based on the ':' delimiter
    while (std::getline(ss, component, ':'))
    {
        time_components.push_back(std::stoi(component));
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
