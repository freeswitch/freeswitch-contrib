#ifndef INCLUDED_TYPES_HPP
#define INCLUDED_TYPES_HPP

#if defined(LINUX) || defined(__LINUX__) || defined(__linux__) 
#include <stdlib.h> 
#endif 

/*** Used for conditional compilation based on K3L version ***/

#define K3L_AT_LEAST(major,minor,build) \
    (((k3lApiMajorVersion == major) && ((k3lApiMinorVersion == minor) && (k3lApiBuildVersion >= build)) || \
     ((k3lApiMajorVersion == major) && (k3lApiMinorVersion > minor))) || \
      (k3lApiMajorVersion  > major))

#define K3L_EXACT(major,minor,build) \
    ((k3lApiMajorVersion == major) && (k3lApiMinorVersion == minor) && (k3lApiBuildVersion >= build))

/**** Used for indexing any kind of Khomp object. ****/

typedef unsigned short int   ushort;

template <class value_type>
struct ktype_id
{
    ktype_id(value_type id): _id(id) {};

    value_type id(void) { return _id; };
    
 protected:
     value_type _id;
};

struct deviceId:  ktype_id<ushort> { deviceId(ushort id):  ktype_id<ushort>(id) {}; };
struct channelId: ktype_id<ushort> { channelId(ushort id): ktype_id<ushort>(id) {}; };
struct playerId:  ktype_id<ushort> { playerId(ushort id):  ktype_id<ushort>(id) {}; };
struct mixerId:   ktype_id<ushort> { mixerId(ushort id):   ktype_id<ushort>(id) {}; };
struct linkId:    ktype_id<ushort> { linkId(ushort id):    ktype_id<ushort>(id) {}; };

struct kobject
{
    struct hash;
    struct less;
    struct equal;
    struct hashLess;

    struct item
    {
        typedef enum
        {
            DEVICE  = 1,
            CHANNEL = 2,
            PLAYER  = 3,
            MIXER   = 4,
            LINK    = 5,
        }
        type_t;

        item(deviceId dev)
        {
            _type = DEVICE;
            _dev = dev.id();
            _obj = 0xffff;
        };

        item(deviceId dev, channelId chan)
        {
            _type = CHANNEL;
            _dev = dev.id();
            _obj = chan.id();
        };

        item(deviceId dev, playerId play)
        {
            _type = PLAYER;
            _dev = dev.id();
            _obj = play.id();
        };

        item(deviceId dev, mixerId mix)
        {
            _type = MIXER;
            _dev = dev.id();
            _obj = mix.id();
        };

        item(deviceId dev, linkId lnk)
        {
            _type = LINK;
            _dev = dev.id();
            _obj = lnk.id();
        };

        type_t    type() { return _type; };
        ushort  device() { return _dev; };
        ushort  object() { return _obj; };

     protected:
        type_t _type;
        ushort _dev;
        ushort _obj;

     private:
        item();

     friend struct kobject::hash;
     friend struct kobject::less;
     friend struct kobject::equal;
     friend struct kobject::hashLess;
    };

    struct hash
    {
        size_t operator()(const item &i) const
        {
            return (((ushort) (i._type)) << 28) + ((i._dev) << 16) + (i._obj);
        }
    };

    struct equal
    {
        bool operator()(const item &i1, const item &i2) const
        {
            return
                ((i1._type == i2._type) &&
                 (i1._dev  == i2._dev)  &&
                 (i1._obj  == i2._obj));
        }
    };

    struct less
    {
        bool comp(const item &i1, const item &i2) const
        {
            if (i1._type < i2._type)
                return true;

            if (i1._type == i2._type)
            {
                if (i1._dev  < i2._dev)
                    return true;

                if (i1._dev == i2._dev)
                {
                    if (i1._obj  < i2._obj)
                        return true;
                    else
                        return false;
                }
            }

            return false;
        }

        bool operator()(const item &i1, const item &i2) const
        {
            return comp(i1,i2);
        }
    };

    struct hashLess
    : public hash, public less
    {
         public:
            static const size_t bucket_size = 4;
            static const size_t min_buckets = 8;
    };
};

#endif /* INCLUDED_TYPES_HPP */
