/**
 * @file EmergencyNotification.hpp
 * @brief EmergencyNotification.cpp header file
 */

#ifndef EMERGENCYNOTIFICATION_H
#define EMERGENCYNOTIFICATION_H

#include <array>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

/**
 * @def ENTRYSIZE 3
 * @brief Size of an entry in the data structure.
 *
 * This definition sets the size of an entry in the data structure to 3. It is used to control the size of entries in
 * the data structure.
 */
#define ENTRYSIZE 3

/**
 * @var constexpr size_t SIZEMESSAGE = 256
 * @brief Size of the message.
 *
 * This constant variable defines the size of a message in the system. It is used to control the size of the messages
 * that are sent and received.
 */
constexpr size_t SIZEMESSAGE = 256;

/**
 * @brief Generates a random emergency notification message.
 *
 * This function randomly selects one of the predefined emergency messages:
 * "Server failure", "Power outage", or "Earthquake". It uses the system's time to seed
 * the random number generator, ensuring that the selected message is different each time
 * the function is called. The selected message is then returned as a string.
 *
 * @return A string containing the selected emergency message.
 */
std::string EmergNotif();

#endif

