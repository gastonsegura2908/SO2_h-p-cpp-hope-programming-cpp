/**
 * @file SuppliesData.hpp
 * @brief SuppliesData.cpp header file
 */

#ifndef SUPPLIESDATA_HPP
#define SUPPLIESDATA_HPP

#include <map>
#include <optional>
#include <string>
#include <tuple>

/**
 * @def STATE0 0
 * @brief username
 */
#define STATE0 0

/**
 * @def STATE0 1
 * @brief authorized client
 */
#define STATE1 1

/**
 * @def STATE0 2
 * @brief socket
 */
#define STATE2 2

namespace UserManagement
{

/**
 *  This class provides methods for adding, retrieving, and removing users,
 *  as well as updating and checking user information.
 */
class UserManager
{
    std::map<std::string, std::tuple<std::string, bool, int>> users;

  public:
    /**
     * @brief Adds a new user to the user manager.
     *
     * @param username The name of the user.
     * @param token The user's token.
     * @param isAuthorized Indicates whether the user is authorized.
     * @param socket The user's socket.
     */
    void AddUser(const std::string &username, const std::string &token, bool isAuthorized, const int socket);

    /**
     * @brief Gets the username from a token.
     *
     * @param token The user's token.
     * @return The username if the token exists, otherwise returns a null value.
     */
    std::optional<std::string> GetUserFromToken(const std::string &token);

    /**
     * @brief Gets the token from a username.
     *
     * @param username The name of the user.
     * @return The user's token if the username exists, otherwise returns a null value.
     */
    std::optional<std::string> GetTokenFromUser(const std::string &username);

    /**
     * @brief Updates the socket of an existing user.
     *
     * @param token The user's token.
     * @param newSocket The new socket value.
     */
    void UpdateTokenSocket(const std::string &token, const int newSocket);

    /**
     * @brief Checks if a user is authorized.
     *
     * @param token The user's token.
     * @return std::optional<bool> True if the user is authorized, false otherwise.
     *          Returns null if the user does not exist.
     */
    std::optional<bool> IsUserAuthorized(const std::string &token);

    /**
     * @brief Delete a user.
     *
     * @param token The user's token.
     */
    void RemoveUser(const std::string &token);

    /**
     * @brief Gets a user's socket from a token.
     *
     * @param token The user's token.
     * @return The user's socket if the token exists, otherwise returns a null value.
     */
    std::optional<int> GetSocketFromToken(const std::string &token);
};
} // namespace UserManagement

#endif

