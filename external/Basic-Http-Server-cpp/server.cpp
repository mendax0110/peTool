#include <iostream>
#include <fstream>
#include <algorithm>
#include <condition_variable>
#include <filesystem>
#include <thread>

#if defined(__LINUX__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "server.h"

using namespace serverHandler;

website_handler::website_handler()
{
}

char* website_handler::readFile(const char* fileName)
{
    FILE* file = fopen(fileName, "rb");
    if (file == nullptr)
    {
        perror("Error opening file");
        return nullptr;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    if (fileSize <= 0)
    {
        fclose(file);
        std::cerr << "Error: File size is 0 or negative." << std::endl;
        return nullptr;
    }

    std::cout << "File size: " << fileSize << " bytes" << std::endl;

    char* code = new (std::nothrow) char[fileSize + 1];
    if (code == nullptr)
    {
        perror("Error allocating memory");
        fclose(file);
        return nullptr;
    }

    size_t bytesRead = fread(code, sizeof(char), fileSize, file);
    fclose(file);

    if (bytesRead != static_cast<size_t>(fileSize))
    {
        if (feof(file))
        {
            std::cerr << "Error: End of file reached prematurely\n";
        }
        else if (ferror(file))
        {
            std::cerr << "Error reading file: " << strerror(errno) << "\n";
        }

        delete[] code; // Free memory on error
        return nullptr;
    }

    code[fileSize] = '\0';
    return code;
}

int website_handler::load(const char* filename)
{
    if (page.find(filename) != page.end())
    {
        printf("File '%s' is already loaded.\n", filename);
        return 0;
    }

    char* fileContent = readFile(filename);
    if (fileContent == nullptr)
    {
        printf("Error loading file '%s'.\n", filename);
        return 1;
    }

    page[filename] = std::unique_ptr<char[]>(fileContent);
    return 0;
}

const char* website_handler::get_page(const char* filename, int request_type, std::string input, std::string text)
{
    if (std::filesystem::exists(filename))
    {
        std::string str = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=ISO-8859-1\r\n\r\n" + std::string(page[filename].get());
        if (request_type == 1)
        {
            add_dictionary(text);
        }
        else if (request_type == 0 && !input.empty())
        {
            int is_contains = check_dictionary(input);
            if (is_contains)
            {
                input += " is found in your Dictionary";
                size_t pos = str.find("<!Rvalue>");
                if (pos != std::string::npos)
                {
                    str.replace(pos, 9, input);
                }
            }
            else
            {
                input += " is NOT found in your Dictionary";
                size_t pos = str.find("<!Rvalue>");
                if (pos != std::string::npos)
                {
                    str.replace(pos, 9, input);
                }
            }
        }

        char* cstr = new char[str.length() + 1];
        strncpy(cstr, str.c_str(), str.length() + 1);
        return cstr;
    }
    else
    {
        std::cerr << filename << " could not be opened! Check if the path is correct\n";
        return "";
    }
}

void website_handler::add_dictionary(std::string word)
{
    if (word.empty())
        return;

    std::lock_guard<std::mutex> lock(dictionaryMutex);
    if (dictionary.count(word) == 0)
    {
        dictionary.insert(word);

        int saveResult = saveDataToXMLDatabase("DATABASE.xml");
        if (saveResult == 0)
        {
            std::cout << "Data saved to DATABASE.xml" << std::endl;
        }
        else
        {
            std::cerr << "Failed to save data to DATABASE.xml" << std::endl;
        }

        std::ofstream fDictionary("dictionary.txt", std::ios::app);
        if (fDictionary.good())
        {
            fDictionary << word << "\n";
            fDictionary.close();
        }
        else
        {
            std::cerr << "Failed to open file\n";
        }
    }
}

int website_handler::check_dictionary(std::string word)
{
    std::lock_guard<std::mutex> lock(dictionaryMutex);
    return dictionary.count(word);
}

void website_handler::init_dictionary()
{
    std::lock_guard<std::mutex> lock(dictionaryMutex);
    dictionary.insert("");
    std::ifstream fp("dictionary.txt");
    std::ifstream fp2("DATABASE.xml");
    if (!fp.good() && fp2.good())
    {
        std::ofstream created("dictionary.txt");
        std::ofstream created2("DATABASE.xml");
        created.close();
        created2.close();
        fp.close();
        fp2.close();
        return;
    }

    std::string word;
    char c;
    while ((c = fp.get()) != EOF && fp2.get() != EOF)
    {
        if (c == '\n')
        {
            dictionary.insert(word);
            word.clear();
        }
        else
            word.push_back(c);
    }

    if (!word.empty())
        dictionary.insert(word);
    fp.close();
    fp2.close();
}

int website_handler::saveDataToXMLDatabase(const std::string& filename)
{
    std::ofstream file(filename);
    if (file.is_open())
    {
        int entryNumber = 1;
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
        file << "<Database>" << std::endl;

        for (const std::string& item : dictionary)
        {
            if (!item.empty() && !std::all_of(item.begin(), item.end(), ::isspace))
            {
                file << "  <Entry_" << entryNumber << ">" << item << "</Entry_" << entryNumber << ">" << std::endl;
                entryNumber++;
            }
        }

        file << "</Database>" << std::endl;
        file.close();
        return 0;
    }
    else
    {
        std::cerr << "Error opening file for saving data." << std::endl;
        return 1;
    }
}

int website_handler::loadDataFromXMLDatabase(const std::string& filename)
{
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;

        while (std::getline(file, line))
        {
            if (line.find("<Entry_") != std::string::npos &&
                line.find("</Entry_") != std::string::npos)
            {
                std::string word = line.substr(line.find("<Entry_") + 7, line.find("</Entry_") - 7);
                dictionary.insert(word);
            }
        }

        file.close();
        return 0;
    }
    else
    {
        std::cerr << "Error opening file for loading data." << std::endl;
        return 1;
    }
}

server::server(int internet_address, int port_number, int THREAD_COUNT)
{
    this->THREAD_COUNT = THREAD_COUNT;
    server_up = 0;
    sizeof_address = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(internet_address);
    address.sin_port = htons(port_number);
    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    bind_address();
    start_listen();
}

server::~server()
{
#if defined(_WIN32) || defined(_WIN64)
    closesocket(file_descriptor);
#else
    close(file_descriptor);
#endif
}

void server::start()
{
    server_up = 1;
    while (server_up)
    {
        accept_connection();
    }
}

int server::new_socket()
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int server::bind_address()
{
    return bind(file_descriptor, (struct sockaddr*)&address, sizeof_address);
}

int server::start_listen(int k)
{
    return listen(file_descriptor, k);
}

int server::accept_connection()
{
    struct sockaddr_in client_addr;
    socklen_t c = sizeof(struct sockaddr_in);
    int sock = accept(file_descriptor, (struct sockaddr*)&client_addr, &c);

    if (sock == -1)
    {
        std::cerr << "Failed to accept connection" << std::endl;
        return -1;
    }

    std::thread connectionThread(&server::connection_thread, this, sock);
    connectionThread.detach();

    return 0;
}

void server::connection_thread(int sock)
{
    char buf[1024] = { 0 };
    const char* str = "<!DOCTYPE html><html><head></head><body><h1>Hello, World!</h1></body></html>";

#ifdef _WIN32
    recv(sock, buf, 1024, 0);
#else
    read(sock, buf, 1024);
#endif
    send(sock, str, strlen(str), 0);
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void setProcessedData(const char* data)
{
    std::string temp_data(data);
    std::transform(temp_data.begin(), temp_data.end(), temp_data.begin(), ::toupper);
    std::cout << "Processed Data: " << temp_data << std::endl;
}

const char* getProcessedData()
{
    return "Dummy Data";
}

void start_http_server()
{
    std::cout << "HTTP Server started." << std::endl;
}
