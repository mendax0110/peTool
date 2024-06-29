#ifndef WEBSITE_HANDLER_H
#define WEBSITE_HANDLER_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <set>

/// @brief The html namespace \name html
namespace html
{
    /// @brief The html_parser class, which has the public and private members \class html_parser
    class html_parser
    {
    private:
        int request_type; // get => 0 , post => 1 , put=> 2
        std::string url;
        std::string text;
        std::map<std::string,std::string> request_inputs;
        void url_parser(std::string url);
    public:
        int get_request_type();
        std::string get_input(std::string a);
        std::string get_text();
        html_parser(char * buffer,int buffer_length);
    };
};

/// @brief The website_n namespace \name website_n
namespace website_n
{
    //class website_handler;;
};

#endif // WEBSITE_HANDLER_H