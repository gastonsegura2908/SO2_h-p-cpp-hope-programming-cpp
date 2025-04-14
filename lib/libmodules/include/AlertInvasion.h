/**
 * @file AlertInvasion.hpp
 * @brief AlertInvasion.cpp header file
 */

#ifndef ALERTINVASION_H
#define ALERTINVASION_H

#include <array>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <unistd.h>

/**
 * @def SIZE 256
 * @brief General size used in the system.
 *
 * This definition sets a general size that is used in various parts of the system.
 */
#define SIZE 256

/**
 * @def SIZETS 100
 * @brief Specific size used in the system.
 *
 * This definition sets a specific size that is used in certain parts of the system.
 */
#define SIZETS 100

/**
 * @def MAXTEMP 40.0
 * @brief Maximum allowed temperature.
 *
 * This definition sets the maximum allowed temperature in the system.
 */
#define MAXTEMP 40.0

/**
 * @def MINTEMP 36.0
 * @brief Minimum allowed temperature.
 *
 * This definition sets the minimum allowed temperature in the system.
 */
#define MINTEMP 36.0

/**
 * @def TEMP 38.0
 * @brief Reference temperature.
 *
 * This definition sets a reference temperature in the system.
 */
#define TEMP 38.0

/**
 * @def TRUE 1
 * @brief Definition of true.
 *
 * This definition sets the value of true in the system.
 */
#define TRUE 1

/**
 * @def ENTRY_SIZE 4
 * @brief Size of an entry in the data structure.
 *
 * This definition sets the size of an entry in the data structure.
 */
#define ENTRY_SIZE 4

/**
 * @def PRECISION 1
 * @brief Precision used in calculations.
 *
 * This definition sets the precision used in calculations in the system.
 */
#define PRECISION 1

/**
 * @brief Generates a temperature alert message.
 *
 * This function generates a temperature alert message based on a random temperature
 * value and a random entry point. If the temperature exceeds a certain threshold,
 * an alert message is generated indicating a possible infection.
 *
 * @return A string containing the temperature alert message. If the temperature exceeds the threshold, the message will
 * include an alert of possible infection.
 */
std::string TempAlert();

#endif

