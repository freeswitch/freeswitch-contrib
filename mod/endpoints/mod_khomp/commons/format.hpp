#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <string>
#include <queue>
#include <set>
#include <typeinfo>
#include <cstring>
#include <cstdlib>

/* macros used for shortening lines and making the code clearer */
#define STG(x) (x).str()
#define FMT(x) Format(x)

struct Format
{
    static const char * INVALID_MSG;

    struct Argument
    {
        typedef std::set < std::string > StringSet;

        Argument(std::string init_fmts, std::string init_type)
            : _fmts(init_fmts) 
        {
            _types.insert(init_type);
        };

        void type(std::string t)
        {
            _types.insert(t);
        }

        bool compare(const char * type)
        {
            for(StringSet::iterator it = _types.begin(); it != _types.end(); it++)
            {
                if(!strcmp(type, (*it).c_str()))
                    return true;
            }
            return false;
        }

        std::string fmts()
        {
            return _fmts;
        }

    protected:
        std::string _fmts;
        StringSet _types;
    };

    struct InvalidFormat
    {
        InvalidFormat(std::string msg) : _msg(msg) {}

        std::string _msg;
    };


    typedef std::queue < Argument *> ArgumentsQueue;

    Format(const char * fmt, bool exception = false);
    
    std::string str();

    template < typename Type >
    Format & operator % ( Type value )
    {
        Argument * top;
        bool text;

        do
        {
            text = false;

            if(_args.empty()) {
                _valid = false;
                if(_exception)
                    throw InvalidFormat(INVALID_MSG);
                return *this;
            }

            top = _args.front();
            _args.pop();

            if(top->compare("text"))
            {
                _result += top->fmts();
                text = true;
                free(top);
            }
            
        }
        while(text);

        char tmp[1000];

        if(top->compare(""))
        {
            snprintf(tmp, 1000, top->fmts().c_str(), value);
            _result += tmp;
        } 
        else if (top->compare(typeid(Type).name()))
        {
            snprintf(tmp, 1000, top->fmts().c_str(), value);
            _result += tmp;
        } 
        else 
        {
            free(top);
            _valid = false;
            if(_exception)
                throw InvalidFormat(INVALID_MSG);
            return *this;
        }

        free(top);

       return *this;
    }

Format & operator%( std::string & value )
{
    Argument * top;
    bool text;

    do
    {
        text = false;

        if(_args.empty()) {
            _valid = false;
            if(_exception)
                throw InvalidFormat(INVALID_MSG);
            return *this;
        }

        top = _args.front();
        _args.pop();

        if(top->compare("text"))
        {
            _result += top->fmts();
            text = true;
            free(top);
        }
        
    }
    while(text);

    char tmp[1000];

    if (top->compare(typeid(char *).name()))
    {
        snprintf(tmp, 1000, top->fmts().c_str(), value.c_str());
        _result += tmp;
    } 
    else 
    {
        free(top);
        _valid = false;
        if(_exception)
            throw InvalidFormat(INVALID_MSG);
        return *this;
    }

    free(top);

   return *this;
}

private:
    bool _valid;
    bool _exception;
    std::string _result;
    ArgumentsQueue _args;

};

#endif /* _FORMAT_H_ */

