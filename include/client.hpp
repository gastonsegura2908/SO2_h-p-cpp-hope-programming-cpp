/**
 * @file client.hpp
 * @brief  detail of each function of client.cpp
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>

#include "cppSocket.hpp"
#include "httplib.h"

#include <arpa/inet.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <vector>

/**
 * @brief Client buffer size.
 */
#define BUFSIZECLI 2048

/**
 * @brief Maximum client length.
 */
#define MAXLENGTHCLI 100

/**
 * @brief Argument size.
 */
#define ARGSIZE 3

/**
 * @brief Program index.
 */
#define PROGRAM 0

/**
 * @brief Hostname index.
 */
#define HOSTNAME 1

/**
 * @brief Port index.
 */
#define PORT 2

/**
 * @brief Success code.
 */
#define SUCCESS 200

/**
 * @brief Path of the received image.
 */
#define RECIMAGE "../imgtrial/ReceiveImage_"

/**
 * @brief Configuration file path.
 */
#define CONFPATH "../startproject/configuration.txt"

/**
 * @brief Gets the username and password from the user.
 *
 * @param username A character array to store the username.
 * @param password A character array to store the password.
 *
 * @details This function prompts the user to enter their username and password, and stores them in the provided
 * character arrays.
 */
void GetCredentials(char* username, char* password);
/**
 * @brief Handles the TCPv6 client.
 *
 * @param client A unique pointer to a TCPv6Connection object representing the client.
 * @param token A string representing the token of the client.
 */
void HandleTCPv6Client(std::unique_ptr<TCPv6Connection> client, const std::string& token);

/**
 * @brief Reads the port from the third line of a file.
 *
 *
 * @param filename The name of the file from which the port will be read.
 * @return The port number read from the third line of the file.
 */
int ReadPortFromThirdLine(const std::string& filename);

/**
 * @brief Establishes a TCP connection with a server.
 *
 *
 * @param hostname IP address of the server as a character string.
 * @param token token of the client as a character string.
 */
void establishConnection(const char* hostname, const std::string& token);

/**
 * @brief Sends a GET request to the server to retrieve supplies.
 *
 * @param cli httplib::Client object used for HTTP communication.
 */
void GetSupplies(httplib::Client& cli);

/**
 * @brief Sends a GET request to the server to retrieve alerts.
 *
 * @param cli httplib::Client object used for HTTP communication.
 */
void GetAlerts(httplib::Client& cli);

/**
 * @brief Sends a GET request to the server to retrieve an image.
 *
 * @param cli httplib::Client object used for HTTP communication.
 */
void GetImage(httplib::Client& cli);

/**
 * @brief Sends a POST request to the server to modify data or end the session.
 *
 * @param cli httplib::Client object used for HTTP communication.
 * @param params httplib::Params object used for HTTP communication.
 * @param command The command to be sent to the server.
 * @param token The token to be sent to the server.
 */
void PostImageModifyOrEnd(httplib::Client& cli, httplib::Params& params, const std::string& command,
                          const std::string& token);

/**
 * @brief Checks if a string starts with a specific word.
 *
 * @param str The string to check.
 * @param word The word to check for.
 * @return true if the string starts with the word, false otherwise.
 */
bool startsWith(const std::string& str, const std::string& word);

/**
 * @brief Handles the communication between the client and the server.
 *
 *
 * @param cli httplib::Client object used for HTTP communication.
 * @param username Username of the client as a character string.
 * @param password Password of the client as a character string.
 * @param hostname IP address of the server as a character string.
 */
void HandleCommunication(httplib::Client& cli, const char* username, const char* password, const char* hostname);

/**
 * @brief Reads the IP address and port from a file.
 *
 * @param filename The name of the file from which the IP address and port will be read.
 * @return A pair where the first element is the IP address and the second element is the port.
 */
std::pair<std::string, int> ReadIPAndPortFromFile(const std::string& filename);

/**
 * @brief The main function of the client application.
 *
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return 0 if the program finishes successfully.
 */
int main(int argc, char* argv[]);

#endif
