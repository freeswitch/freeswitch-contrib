#include <set>
#include <map>
#include <vector>

#include <strings.hpp>

#include <format.hpp>

#ifndef INCLUDED_CONFIG_OPTIONS
#define INCLUDED_CONFIG_OPTIONS

template < typename base_type >
struct Variant
{
    struct InvalidType {};

    Variant(base_type * value) : _value(value) 
    {
        _count = new int(1);
    };

    Variant(const Variant & v) : _value(v._value), _count(v._count)
    {
       
       (*_count)++;
    }

    ~Variant()
    {
        (*_count)--;
        if (_value && !(*_count)) 
        {
            delete _value;
            delete _count;
        }
    };

    template < typename return_type >
    return_type & get(void)
    {
        try
        {
            return_type & ret = dynamic_cast < return_type & > (*_value);
            return ret;
        }
        catch (std::bad_cast & e)
        {
            throw InvalidType();
        }

    };

    int which()
    {
        return _value->which();
    }

 protected:
    base_type * _value;
    int *_count;
};

struct config_process_failure
{
    config_process_failure(std::string _msg): msg(_msg) {};
    std::string msg;
};

struct config_option
{
    typedef int          sint_type;
    typedef unsigned int uint_type;
    typedef bool         bool_type;

    typedef std::string                          string_type;
    //typedef boost::function1<void, std::string>     
    typedef void (*fun_type)(std::string);

    typedef std::set < string_type >     string_allowed_type;
    
    /* this should reflect 'variant.which()'! */
    typedef enum
    {
        ID_SINT   = 0,
        ID_UINT   = 1,
        ID_BOOL   = 2,
        ID_STRING = 3,
        ID_FUN    = 4,
    }
    value_id_type;

    template <typename number_type>
    struct range
    {
        range(number_type _minimum, number_type _maximum, number_type _step)
        : minimum(_minimum), maximum(_maximum), step(_step) {};

        number_type minimum, maximum, step;
    };

    struct BaseType
    {
        BaseType() {}

        virtual ~BaseType() {} 

        virtual int which() = 0;
    };

    struct sint_data_type : public BaseType
    {
        sint_data_type(sint_type & _sint_val, sint_type _sint_default, range<sint_type> _sint_range)
        : sint_val(_sint_val), sint_default(_sint_default), sint_range(_sint_range) {};

        int which()
        {
            return ID_SINT;
        }

        sint_type        & sint_val;
        sint_type          sint_default;
        range<sint_type>   sint_range;
    };

    struct uint_data_type : public BaseType
    {
        uint_data_type(uint_type & _uint_val, uint_type _uint_default, range<uint_type> _uint_range)
        : uint_val(_uint_val), uint_default(_uint_default), uint_range(_uint_range) {};

        int which()
        {
            return ID_UINT;
        }

        uint_type        & uint_val;
        uint_type          uint_default;
        range<uint_type>   uint_range;
    };

    struct bool_data_type : public BaseType
    {
        bool_data_type(bool_type & _bool_val, bool_type _bool_default)
        : bool_val(_bool_val), bool_default(_bool_default) {};
        
        int which()
        {
            return ID_BOOL;
        }

        bool_type      & bool_val;
        bool_type        bool_default;
    };

    struct string_data_type : public BaseType
    {
        string_data_type(std::string & _string_val, std::string _string_default, string_allowed_type _string_allowed)
        : string_val(_string_val), string_default(_string_default), string_allowed(_string_allowed) {};
        
        int which()
        {
            return ID_STRING;
        }

        std::string         & string_val;
        std::string           string_default;
        string_allowed_type   string_allowed;
    };

    struct fun_data_type : public BaseType
    {
        fun_data_type(fun_type _fun_val, std::string _fun_default, string_allowed_type _fun_allowed)
        : fun_val(_fun_val), fun_default(_fun_default), fun_allowed(_fun_allowed) {};
        
        int which()
        {
            return ID_FUN;
        }

        fun_type             fun_val;
        std::string          fun_default;
        string_allowed_type  fun_allowed;
    };


    typedef Variant < BaseType >  value_type;
    //typedef boost::variant < sint_data_type , uint_data_type, bool_data_type, string_data_type, fun_data_type >  value_type;

    config_option(std::string, const string_type &, const string_type, string_allowed_type allowed, bool list_me = true);
    config_option(std::string, const string_type &, const string_type = "", bool list_me = true);
    config_option(std::string, const sint_type &, const sint_type = 0, sint_type min = -INT_MAX, sint_type max = INT_MAX, sint_type step = 1, bool list_me = true);
    config_option(std::string, const uint_type &, const uint_type = 0, uint_type min = 0, uint_type max = UINT_MAX, uint_type step = 1, bool list_me = true);
    config_option(std::string, const bool_type &, const bool_type = false, bool list_me = true);
    config_option(std::string, fun_type, std::string defvalue, string_allowed_type allowed, bool list_me = true);
    config_option(std::string, fun_type, std::string defvalue = "", bool list_me = true);

    ~config_option(void);

    void set(string_type value);

    void set(sint_type value);
    void set(uint_type value);
    void set(bool_type value);

    std::string    & name(void);
    value_id_type    type(void);

    const char **  values(void);

    void            reset(void);
    void           commit(void);

    bool          list_me(void) { return _list_me; };
    bool           loaded(void) { return _loaded;  };

    void        copy_from(config_option &);

 protected:
    std::string       _my_name;
    value_type     _value_data;

    bool              _list_me;
    const char **      _values;
    bool               _loaded;
};

struct config_options
{
    typedef std::vector < std::string >    messages_type;

    config_options(void): _values(NULL) {};

    typedef std::set < std::string >  string_set;

    typedef std::map  < std::string, config_option >   option_map_type;
    typedef std::pair < std::string, config_option >  option_pair_type;

    typedef std::map  < std::string, std::string >   syn_option_map_type;
    typedef std::pair < std::string, std::string >  syn_option_pair_type;

    bool add(config_option option);

    /* only valid in "process" (for backwards compatibility config files) */
    bool synonym(std::string, std::string);

    template <typename value_type>
    void set(std::string name, value_type value)
    {
        option_map_type::iterator iter = _map.find(name);

        if (iter == _map.end())
            throw config_process_failure(STG(FMT("unknown option: %s") % name));

        (*iter).second.set(value);
    }

    string_set options(void);

    void process(const char *, const char *); /* process option from file */

    void           reset(void);         /* reset loaded opts */
    messages_type commit(void);         /* set defaults */

    const char ** values(const char *); /* option value */
    const char ** values(void);         /* values from options */

    bool          loaded(std::string);  /* return if config was loaded */

    void          copy_from(config_options &, std::string);

 protected:
    option_map_type::iterator find_option(std::string);

 protected:
    option_map_type      _map;
    syn_option_map_type  _syn_map;

    const char ** _values;
};

#endif /* INCLUDED_CONFIG_OPTIONS */

