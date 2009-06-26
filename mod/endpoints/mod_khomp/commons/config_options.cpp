#include <config_options.hpp>

config_option::config_option(std::string name, const config_option::string_type & value, const config_option::string_type defvalue, string_allowed_type allowed, bool list_me)
: _my_name(name), _value_data(new string_data_type(const_cast<string_type &>(value), defvalue, allowed)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, const config_option::string_type & value, const config_option::string_type defvalue, bool list_me)
: _my_name(name), _value_data(new string_data_type(const_cast<string_type &>(value), defvalue, string_allowed_type())),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, const config_option::sint_type & value, const config_option::sint_type defvalue,
                             config_option::sint_type min, config_option::sint_type max, config_option::sint_type step, bool list_me)
: _my_name(name), _value_data(new sint_data_type(const_cast<sint_type &>(value), defvalue, range<sint_type>(min, max, step))),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, const config_option::uint_type & value, const config_option::uint_type defvalue,
                             config_option::uint_type min, config_option::uint_type max, config_option::uint_type step, bool list_me)
: _my_name(name), _value_data(new uint_data_type(const_cast<uint_type &>(value), defvalue, range<uint_type>(min, max, step))),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, const config_option::bool_type & value, const config_option::bool_type defvalue, bool list_me)
: _my_name(name), _value_data(new bool_data_type(const_cast<bool_type &>(value), defvalue)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, config_option::fun_type fun, std::string defvalue, string_allowed_type allowed, bool list_me)
: _my_name(name), _value_data(new fun_data_type(fun, defvalue, allowed)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::config_option(std::string name, config_option::fun_type fun, std::string defvalue, bool list_me)
: _my_name(name), _value_data(new fun_data_type(fun, defvalue, string_allowed_type())),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

config_option::~config_option(void)
{
    if (_values)
    {
        for (unsigned int i = 0; _values[i] != NULL; i++)
            delete _values[i];

        delete[] _values;
    }
};

void config_option::set(config_option::string_type value)
{
    switch (_value_data.which())
    {
        case ID_STRING:
        {
            try 
            {
                string_data_type & tmp = _value_data.get<string_data_type>();
            

                if (tmp.string_allowed.empty())
                {
                    tmp.string_val = value;
                    _loaded = true;
                }
                else
                {
                    if (tmp.string_allowed.find(value) != tmp.string_allowed.end())
                    {
                        tmp.string_val = value;
                        _loaded = true;
                        return;
                    }

                    std::string allowed_string;

                    for (string_allowed_type::iterator i = tmp.string_allowed.begin(); i != tmp.string_allowed.end(); i++)
                    {
                        allowed_string += " '";
                        allowed_string += (*i);
                        allowed_string += "'";
                    }
    
                    throw config_process_failure(STG(FMT("value '%s' not allowed for option '%s' (allowed values:%s)")
                        % value % _my_name % allowed_string));
                }
                break;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
        }

        case ID_FUN:
        {
            try 
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
                tmp.fun_val(value);
                _loaded = true;
            break;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

        }

        default:
        {
            throw config_process_failure(STG(FMT("option '%s' is not of type string, nor function defined") % _my_name));
        }
    }
}

void config_option::set(config_option::sint_type value)
{
    try
    {
        sint_data_type & tmp = _value_data.get<sint_data_type>();

        if (value < tmp.sint_range.minimum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too low)") % value % _my_name));

        if (value > tmp.sint_range.maximum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too high)") % value % _my_name));

        if (((value - tmp.sint_range.minimum) % tmp.sint_range.step) != 0)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (outside allowed step)") % value % _my_name));

        tmp.sint_val = value;
        _loaded = true;
    }
    catch(value_type::InvalidType & e)
    {
        throw;
    }
}

void config_option::set(config_option::uint_type value)
{
    try
    {
        uint_data_type & tmp = _value_data.get<uint_data_type>();

        if (value < tmp.uint_range.minimum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too low)") % value % _my_name));

        if (value > tmp.uint_range.maximum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too high)") % value % _my_name));

        if (((value - tmp.uint_range.minimum) % tmp.uint_range.step) != 0)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (outside allowed step)") % value % _my_name));

        tmp.uint_val = value;
        _loaded = true;
    }
    catch(value_type::InvalidType & e)
    {
        throw;
    }
}

void config_option::set(config_option::bool_type value)
{
    try
    {
        bool_data_type & tmp = _value_data.get<bool_data_type>();
        tmp.bool_val = value;
        _loaded = true;
    }
    catch(value_type::InvalidType & e)
    {
        throw;
    }

}

std::string & config_option::name(void) { return _my_name; };

config_option::value_id_type config_option::type(void) 
{ 
    return (value_id_type) _value_data.which(); 
};

const char ** config_option::values(void)
{
    if (_values != NULL)
        return _values;

    switch ((value_id_type) _value_data.which())
    {
        case config_option::ID_BOOL:
        {
            _values = new const char*[3];

            _values[0] = strdup("yes");
            _values[1] = strdup("no");
            _values[2] = NULL;

            return _values;
        }

        case config_option::ID_SINT:
        {
            try
            {
                sint_data_type & tmp = _value_data.get<sint_data_type>();


                unsigned int count = ((tmp.sint_range.maximum - tmp.sint_range.minimum) / tmp.sint_range.step) + 1;
                unsigned int index = 0;

                _values = new const char*[count + 1];

                for (sint_type i = tmp.sint_range.minimum; i <= tmp.sint_range.maximum; i += tmp.sint_range.step, index++)
                    _values[index] = strdup(STG(FMT("%d") % i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
        }

        case config_option::ID_UINT:
        {
            try
            {
                uint_data_type & tmp = _value_data.get<uint_data_type>();

                unsigned int count = ((tmp.uint_range.maximum - tmp.uint_range.minimum) / tmp.uint_range.step) + 1;
                unsigned int index = 0;

                _values = new const char*[count + 1];

                for (uint_type i = tmp.uint_range.minimum; i <= tmp.uint_range.maximum; i += tmp.uint_range.step, index++)
                    _values[index] = strdup(STG(FMT("%d") % i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
        }

        case config_option::ID_STRING:
        {
            try
            {
                string_data_type & tmp = _value_data.get<string_data_type>();
            
                _values = new const char*[ tmp.string_allowed.size() + 1 ];

                unsigned int index = 0;

                for (string_allowed_type::iterator i = tmp.string_allowed.begin(); i != tmp.string_allowed.end(); i++, index++)
                    _values[index] = strdup((*i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
        }

        case config_option::ID_FUN:
        {
            try
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
            
                _values = new const char*[ tmp.fun_allowed.size() + 1 ];

                unsigned int index = 0;
    
                for (string_allowed_type::iterator i = tmp.fun_allowed.begin(); i != tmp.fun_allowed.end(); i++, index++)
                    _values[index] = strdup((*i).c_str());

                _values[index] = NULL;
                return _values;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }
};

void config_option::reset(void)
{
    _loaded = false;
};

void config_option::commit(void)
{
    if (_loaded)
        return;

    switch ((value_id_type) _value_data.which())
    {
        case config_option::ID_BOOL:
        {
            try
            {
                bool_data_type & tmp = _value_data.get<bool_data_type>();
                tmp.bool_val = tmp.bool_default;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case config_option::ID_SINT:
        {
            try
            {
                sint_data_type & tmp = _value_data.get<sint_data_type>();
                tmp.sint_val = tmp.sint_default;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case config_option::ID_UINT:
        {
            try
            {
                uint_data_type & tmp = _value_data.get<uint_data_type>();
                tmp.uint_val = tmp.uint_default;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case config_option::ID_STRING:
        {
            try
            {
                string_data_type & tmp = _value_data.get<string_data_type>();
                tmp.string_val = tmp.string_default;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case config_option::ID_FUN:
        {
            try
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
                tmp.fun_val(tmp.fun_default);
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }
            break;
        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }

    _loaded = true;
};

void config_option::copy_from(config_option & src)
{
    if (src._value_data.which() != _value_data.which())
        throw config_process_failure(STG(FMT("unable to copy options, source type differs from destination.")));

    if (!src._loaded)
        return;

    switch ((value_id_type) _value_data.which())
    {
        case config_option::ID_BOOL:
        {
            try
            {
                bool_data_type & stmp = src._value_data.get<bool_data_type>();
                bool_data_type & dtmp = _value_data.get<bool_data_type>();
                /* do not copy references, but values.. */
                bool tmpval = stmp.bool_val;
                dtmp.bool_val = tmpval;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case config_option::ID_SINT:
        {
            try
            {
                sint_data_type & stmp = src._value_data.get<sint_data_type>();
                sint_data_type & dtmp = _value_data.get<sint_data_type>();
                /* do not copy references, but values.. */
                int tmpval = stmp.sint_val;
                dtmp.sint_val = tmpval;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case config_option::ID_UINT:
        {
            try
            {
                uint_data_type & stmp = src._value_data.get<uint_data_type>();
                uint_data_type & dtmp = _value_data.get<uint_data_type>();
                /* do not copy references, but values.. */
                unsigned int tmpval = stmp.uint_val;
                dtmp.uint_val = tmpval;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }


            break;
        }

        case config_option::ID_STRING:
        {
            try
            {
                string_data_type & stmp = src._value_data.get<string_data_type>();
                string_data_type & dtmp = _value_data.get<string_data_type>();
                /* do not copy references, but values.. */
                std::string tmpval = stmp.string_val;
                dtmp.string_val = tmpval;
            }
            catch(value_type::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case config_option::ID_FUN:
        {
            /* TO IMPLEMENT (NEEDS ANOTHER METHOD ON FUNCTION FOR GETTING VALUE) */

//            fun_data_type & tmp = boost::get<fun_data_type>(_value_data);
//
//            if (!tmp.loaded)
//            {
//                tmp.fun_val(tmp.fun_default);
//                tmp.loaded = true;
//            }
            break;
        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }

    _loaded = true;
};

/*********************************/

bool config_options::add(config_option option)
{
    //option_map_type::iterator iter2 = _map.begin();

    //boost::tie(iter2, ok2) 
    std::pair<option_map_type::iterator, bool> ret = _map.insert(option_pair_type(option.name(), option));

    return ret.second;
}

bool config_options::synonym(std::string equiv_opt, std::string main_opt)
{
    //syn_option_map_type::iterator iter = _syn_map.begin();

    //boost::tie(iter, ok) 
    std::pair<syn_option_map_type::iterator, bool> ret = _syn_map.insert(syn_option_pair_type(equiv_opt, main_opt));

    return ret.second;
}

config_options::string_set config_options::options(void)
{
    string_set res;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        res.insert((*i).first);

    return res;
}

void config_options::process(const char * name, const char * value)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s'") % name));

    try
    {
        switch ((*iter).second.type())
        {
            case config_option::ID_SINT:
                set<config_option::sint_type>((*iter).first, Strings::toulong(value));
                return;
            case config_option::ID_UINT:
                set<config_option::uint_type>((*iter).first, Strings::tolong(value));
                return;
            case config_option::ID_BOOL:
                set<config_option::bool_type>((*iter).first, Strings::toboolean(value));
                return;
            case config_option::ID_STRING:
            case config_option::ID_FUN:
                set<config_option::string_type>((*iter).first, std::string(value));
                return;
            default:
                throw config_process_failure(STG(FMT("unknown type identifier '%d'") % (*iter).second.type()));
        }
    }
    catch (Strings::invalid_value e)
    {
        throw config_process_failure(STG(FMT("invalid value '%s' for option '%s'") % value % name));
    }
}

const char ** config_options::values(const char * name)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s'") % name));

    return (*iter).second.values();
}

const char ** config_options::values(void)
{
    if (_values != NULL)
        return _values;

    unsigned int count = 0;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        if ((*i).second.list_me())
            ++count;

    _values = new const char*[ count + 1 ];

    unsigned int index = 0;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
    {
        if ((*i).second.list_me())
        {
            _values[index] = strdup((*i).first.c_str());
            ++index;
        }
    }

    _values[index] = NULL;

    return _values;
}

void config_options::reset(void)
{
    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        (*i).second.reset();
}

config_options::messages_type config_options::commit(void)
{
    messages_type msgs;
    
    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
    {
        try
        {
            (*i).second.commit();
        }
        catch (config_process_failure e)
        {
            msgs.push_back(e.msg);
        }
    }
    
    return msgs;
}

bool config_options::loaded(std::string name)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        return false;

    return iter->second.loaded();
}

void config_options::copy_from(config_options & source, std::string name)
{
    option_map_type::iterator iter_src = source.find_option(name);
    option_map_type::iterator iter_dst = find_option(name);

    if (iter_src == source._map.end())
        throw config_process_failure(STG(FMT("unknown option '%s' on source") % name));

    if (iter_dst == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s' on destination") % name));

    iter_dst->second.copy_from(iter_src->second);
}

config_options::option_map_type::iterator config_options::find_option(std::string name)
{
    syn_option_map_type::iterator syn_iter = _syn_map.find(name);

    if (syn_iter != _syn_map.end())
        name = syn_iter->second;

    option_map_type::iterator iter = _map.find(name);

    return iter;
}
