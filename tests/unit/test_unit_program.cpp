/**
 * @file test_unit_program.cpp
 * @brief program on tests
 */

#include "AlertInvasion.h"
#include "EmergencyNotification.h"
#include "SuppliesData.h"
#include "cannyEdgeFilter.hpp"
#include "cppSocket.hpp"
#include "rocksDbWrapper.hpp"
#include <gtest/gtest.h>
#include <httplib.h>
#include <optional>
#include <regex>
#include <set>
#include <signal.h>
#include <thread>

void setRand(int value)
{
    srand(value);
}

TEST(EmergencyNotificationTest, GeneratesValidEmergencyMessage)
{
    setRand(0);

    std::string message = EmergNotif();

    std::set<std::string> validMessages = {"Server failure", "Power outage", "Earthquake"};

    ASSERT_TRUE(validMessages.count(message) > 0) << "Generated message is not valid: " << message;
}

TEST(UserManagerTest, AddUser)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);

    auto username = userManager.GetUserFromToken("token123");
    ASSERT_TRUE(username.has_value());
    ASSERT_EQ(username.value(), "testuser");
}

TEST(UserManagerTest, GetUserFromToken)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);

    auto username = userManager.GetUserFromToken("token123");
    ASSERT_TRUE(username.has_value());
    ASSERT_EQ(username.value(), "testuser");

    auto invalidUsername = userManager.GetUserFromToken("invalidToken");
    ASSERT_FALSE(invalidUsername.has_value());
}

TEST(UserManagerTest, GetTokenFromUser)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);

    auto token = userManager.GetTokenFromUser("testuser");
    ASSERT_TRUE(token.has_value());
    ASSERT_EQ(token.value(), "token123");

    auto invalidToken = userManager.GetTokenFromUser("invaliduser");
    ASSERT_FALSE(invalidToken.has_value());
}

TEST(UserManagerTest, RemoveUser)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);
    userManager.RemoveUser("token123");

    auto username = userManager.GetUserFromToken("token123");
    ASSERT_FALSE(username.has_value());
}

TEST(UserManagerTest, IsUserAuthorized)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);

    auto isAuthorized = userManager.IsUserAuthorized("token123");
    ASSERT_TRUE(isAuthorized.has_value());
    ASSERT_TRUE(isAuthorized.value());

    auto notAuthorized = userManager.IsUserAuthorized("invaliduser");
    ASSERT_FALSE(notAuthorized.has_value());
}

TEST(UserManagerTest, UpdateUserSocket)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);
    userManager.UpdateTokenSocket("token123", 84);

    auto socket = userManager.GetSocketFromToken("token123");
    ASSERT_TRUE(socket.has_value());
    ASSERT_EQ(socket.value(), 84);
}

TEST(UserManagerTest, GetSocketFromToken)
{
    UserManagement::UserManager userManager;
    userManager.AddUser("testuser", "token123", true, 42);

    auto socket = userManager.GetSocketFromToken("token123");
    ASSERT_TRUE(socket.has_value());
    ASSERT_EQ(socket.value(), 42);

    auto invalidSocket = userManager.GetSocketFromToken("invalidToken");
    ASSERT_FALSE(invalidSocket.has_value());
}

TEST(TempAlertTest, GeneratesValidAlertMessage)
{
    setRand(0);

    std::string alertMessage = TempAlert();

    std::set<std::string> validEntries = {"north_entry", "east_entry", "west_entry", "south_entry"};

    bool containsValidEntry = false;
    for (const auto &entry : validEntries)
    {
        if (alertMessage.find(entry) != std::string::npos)
        {
            containsValidEntry = true;
            break;
        }
    }
    ASSERT_TRUE(containsValidEntry) << "Alert message does not contain a valid entry point: " << alertMessage;
}

TEST(TempAlertTest, GeneratesInfectionAlert)
{
    setRand(0);

    std::string alertMessage = TempAlert();

    std::smatch match;
    std::regex tempRegex("([0-9]+\\.[0-9]+)");
    ASSERT_TRUE(std::regex_search(alertMessage, match, tempRegex)) << "Alert message does not contain a temperature: " << alertMessage;

    float temperature = std::stof(match.str(0));
    if (temperature >= TEMP)
    {
        std::string expectedAlertMessage = "Alert of possible infection";
        ASSERT_TRUE(alertMessage.find(expectedAlertMessage) != std::string::npos) << "Expected infection alert not found in message: " << alertMessage;
    }
}

TEST(TempAlertTest, GeneratesNormalAlert)
{
    setRand(0);

    std::string alertMessage = TempAlert();

    std::smatch match;
    std::regex tempRegex("([0-9]+\\.[0-9]+)");
    ASSERT_TRUE(std::regex_search(alertMessage, match, tempRegex)) << "Alert message does not contain a temperature: " << alertMessage;

    float temperature = std::stof(match.str(0));
    if (temperature < TEMP)
    {
        std::string notExpectedAlertMessage = "Alert of possible infection";
        ASSERT_TRUE(alertMessage.find(notExpectedAlertMessage) == std::string::npos) << "Unexpected infection alert found in message: " << alertMessage;
    }
}

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
