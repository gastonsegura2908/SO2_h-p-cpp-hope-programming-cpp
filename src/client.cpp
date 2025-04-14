/**
 * @file client.cpp
 * @brief program on client in the client-servel model
 */

#include "client.hpp"

void GetCredentials(char* username, char* password)
{
    std::cout << "[AUTHENTICATION]" << std::endl;
    std::cout << "Please enter your username: ";
    std::cin.getline(username, MAXLENGTHCLI);

    std::cout << "Please enter your password: ";
    std::cin.getline(password, MAXLENGTHCLI);
}

void HandleTCPv6Client(std::unique_ptr<TCPv6Connection> client, const std::string& token)
{
    int NumBytes;
    std::array<char, BUFSIZECLI> buffer;
    bool FirstTime = true;
    bool start = true;
    int imageCounter = 0;
    std::ofstream ImageFile;
    std::stringstream ss;
    std::streamsize totalBytesReceived = 0;
    std::streamsize sizeCompressedImage = 0;
    try
    {
        while (true)
        {
            if (FirstTime)
            {
                NumBytes = read(client->getSocket(), &sizeCompressedImage, sizeof(sizeCompressedImage));
                if (NumBytes < 0)
                {
                    perror("Socket read");
                    exit(EXIT_FAILURE);
                }

                ss << token;
                std::string message = ss.str();
                NumBytes = send(client->getSocket(), message.c_str(), message.size(), 0);
                if (NumBytes < 0)
                {
                    perror("Writing to socket");
                    exit(EXIT_FAILURE);
                }
                FirstTime = false;
            }
            else
            {
                if (start)
                {
                    std::ostringstream imageName;
                    imageName << RECIMAGE << token << "_" << imageCounter << ".tar.gz";
                    ImageFile.open(imageName.str(), std::ios::binary);
                    if (!ImageFile.is_open())
                    {
                        std::cerr << "Error opening image file for writing." << std::endl;
                        return;
                    }
                    start = false;
                }
                std::fill(buffer.begin(), buffer.end(), 0);
                NumBytes = read(client->getSocket(), buffer.data(), BUFSIZECLI);

                if (NumBytes < 0)
                {
                    perror("Read from socket");
                    if (close(client->getSocket()) < 0)
                    {
                        perror("Close");
                    }
                    exit(EXIT_FAILURE);
                }

                if (NumBytes == 0)
                {
                    continue;
                }

                ImageFile.write(buffer.data(), NumBytes);
                if (!ImageFile)
                {
                    std::cerr << "Error writing to image file." << std::endl;
                    break;
                }
                totalBytesReceived += NumBytes;
                if (totalBytesReceived >= sizeCompressedImage)
                {
                    ImageFile.close();
                    start = true;
                    imageCounter++;
                    totalBytesReceived = 0;
                }
            }
        }
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error in TCPv6Client: " << e.what() << std::endl;
        ImageFile.close();
    }
}

int ReadPortFromThirdLine(const std::string& filename)
{
    std::ifstream file(filename);
    std::string ip;
    int port;
    if (file.is_open())
    {
        std::getline(file, ip);
        std::getline(file, ip);
        file >> port;
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        exit(1);
    }
    return port;
}

void establishConnection(const char* hostname, const std::string& token)
{
    std::thread clientThread;

    std::string port = std::to_string(ReadPortFromThirdLine(CONFPATH));
    auto TCPv6Client = std::make_unique<TCPv6Connection>(hostname, port, true);
    if (!TCPv6Client->connect())
    {
        std::cerr << "Error connecting client." << std::endl;
        return;
    }
    clientThread = std::thread(HandleTCPv6Client, std::move(TCPv6Client), token);

    clientThread.detach();
}

void GetSupplies(httplib::Client& cli)
{
    auto res = cli.Get("/supplies");
    if (res && res->status == SUCCESS)
    {
        std::cout << "Server response... Supplies:" << res->body << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve supplies. Status code: " << res->status << std::endl;
    }
}

void GetAlerts(httplib::Client& cli)
{
    auto res = cli.Get("/alerts");
    if (res && res->status == SUCCESS)
    {
        std::cout << "Server response... Alerts:" << res->body << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve alerts. Status code: " << res->status << std::endl;
    }
}

void GetImage(httplib::Client& cli)
{
    auto res = cli.Get("/image");
    if (res && res->status == SUCCESS)
    {
        std::cout << "Server response: " << res->body << std::endl;
    }
    else
    {
        std::cout << "Failed to retrieve image. Status code: " << res->status << std::endl;
    }
}

void PostImageModifyOrEnd(httplib::Client& cli, httplib::Params& params, const std::string& command,
                          const std::string& token)
{
    params.clear();
    params.emplace("command", command);
    params.emplace("token", token);
    auto res = cli.Post("/", params);
    if (res && res->status == SUCCESS)
    {
        std::cout << "Server response: " << res->body << std::endl;
    }
}

bool startsWith(const std::string& str, const std::string& word)
{
    if (word.size() > str.size())
    {
        return false;
    }
    return str.substr(0, word.size()) == word;
}

void HandleCommunication(httplib::Client& cli, const char* username, const char* password, const char* hostname)
{
    std::mutex mtx;
    bool AuthorizedClient = false;
    bool finished = false;
    std::cout << "[CONNECTION WITH SERVER]" << std::endl;
    httplib::Params params;
    params.emplace("command", "authenticate");
    params.emplace("username", username);
    params.emplace("password", password);

    auto res = cli.Post("/", params);
    if (res && res->status == SUCCESS)
    {
        std::string serverResponse = res->body;
        std::string token;
        size_t pos = serverResponse.find_last_of(":");
        if (pos != std::string::npos)
        {
            token = serverResponse.substr(pos + 1);
        }
        else
        {
            std::cerr << "Error: Could not find token in server response." << std::endl;
            return;
        }
        establishConnection(hostname, token);
        if (serverResponse.find("Authorized client") != std::string::npos)
        {
            AuthorizedClient = true;
        }
        if (AuthorizedClient)
        {
            std::cout << "--- You are a privileged user ---" << std::endl;
            std::cout << "You can manage: meat, vegetables, fruits, water, antibiotics, analgesics or bandages"
                      << std::endl;
        }
        else
        {
            std::cout << "--- You are a non-privileged user ---" << std::endl;
        }
        std::string command;
        do
        {
            if (AuthorizedClient)
            {
                std::cout << "Enter 'modify' field to change (e.g., 'meat') amount (e.g., '15') or..." << std::endl;
            }
            std::cout << "Enter 'supplies' or 'alerts' or 'image' or 'end': ";
            std::getline(std::cin, command);
            if (command == "supplies")
            {
                GetSupplies(cli);
            }
            else if (command == "alerts")
            {
                GetAlerts(cli);
            }
            else if ((command == "image") || (command == "end") || (startsWith(command, "modify")))
            {
                PostImageModifyOrEnd(cli, params, command, token);
                if ((command == "end"))
                {
                    finished = true;
                }
            }
            else
            {
                std::cout << "You inserted a nonexistent command" << std::endl;
            }

        } while (!finished);
    }
    else
    {
        std::cout << "Error communicating with the server." << std::endl;
        exit(1);
    }
}

std::pair<std::string, int> ReadIPAndPortFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    std::string ip;
    int port;
    if (file.is_open())
    {
        std::getline(file, ip);
        file >> port;
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file: " << filename << std::endl;
        exit(1);
    }
    return {ip, port};
}

int main(int argc, char* argv[])
{
    auto [hostname, port] = ReadIPAndPortFromFile(CONFPATH);

    char username[MAXLENGTHCLI];
    char password[MAXLENGTHCLI];
    GetCredentials(username, password);
    std::cout << "--- WELCOME " << username << " ---" << std::endl;

    httplib::Client cli(hostname, port);
    HandleCommunication(cli, username, password, hostname.c_str());

    std::cout << "finished client." << std::endl;
    return 0;
}
