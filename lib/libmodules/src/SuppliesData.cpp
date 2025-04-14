/**
 * @file SuppliesData.cpp
 * @brief supply management module
 */

#include "SuppliesData.h"

namespace UserManagement
{
void UserManager::AddUser(const std::string &username, const std::string &token, bool isAuthorized, const int socket)
{
    users[token] = std::make_tuple(username, isAuthorized, socket);
}

std::optional<std::string> UserManager::GetUserFromToken(const std::string &token)
{
    if (users.find(token) != users.end())
    {
        return std::get<STATE0>(users[token]);
    }
    return std::nullopt;
}

std::optional<std::string> UserManager::GetTokenFromUser(const std::string &username)
{
    for (const auto &pair : users)
    {
        if (std::get<STATE0>(pair.second) == username)
        {
            return pair.first;
        }
    }
    return std::nullopt;
}

void UserManager::UpdateTokenSocket(const std::string &token, const int newSocket)
{
    if (users.find(token) != users.end())
    {
        std::get<STATE2>(users[token]) = newSocket;
    }
}

std::optional<bool> UserManager::IsUserAuthorized(const std::string &token)
{
    if (users.find(token) != users.end())
    {
        return std::get<STATE1>(users[token]);
    }
    return std::nullopt;
}

void UserManager::RemoveUser(const std::string &token)
{
    if (users.find(token) != users.end())
    {
        users.erase(token);
    }
}

std::optional<int> UserManager::GetSocketFromToken(const std::string &token)
{
    if (users.find(token) != users.end())
    {
        return std::get<STATE2>(users[token]);
    }
    return std::nullopt;
}

} // namespace UserManagement

