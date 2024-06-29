#pragma once

#include <string>
#include <map>
#include <set>
#include <mutex>

#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#ifdef HTTP_SERVER_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

namespace serverHandler
{
    class DLL_EXPORT website_handler
    {
    public:
        website_handler();
        int load(const char* filename);
        const char* get_page(const char* filename, int request_type, std::string input, std::string text);
        void add_dictionary(std::string word);
        int check_dictionary(std::string word);
        void init_dictionary();
        char* readFile(const char* fileName);

    private:
        std::map<std::string, std::unique_ptr<char[]>> page;
        std::set<std::string> dictionary;
        std::mutex dictionaryMutex;
        int saveDataToXMLDatabase(const std::string& filename);
        int loadDataFromXMLDatabase(const std::string& filename);
    };

    class DLL_EXPORT server
    {
    public:
        server(int internet_address, int port_number = 80, int THREAD_COUNT = 10);
        ~server();
        void start();

    private:
        int new_socket();
        int bind_address();
        int start_listen(int k = 100000);
        int accept_connection();
        void connection_thread(int sock);

        int THREAD_COUNT;
        int server_up;
        int sizeof_address;
        int file_descriptor;
        struct sockaddr_in address;
    };

    extern "C" {
        DLL_EXPORT void setProcessedData(const char* data);
        DLL_EXPORT const char* getProcessedData();
        DLL_EXPORT void start_http_server();
    };
}
