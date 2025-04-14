/**
 * @file server.cpp
 * @brief program on server in the client-servel model
 */

#include "server.hpp"

void LogActivity(const std::string& activity, std::mutex& LogMutex, const std::string& token)
{
    auto CurrentTime = std::chrono::system_clock::now();
    std::time_t TimeTCurrentTime = std::chrono::system_clock::to_time_t(CurrentTime);
    std::tm* LocalTime = std::localtime(&TimeTCurrentTime);

    std::array<char, LOGBUF> buf;
    std::strftime(buf.data(), buf.size(), "%a %b %d %T %Y", LocalTime);

    std::lock_guard<std::mutex> lock(LogMutex);
    std::ofstream LogFile("../var/log/refuge.log", std::ios_base::app);
    LogFile << buf.data() << ", " << activity;
    if (!token.empty())
    {
        LogFile << ", " << token;
    }
    LogFile << "\n";
}

Server::Server() : fileReadComplete(false), db(DBPATH)
{
    std::unordered_map<std::string, int> foodItems = {
        {"meat", 100}, {"vegetables", 200}, {"fruits", 150}, {"water", 1000}};

    std::unordered_map<std::string, int> medicineItems = {{"antibiotics", 50}, {"analgesics", 100}, {"bandages", 100}};

    std::unordered_map<std::string, int> alertsItems = {
        {"north_entry", 0}, {"east_entry", 0}, {"west_entry", 0}, {"south_entry", 0}};
    std::unordered_map<std::string, std::string> emergenciesItems = {{"last_event", "NULL"}, {"date", "NULL"}};

    nlohmann::json initialData;
    initialData["food"] = foodItems;
    initialData["medicine"] = medicineItems;
    initialData["alerts"] = alertsItems;
    initialData["emergencies"] = emergenciesItems;

    std::string jsonData = initialData.dump();

    db.put("data", jsonData);
}

void Server::setFileReadComplete(bool value)
{
    fileReadComplete = value;
}

bool Server::getFileReadComplete()
{
    return fileReadComplete;
}

std::string Server::GenerateToken()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::string token;
    for (int i = 0; i < 32; ++i)
    {
        int num = dis(gen);
        token += (num < 10) ? ('0' + num) : ('A' + num - 10);
    }
    return token;
}

void Server::ProcessImageRequests()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondition.wait(lock, [this] { return !imageRequestQueue.empty(); });

        while (!imageRequestQueue.empty())
        {
            auto request = imageRequestQueue.front();
            imageRequestQueue.pop();
            lock.unlock();

            int clientSocket = request.first;
            std::string token = request.second;

            try
            {
                SendChunksTCPv6(*this, clientSocket, chunks);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error processing image request: " << e.what() << std::endl;
            }

            lock.lock();
        }
    }
}

bool Server::HandleModifyCommand(const std::string& command)
{
    std::istringstream iss(command);
    std::vector<std::string> parts((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
    if (parts.size() != PART3 || parts[PART0] != "modify")
    {
        return false;
    }

    std::string field = parts[PART1];
    int amount = std::stoi(parts[PART2]);

    std::array<std::string, 7> ValidFields = {"meat",        "vegetables", "fruits",  "water",
                                              "antibiotics", "analgesics", "bandages"};
    if (std::find(ValidFields.begin(), ValidFields.end(), field) == ValidFields.end())
    {
        return false;
    }

    if (amount < 0)
    {
        return false;
    }
    return true;
}

void Server::HandleAuthentication(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex)
{
    std::string username = req.get_param_value("username");
    std::string password = req.get_param_value("password");
    std::string token = GenerateToken();

    bool isAuthorized = (username == UBUNTU && password == UBUNTU);
    userManager.AddUser(username, token, isAuthorized, 0);

    if (isAuthorized)
    {
        res.set_content("Authorized client,username:" + token, "text/plain");
        std::optional<std::string> optUsername = userManager.GetUserFromToken(token);
        if (optUsername.has_value())
        {
            LogActivity("New authorized client", LogMutex, optUsername.value());
        }
    }
    else
    {
        res.set_content("Unauthorized client,username:" + token, "text/plain");
        std::optional<std::string> optUsername = userManager.GetUserFromToken(token);
        if (optUsername.has_value())
        {
            LogActivity("New unauthorized client", LogMutex, optUsername.value());
        }
    }
}

void Server::HandleCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex)
{
    std::string token = req.get_param_value("token");
    std::string message = req.get_param_value("command");
    std::optional<std::string> optUsername = userManager.GetUserFromToken(token);
    if (optUsername.has_value())
    {
        std::cout << "The client " << optUsername.value() << " sent: " << message << std::endl;
    }
    if (message == "end")
    {
        res.set_content("End", "text/plain");
        if (optUsername.has_value())
        {
            LogActivity("Finished the execution of the client ", LogMutex, optUsername.value());
            userManager.RemoveUser(token);
        }
        else
        {
            res.set_content("Loading image. Try again later", "text/plain");
        }
    }
    else if (message == "image")
    {
        if (getFileReadComplete())
        {
            std::string username = optUsername.value();
            std::optional<int> clientSocketOpt = GetUserSocket(token);
            if (clientSocketOpt.has_value())
            {
                int clientSocket = clientSocketOpt.value();

                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    imageRequestQueue.emplace(clientSocket, token);
                }
                queueCondition.notify_one();

                res.set_content("Sending image...", "text/plain");
                LogActivity("Image request made by the client ", LogMutex, optUsername.value());
            }
            else
            {
                res.set_content("No active connection found for user", "text/plain");
            }
        }
        else
        {
            res.set_content("Loading image. Try again later", "text/plain");
        }
    }
    else if (HandleModifyCommand(message))
    {
        if (optUsername.has_value())
        {
            std::optional<bool> isAuthorized = userManager.IsUserAuthorized(token);
            if (isAuthorized.value() && isAuthorized.has_value())
            {
                std::istringstream iss(message);
                std::vector<std::string> parts((std::istream_iterator<std::string>(iss)),
                                               std::istream_iterator<std::string>());
                std::string field = parts[PART1];
                int amount = std::stoi(parts[PART2]);
                if (Modifysupplies(field, amount))
                {
                    res.set_content("Modified", "text/plain");
                    LogActivity("Modification made by the client ", LogMutex, optUsername.value());
                }
                else
                {
                    res.set_content("error,try again", "text/plain");
                }
            }
            else
            {
                res.set_content("You don't have permission to modify", "text/plain");
            }
        }
    }
    else
    {
        res.set_content("Unknown command", "text/plain");
    }
}

void Server::HandleSuppliesCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex)
{
    std::lock_guard<std::mutex> lock(DbMutex);
    std::cout << "The client asked supplies" << std::endl;
    std::string jsonData;
    if (db.get("data", jsonData))
    {
        auto data = nlohmann::json::parse(jsonData);
        nlohmann::json suppliesData;
        suppliesData["food"] = data["food"];
        suppliesData["medicine"] = data["medicine"];
        res.set_content(suppliesData.dump(), "application/json");
    }
    else
    {
        res.set_content("Failed to retrieve state", "text/plain");
    }
}

void Server::HandleAlertsCommand(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex)
{
    std::lock_guard<std::mutex> lock(DbMutex);
    std::cout << "The client asked alerts" << std::endl;
    std::string jsonData;
    if (db.get("data", jsonData))
    {
        auto data = nlohmann::json::parse(jsonData);
        nlohmann::json alertsData;
        alertsData["alerts"] = data["alerts"];
        alertsData["emergencies"] = data["emergencies"];
        res.set_content(alertsData.dump(), "application/json");
    }
    else
    {
        res.set_content("Failed to retrieve state", "text/plain");
    }
}

void Server::HandlePostRequest(const httplib::Request& req, httplib::Response& res, std::mutex& LogMutex)
{
    std::string command = req.get_param_value("command");
    if (command == "authenticate")
    {
        HandleAuthentication(req, res, LogMutex);
    }
    else
    {
        HandleCommand(req, res, LogMutex);
    }
}

bool Server::Modifysupplies(const std::string& field, int amount)
{
    std::lock_guard<std::mutex> lock(DbMutex);
    std::string jsonData;
    if (db.get("data", jsonData))
    {
        nlohmann::json data = nlohmann::json::parse(jsonData);
        if (data["food"].contains(field))
        {
            data["food"][field] = amount;
        }
        else if (data["medicine"].contains(field))
        {
            data["medicine"][field] = amount;
        }
        else
        {
            return false;
        }
        db.put("data", data.dump());
        return true;
    }
    return false;
}

void Server::SetTokenSocket(const std::string& token, int socket)
{
    userManager.UpdateTokenSocket(token, socket);
}

std::optional<int> Server::GetUserSocket(const std::string& token)
{
    return userManager.GetSocketFromToken(token);
}

void Server::notifyFileReadComplete()
{
    std::lock_guard<std::mutex> lock(ImgFlagMutex);
    fileReadComplete = true;
    fileReadCondition.notify_all();
}

void Server::waitForFileReadComplete()
{
    std::unique_lock<std::mutex> lock(ImgFlagMutex);
    fileReadCondition.wait(lock, [this] { return fileReadComplete; });
}

void RunTempAlert(Server& server, std::mutex& LogMutex)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds((rand() % 4) + 5));
        std::string alert = TempAlert();
        if (alert.find("Alert of possible infection") != std::string::npos)
        {
            std::cout << alert << std::endl;
            LogActivity(alert, LogMutex);
            std::istringstream iss(alert);
            std::set<std::string> valid_entries = {"north_entry", "east_entry", "west_entry", "south_entry"};
            std::string word;
            std::string entry;
            while (iss >> word)
            {
                if (valid_entries.find(word) != valid_entries.end())
                {
                    entry = word;
                }
            }
            ModifyAlertsAndEmergencies(server, entry, "1");
        }
    }
}

void RunEmergNotif(Server& server, std::mutex& LogMutex)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds((rand() % 6) + 25));
        std::string emergency = EmergNotif();
        std::cout << emergency << std::endl;
        LogActivity(emergency, LogMutex);
        auto CurrentTime = std::chrono::system_clock::now();
        std::time_t TimeTCurrentTime = std::chrono::system_clock::to_time_t(CurrentTime);
        std::tm* LocalTime = std::localtime(&TimeTCurrentTime);

        std::array<char, LOGBUF> buf;
        std::strftime(buf.data(), buf.size(), "%a %b %d %T %Y", LocalTime);
        ModifyAlertsAndEmergencies(server, "last_event", emergency);
        ModifyAlertsAndEmergencies(server, "date", buf.data());
    }
}

void SignalHandlerFunction(int SigNum)
{
    std::cout << " Ending program" << std::endl;
    exit(0);
}

void compressImage(const std::string& inputPath, const std::string& outputPath)
{
    std::cout << "starting to compress the image" << std::endl;

    std::string command = "tar -czvf " + outputPath + " -C " + inputPath.substr(0, inputPath.find_last_of("/")) + " " +
                          inputPath.substr(inputPath.find_last_of("/") + 1);
    int result = system(command.c_str());

    if (result != 0)
    {
        throw std::runtime_error("Error compressing image with tar.gz");
    }

    std::cout << "finish to compress the image" << std::endl;
}

std::vector<std::vector<char>> readInChunks(const std::string& filePath, std::size_t chunkSize)
{
    std::cout << "starting file reading" << std::endl;
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Error opening file: " + filePath);
    }

    std::vector<std::vector<char>> chunks;

    while (!file.eof())
    {
        std::vector<char> chunk(chunkSize);
        file.read(chunk.data(), chunkSize);

        if (file.gcount() < chunkSize)
        {
            chunk.resize(file.gcount());
        }

        chunks.push_back(chunk);
    }
    std::cout << "finish file reading" << std::endl;
    return chunks;
}

void SendChunksTCPv6(Server& server, int socket, const std::vector<std::vector<char>>& chunks)
{
    int NumBytes;
    std::array<char, BUFSIZESERVER> buffer;
    std::stringstream ss;

    std::ifstream file(COMPPATH, std::ios::binary | std::ios::ate);
    std::streamsize sizeCompressedImage = file.tellg();
    file.close();

    server.waitForFileReadComplete();

    if (chunks.empty())
    {
        std::cerr << "Error: Chunks are empty" << std::endl;
        return;
    }
    std::streamsize bytesSentTotal = 0;
    for (const auto& chunk : chunks)
    {
        size_t TotalSent = 0;
        size_t ChunkSize = chunk.size();
        while (TotalSent < ChunkSize)
        {
            ssize_t bytesSent = send(socket, chunk.data() + TotalSent, ChunkSize - TotalSent, 0);
            if (bytesSent < 0)
            {
                throw std::runtime_error("Error sending data through the socket");
            }
            TotalSent += bytesSent;
            bytesSentTotal += bytesSent;

            if (bytesSentTotal >= sizeCompressedImage)
            {
                break;
            }
        }
        if (bytesSentTotal >= sizeCompressedImage)
        {
            break;
        }
    }
    std::cout << "Finished sending chunks to socket:" << socket << std::endl;
}

void SocketUsername(Server& server, int clientSocket)
{
    int NumBytes;
    std::stringstream ss;
    std::array<char, BUFSIZESERVER> buffer;
    std::ifstream file(COMPPATH, std::ios::binary | std::ios::ate);
    std::streamsize sizeCompressedImage = file.tellg();
    file.close();
    NumBytes = send(clientSocket, &sizeCompressedImage, sizeof(sizeCompressedImage), 0);
    if (NumBytes < 0)
    {
        throw std::runtime_error("Error writing to socket.");
    }
    std::fill(buffer.begin(), buffer.end(), 0);
    NumBytes = read(clientSocket, buffer.data(), BUFSIZESERVER);
    if (NumBytes < 0)
    {
        throw std::runtime_error("Error reading from socket.");
    }
    std::string receivedMessage(buffer.data(), NumBytes);
    server.SetTokenSocket(buffer.data(), clientSocket);
}

void ConnectToTCPv6Client(Server& server)
{
    try
    {
        std::string port = std::to_string(ReadPortFromThirdLine(CONFPATH));
        auto TCPv6Client = std::make_unique<TCPv6Connection>("", port, true);
        if (TCPv6Client->bind())
        {
            while (true)
            {
                try
                {
                    int clientSocket = TCPv6Client->connect();
                    if (clientSocket != -1)
                    {
                        std::cout << "socket connected successfully" << std::endl;
                        std::thread ClientThread(SocketUsername, std::ref(server), clientSocket);
                        ClientThread.detach();
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Error in connect: " << e.what() << std::endl;
                }
            }
        }
        else
        {
            std::cerr << "Failed to bind the server" << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error in tcpv6Server: " << e.what() << std::endl;
    }
}

Timer::Timer() : m_StartTimepoint(std::chrono::high_resolution_clock::now()), mStopped(false)
{
}

Timer::~Timer()
{
    if (!mStopped)
    {
        Stop();
    }
}

void Timer::Stop()
{
    auto endTimepoint = std::chrono::high_resolution_clock::now();

    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
    auto duration = end - start;
    double ms = duration * 0.001;

    std::cout << "Canny process time:" << duration << "us (" << ms << "ms)\n";
    mStopped = true;
}

void CannyCompressAndRead(Server& server, std::size_t chunkSize)
{
    Timer timer;
    EdgeDetection edgeDetection(40.0, 80.0, 1.0);
    edgeDetection.cannyEdgeDetection(IMAGEPATH, DESTIMAGE);
    timer.Stop();
    std::cout << "Finished Canny Edge Detection" << std::endl;
    try
    {
        compressImage(DESTIMAGE, COMPPATH);
        server.chunks = readInChunks(COMPPATH, chunkSize);

        server.notifyFileReadComplete();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
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

void ModifyAlertsAndEmergencies(Server& server, const std::string& field, const nlohmann::json& value)
{
    std::lock_guard<std::mutex> lock(server.DbMutex);
    std::string jsonData;

    if (server.db.get("data", jsonData))
    {
        nlohmann::json data = nlohmann::json::parse(jsonData);

        if (data["alerts"].contains(field))
        {
            data["alerts"][field] = data["alerts"][field].get<int>() + 1;
        }
        else if (data["emergencies"].contains(field))
        {
            data["emergencies"][field] = value;
        }

        server.db.put("data", data.dump());
    }
}

int ReadPortFromFile(const std::string& filename)
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
    return port;
}

int main(int argc, char* argv[])
{
    int port = ReadPortFromFile(CONFPATH);
    std::mutex LogMutex;
    Server server;
    std::size_t chunkSize = CHUNKSIZE * CHUNKSIZE;

    std::thread compressionThread(CannyCompressAndRead, std::ref(server), chunkSize);
    std::ofstream LogFile(LOGPATH);
    signal(SIGINT, &SignalHandlerFunction);

    std::thread AlertInvThread(RunTempAlert, std::ref(server), std::ref(LogMutex));
    std::thread EmergNotifThread(RunEmergNotif, std::ref(server), std::ref(LogMutex));

    std::thread tcpv6Thread(ConnectToTCPv6Client, std::ref(server));
    std::thread imageWorkerThread(&Server::ProcessImageRequests, &server);

    httplib::Server svr;
    svr.Post("/", [&](const httplib::Request& req, httplib::Response& res) {
        server.HandlePostRequest(req, res, LogMutex);
    });
    svr.Get("/supplies", [&](const httplib::Request& req, httplib::Response& res) {
        server.HandleSuppliesCommand(req, res, LogMutex);
    });
    svr.Get("/alerts", [&](const httplib::Request& req, httplib::Response& res) {
        server.HandleAlertsCommand(req, res, LogMutex);
    });
    svr.listen("0.0.0.0", port);

    compressionThread.join();
    AlertInvThread.join();
    EmergNotifThread.join();

    std::cout << "Server closed" << std::endl;
    return 0;
}
