/**
 * @file EmergencyNotification.cpp
 * @brief external emergency module
 */

#include "EmergencyNotification.h"

std::string EmergNotif()
{
    std::array<const char *, ENTRYSIZE> Messages = {"Server failure", "Power outage", "Earthquake"};

    srand(time(NULL));
    int RandomMessage = rand() % Messages.size();

    std::ostringstream EmergencyMessage;
    EmergencyMessage << Messages[RandomMessage];

    return EmergencyMessage.str();
}

