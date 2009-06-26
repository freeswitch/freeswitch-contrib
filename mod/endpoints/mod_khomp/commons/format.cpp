#include "format.hpp"

const char * Format::INVALID_MSG = "** Invalid Format **";


Format::Format(const char * fmt, bool exception)
    : _valid(true), _exception(exception)
{
    std::string text;
    std::string arg;

    const char * ptr = fmt;

    while(*ptr != '\0')
    {
        if (*ptr != '%')
        {
            text += *ptr;
            ptr++;
            continue;
        } 
        else if(*(ptr+1) == '%') 
        {
            text += *ptr;
            ptr++;
            text += *ptr;
            ptr++;
            continue;
        }

        _args.push(new Argument(text, "text"));

        text.clear();

        arg += *ptr;

        bool finished = false;

        ptr++;

        while(*ptr != '\0' && !finished) 
        {
        
            Argument * argument;

            switch (*ptr)
            {
            case ' ':
                arg += *ptr;
                _args.push(new Argument(arg, "text"));
                arg.clear();
                finished = true;
                break;
            
            case '%':
                _args.push(new Argument(arg, "text"));
                arg.clear();
                arg += *ptr;
                break;

            case 'h':
                arg += *ptr;
                break;

            case 'l':
                arg += *ptr;
                break;

            case 'd':
            case 'i':
                arg += *ptr;
                argument = new Argument(arg, typeid(int).name());
                argument->type(typeid(unsigned int).name());
                _args.push(argument);
                argument = NULL;
                arg.clear();
                finished = true;
                break;

            case 'o':
            case 'u':
            case 'x':
            case 'X':
                arg += *ptr;
                argument = new Argument(arg, typeid(unsigned int).name());
                argument->type(typeid(int).name());
                _args.push(argument);
                argument = NULL;
                arg.clear();
                finished = true;
                break;

            case 'e':
            case 'E':
            case 'f':
            case 'F':
            case 'g':
            case 'G':
            case 'a':
            case 'A':
                arg += *ptr;
                argument = new Argument(arg, typeid(double).name());
                argument->type(typeid(float).name()); 
                _args.push(argument);
                argument = NULL;
                arg.clear();
                finished = true;
                break;

            case 'c':
                arg += *ptr;
                argument = new Argument(arg, typeid(char).name());
                argument->type(typeid(unsigned char).name());
                _args.push(argument);
                argument = NULL;
                arg.clear();
                finished = true;
                break;

            case 's':
                arg += *ptr;
                argument = new Argument(arg, typeid(const char *).name());
                argument->type(typeid(char *).name());
                argument->type(typeid(const unsigned char *).name());
                argument->type(typeid(unsigned char *).name());
                _args.push(argument);
                argument = NULL;
                arg.clear();
                finished = true;
                break;

            case 'p':
            case 'C':
            case 'S':
            case 'n':
            case 'm':
                arg += *ptr;
                _args.push(new Argument(arg, ""));
                arg.clear();
                finished = true;
                break;
            default:
                arg += *ptr;
                break;
            }
        
            ptr++;

        }
    }
    
    if(!text.empty())
        _args.push(new Argument(text, "text"));
    
    if(!arg.empty())
        _args.push(new Argument(arg, "text"));

}

std::string Format::str()
{

    if(!_valid) 
    {
        if(_exception)
            throw InvalidFormat(INVALID_MSG);

        _result.clear();
        _result += INVALID_MSG;
        return _result;
    }

    Argument * top;
    bool text;

    do
    {
        text = false;

        if(_args.empty())
            return _result;

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

    free(top);

    _valid = false;

    if(_exception)
        throw InvalidFormat(INVALID_MSG);

    _result.clear();
    _result += INVALID_MSG;
    return _result;
}


