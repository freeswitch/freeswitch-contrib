#include <limits.h>

#include <list>
#include <vector>
#include <string>

#include <format.hpp>

#include <types.hpp>

/* Generic string funcions */

#ifndef INCLUDED_STRINGS_HPP
#define INCLUDED_STRINGS_HPP

struct Strings
{
    typedef std::list<std::string>      list_type;
    typedef std::vector<std::string>  vector_type;

    struct Merger
    {
        void          add(std::string);

        std::string merge(const std::string &);
        std::string merge(const char *);

        bool empty() { return _list.empty(); };
        
     protected:
        list_type   _list;
    };

 public:
    struct invalid_value
    {
        invalid_value(const char  * value): _value(value) {};
        invalid_value(std::string   value): _value(value) {};

        std::string & value() { return _value; }
        
     protected:
         std::string _value;
    };
    
    struct not_implemented {};

    static void tokenize(const std::string &, vector_type &, const std::string & delims = ",;:", long int max_toxens = LONG_MAX);

    static bool        toboolean(std::string);
    static std::string fromboolean(bool);

    static long               tolong(std::string, int base = 10);
    static unsigned long      toulong(std::string, int base = 10);
    static unsigned long long toulonglong(std::string, int base = 10);

    static std::string lower(std::string);
    static std::string hexadecimal(std::string);
    
    static std::string trim(const std::string&, const std::string& trim_chars = " \f\n\r\t\v");
};

#endif // INCLUDED_STRINGS_HPP //
