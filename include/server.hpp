/**
 * @file server.hpp
 * @brief detail of each function of server.cpp
 */
#ifndef SERVER_HPP
#define SERVER_HPP

#include "AlertInvasion.h"
#include "EmergencyNotification.h"
#include "SuppliesData.h"
#include "cannyEdgeFilter.hpp"
#include "cppSocket.hpp"
#include "httplib.h"
#include "rocksDbWrapper.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <future>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <queue>
#include <random>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <utility>

/**
 * @brief Database path.
 */
#define DBPATH "./Database"

/**
 * @brief Server buffer size.
 */
#define BUFSIZESERVER 2048

/**
 * @brief Chunk size.
 */
#define CHUNKSIZE 1024

/**
 * @brief Server port index.
 */
#define PORTSERVER 1

/**
 * @brief Log buffer size.
 */
#define LOGBUF 100

/**
 * @brief Server program index.
 */
#define PROGRAMSERVER 0

/**
 * @brief Name of the Linux distribution.
 */
#define UBUNTU "UBUNTU"
/**
 * @brief Part 0.
 */

#define PART0 0

/**
 * @brief Part 1.
 */
#define PART1 1

/**
 * @brief Part 2.
 */
#define PART2 2

/**
 * @brief Part 3.
 */
#define PART3 3

/**
 * @brief Test image path.
 */
#define IMAGEPATH "../imgtrial/testImage.png"

/**
 * @brief Destination image path.
 */
#define DESTIMAGE "../imgtrial/canny.png"

/**
 * @brief Compressed image path.
 */
#define COMPPATH "../imgtrial/canny.tar.gz"

/**
 * @brief Log file path.
 */
#define LOGPATH "../var/log/refuge.log"

/**
 * @brief Configuration file path.
 */
#define CONFPATH "../startproject/configuration.txt"

/**
 * @brief Logs an activity to a file.
 *
 * @param activity A string representing the activity to log.
 * @param LogMutex A mutex for synchronizing log writes.
 * @param token An optional string representing the token of the user who performed the activity. Default is an
 * empty string.
 *
 * @details This function gets the current time, formats it into a string, locks the log mutex, opens the log file
 * in append mode, writes the time, activity, and token (if provided) to the log file, and unlocks the mutex. If an
 * error occurs during opening the log file, it throws a runtime error.
 */
void LogActivity(const std::string& activity, std::mutex& LogMutex, const std::string& token = "");

/**
 * @class Server
 *
 * @brief A server class that handles user management, file reading, and database operations.
 *
 * @details This class uses a UserManager object for user management, a RocksDbWrapper object for database operations,
 * and several other member variables for file reading and synchronization. It provides methods for handling
 * authentication, commands, and post requests, as well as modifying supplies.
 */
class Server
{
    friend void ModifyAlertsAndEmergencies(Server& server, const std::string& field, const nlohmann::json& value);

  private:
    UserManagement::UserManager userManager;   /**< A UserManager object for user management. */
    bool fileReadComplete;                     /**< A boolean indicating whether the file read is complete. */
    RocksDbWrapper db;                         /**< A RocksDbWrapper object for database operations. */
    std::mutex DbMutex;                        /**< A mutex for synchronizing database operations. */
    std::mutex ImgFlagMutex;                   /**< A mutex for synchronizing access to the image flags. */
    std::condition_variable fileReadCondition; /**< A condition variable for waiting for the file read to complete. */
  public:
    /**
     * @brief Vector containing pieces of the image to send.
     */
    std::vector<std::vector<char>> chunks;
    /**
     * @brief Queue of image requests.
     */
    std::queue<std::pair<int, std::string>> imageRequestQueue;
    /**
     * @var std::mutex queueMutex
     * @brief Mutex for synchronizing access to the image request queue.
     *
     */
    std::mutex queueMutex;
    /**
     * @var std::condition_variable queueCondition
     * @brief Condition variable for waiting for requests in the queue.
     *
     */
    std::condition_variable queueCondition;

    /**
     * @brief Constructs a new Server object and initializes the database.
     *
     * @details The constructor initializes the fileReadComplete flag to false, the database to the specified path, and
     * the data in the database to the initial data.
     */
    Server();
    /**
     * @brief Sets the fileReadComplete flag.
     *
     * @param value The new value for the fileReadComplete flag.
     */
    void setFileReadComplete(bool value);
    /**
     * @brief Gets the value of the fileReadComplete flag.
     *
     * @return The value of the fileReadComplete flag.
     */
    bool getFileReadComplete();
    /**
     * @brief Generates a token for a user.
     *
     * @return The generated token.
     *
     * @details This method generates a random 32-character token consisting of digits and uppercase letters.
     */
    std::string GenerateToken();

    /**
     * @brief Processes image requests from clients.
     *
     * @note This function should be run in a separate thread, as it contains an infinite loop that can block the main
     * thread.
     */
    void ProcessImageRequests();

    /**
     * @brief Checks if a command is a valid modify command.
     *
     * @param command The command to check.
     *
     * @return True if the command is a valid modify command, false otherwise.
     *
     * @details This method checks if the command is a modify command with a valid field and a non-negative amount.
     */
    bool HandleModifyCommand(const std::string& command);
    /**
     * @brief Handles an authentication request.
     *
     * @param req The request to handle.
     * @param res The response to send.
     * @param LogMutex A mutex for logging.
     *
     * @details This method gets the username and password from the request, generates a token, adds the user to the
     * user manager, and sends a response indicating whether the user is authorized or not. It also logs the event.
     */
    void HandleAuthentication(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex);
    /**
     * @brief Handles a command request.
     *
     * @param req The request to handle.
     * @param res The response to send.
     * @param LogMutex A mutex for logging.
     *
     * @details This method gets the token and command from the request, checks if the user is authorized to modify, and
     * sends a response based on the command. It also logs the event.
     */
    void HandleCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex);

    /**
     * @brief Handles the supplies command from the client.
     *
     * @param req The HTTP request.
     * @param res The HTTP response.
     * @param LogMutex The mutex for logging.
     */
    void HandleSuppliesCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex);

    /**
     * @brief Handles the alerts command from the client.
     *
     * @param req The HTTP request.
     * @param res The HTTP response.
     * @param LogMutex The mutex for logging.
     */
    void HandleAlertsCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex);

    /**
     * @brief Handles a POST request.
     *
     * @param req The request to handle.
     * @param res The response to send.
     * @param LogMutex A mutex for logging.
     *
     * @details This method gets the command from the request. If the command is "authenticate", it handles the
     * authentication request. Otherwise, it handles the command request.
     */
    void HandlePostRequest(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex);

    /**
     * @brief Modifies the supplies in the database.
     *
     * @param field The field to modify.
     * @param amount The new amount for the field.
     *
     * @return True if the modification was successful, false otherwise.
     *
     * @details This method locks the database mutex, gets the data from the database, checks if the field is in the
     * food or medicine objects, sets the field to the provided amount, and puts the data back into the database.
     */
    bool Modifysupplies(const std::string& field, int amount);

    /**
     * @brief Sets the socket for a user.
     *
     * @param token The token of the user.
     * @param socket The new value for the socket.
     *
     * @details This method updates the user's socket in the user manager.
     */
    void SetTokenSocket(const std::string& token, int socket);

    /**
     * @brief Gets the socket for a user.
     *
     * @param token The token of the user.
     *
     * @return The socket for the user, or nullopt if the user does not have a socket.
     *
     * @details This method gets the user's socket from the user manager using the user's token.
     */
    std::optional<int> GetUserSocket(const std::string& token);

    /**
     * @brief Notifies that the file read is complete.
     *
     * @details This method locks the image flag mutex, sets the fileReadComplete flag to true, and notifies all waiting
     * threads.
     */
    void notifyFileReadComplete();

    /**
     * @brief Waits for the file read to complete.
     *
     * @details This method locks the image flag mutex and waits for the fileReadComplete flag to be true.
     */
    void waitForFileReadComplete();
};

/**
 * @brief Runs a temperature alert loop.
 *
 * @param server A reference to a Server object.
 * @param LogMutex A reference to a mutex for logging.
 *
 * @details This function enters a loop where it sleeps for a random amount of time, generates a temperature alert, and
 * logs the alert. If the alert indicates a possible infection, it modifies the alerts in the server's database.
 */
void RunTempAlert(Server& server, std::mutex& LogMutex);

/**
 * @brief Runs an emergency notification loop.
 *
 * @param server A reference to a Server object.
 * @param LogMutex A reference to a mutex for logging.
 *
 * @details This function enters a loop where it sleeps for a random amount of time, generates an emergency
 * notification, and logs the notification. It then modifies the emergencies in the server's database.
 */
void RunEmergNotif(Server& server, std::mutex& LogMutex);

/**
 * @brief Handles a signal.
 *
 * @param SigNum The signal number.
 *
 * @details This function prints a message indicating that the program is ending and exits the program.
 */
void SignalHandlerFunction(int SigNum);

/**
 * @brief Compresses an image and saves it to a file.
 *
 * @param inputPath A string representing the path of the input image.
 * @param outputPath A string representing the path to save the compressed image.
 *
 * @details This function reads the input image into a buffer, compresses the buffer, and writes the compressed data to
 * the output file. If an error occurs during any of these operations, it throws a runtime error.
 */
void compressImage(const std::string& inputPath, const std::string& outputPath);

/**
 * @brief Reads a file into chunks.
 *
 * @param filePath A string representing the path of the file to read.
 * @param chunkSize The size of the chunks to read the file into.
 *
 * @return A vector of vectors of characters representing the chunks of the file.
 *
 * @details This function opens the file, reads it into chunks of the specified size, and returns the chunks. If an
 * error occurs during opening the file, it throws a runtime error.
 */
std::vector<std::vector<char>> readInChunks(const std::string& filePath, std::size_t chunkSize);

/**
 * @brief Sends chunks of data to a TCPv6 client.
 *
 * @param server A reference to a Server object.
 * @param socket An integer representing the socket to send data through.
 * @param chunks A vector of vectors of characters representing the chunks of data to be sent.

 *
 * @details This function waits for the file read to complete and checks if the chunks are empty. If not, it enters a
 * loop where it continuously sends chunks of data to the client. The first time it sends data, it sends the username to
 * the client. After that, it sends the chunks of data. If the server's image flag for the username is set, it resets
 * the flag and starts sending data again. If an error occurs during sending or receiving data, it prints an error
 * message.
 */
void SendChunksTCPv6(Server& server, int socket, const std::vector<std::vector<char>>& chunks);
/**
 * @brief Handles the communication with a client over a socket.
 *
 * @param server The server that is communicating with the client.
 * @param clientSocket The socket of the client.
 *
 * @throws std::runtime_error If there is an error writing to or reading from the socket.
 */
void SocketUsername(Server& server, int clientSocket);

/**
 * @brief Establishes a connection to a TCPv6 client.
 *
 *
 * @param server server
 * @exception std::exception Handles any exceptions thrown within the function and returns -1.
 */
void ConnectToTCPv6Client(Server& server);

/**
 * @class Timer
 *
 * @brief A simple timer class that measures the duration between its creation and when the Stop() method is called.
 *
 * @details This class uses the high resolution clock from the standard library to measure time. The timer starts when a
 * Timer object is created and stops when the Stop() method is called. If the Stop() method is not called before the
 * Timer object is destroyed, it will be called in the destructor.
 */
class Timer
{
  public:
    /**
     * @brief Constructs a new Timer object and starts the timer.
     *
     * @details The constructor initializes the start time point to the current time and sets the stopped flag to false.
     */
    Timer();

    /**
     * @brief Destroys the Timer object and stops the timer if it has not been stopped yet.
     *
     * @details The destructor checks if the timer has been stopped. If not, it calls the Stop() method.
     */
    ~Timer();
    /**
     * @brief Stops the timer and prints the duration.
     *
     * @details This method gets the current time point, calculates the duration from the start time point to the
     * current time point, converts the duration to microseconds and milliseconds, prints the duration, and sets the
     * stopped flag to true.
     */
    void Stop();

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock>
        m_StartTimepoint; /**< The time point when the timer was started. */
    bool mStopped;        /**< A flag indicating whether the timer has been stopped. */
};

/**
 * @brief Applies Canny edge detection to an image, compresses the image, and reads the compressed image into chunks.
 *
 * @param server A reference to a Server object.
 * @param chunkSize The size of the chunks to read the compressed image into.
 *
 * @details This function applies Canny edge detection to an image, compresses the image, and reads the compressed image
 * into chunks. If an error occurs during any of these operations, it prints an error message.
 */
void CannyCompressAndRead(Server& server, std::size_t chunkSize);

/**
 * @brief Reads the port from the third line of the given file.
 *
 */
int ReadPortFromThirdLine(const std::string& filename);

/**
 * @brief Modifies the alerts and emergencies in the server's database.
 *
 * @param server A reference to a Server object.
 * @param field A string representing the field to be modified.
 * @param value A json object representing the new value for the field.
 *
 * @details This function locks the server's database mutex to ensure thread safety. It then retrieves the "data" entry
 * from the database and parses it into a json object. If the "alerts" object contains the provided field, it increments
 * its value by 1. If the "emergencies" object contains the field, it sets its value to the provided value. The modified
 * data is then written back to the database.
 */
void ModifyAlertsAndEmergencies(Server& server, const std::string& field, const nlohmann::json& value);

/**
 * @brief Read the port from a file.
 *
 */
int ReadPortFromFile(const std::string& filename);

/**
 * @brief The main function of the server application.
 *
 */
int main(int argc, char* argv[]);

#endif
