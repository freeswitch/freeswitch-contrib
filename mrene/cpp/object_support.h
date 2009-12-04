#ifndef OBJECT_SUPPORT_H
#define OBJECT_SUPPORT_H

#include <switch.h>

class Mutex 
{
private:
	bool m_owned;
	switch_mutex_t *m_mutex;
	
	Mutex(const Mutex &p)
	{
	}
	
public:
	Mutex(switch_mutex_t *m) 
	{
		m_owned = false;
		m_mutex = m;
	}
	
	Mutex(switch_memory_pool_t *pool)
	{
		m_owned = true;
		switch_mutex_init(&m_mutex, SWITCH_MUTEX_NESTED, pool);
	}
	
	~Mutex()
	{
		if (m_owned) {
			switch_mutex_destroy(m_mutex);
		}
	}
	
	void lock()
	{
		switch_mutex_lock(m_mutex);
	}
	
	void unlock()
	{
		switch_mutex_unlock(m_mutex);
	}
	
	class scoped_lock 
	{
		
	private:
		Mutex &m_mutex;
		scoped_lock(const scoped_lock &p) : m_mutex(p.m_mutex) { };
		
	public:
		scoped_lock(Mutex &m) : m_mutex(m)
		{
			m_mutex.lock();
		}
		
		~scoped_lock()
		{
			m_mutex.unlock();
		}
	};
};

class RWLock
{
private:
	bool m_owned;
	switch_thread_rwlock_t *m_rwlock;
	
	RWLock(const RWLock& p)
	{
		
	}
public:
	RWLock(switch_thread_rwlock_t *rw)
	{
		m_owned = false;
		m_rwlock = rw;
	}
	RWLock(switch_memory_pool_t *pool)
	{
		m_owned = true;
		switch_thread_rwlock_create(&m_rwlock, pool);
	}
	
	void rdlock() 
	{
		switch_thread_rwlock_rdlock(m_rwlock);
	}
	
	switch_status_t tryrdlock()
	{
		return switch_thread_rwlock_tryrdlock(m_rwlock);
	}
	
	void wrlock()
	{
		switch_thread_rwlock_wrlock(m_rwlock);
	}
	
	switch_status_t trywrlock()
	{
		return switch_thread_rwlock_trywrlock(m_rwlock);
	}
	
	switch_status_t trywrlock_timeout(int timeout)
	{
		return switch_thread_rwlock_trywrlock_timeout(m_rwlock, timeout);
	}
	
	void unlock()
	{
		switch_thread_rwlock_unlock(m_rwlock);
	}
	
	class scoped_lock 
	{
	private:
		RWLock &m_rwlock;
		scoped_lock(const scoped_lock &p) : m_rwlock(p.m_rwlock) { }
	public:
		scoped_lock(RWLock &rwlock, switch_bool_t write = SWITCH_FALSE) : m_rwlock(rwlock)
		{
			if (write) {
				m_rwlock.rdlock();
			} else {
				m_rwlock.wrlock();
			}
		}
		~scoped_lock()
		{
			m_rwlock.unlock();
		}
	};
};

class MemoryPool
{
private:
	bool m_owned;
	switch_memory_pool_t *m_pool;
	MemoryPool(const MemoryPool &p) { }
public:
	MemoryPool(switch_memory_pool_t *pool)
	{
		if (pool) {
			m_owned = false;
			m_pool = pool;	
		} else {
			m_owned = true;
			switch_core_new_memory_pool(&m_pool);
		}
	}
	
	MemoryPool(switch_core_session_t *session)
	{
		if (session) {
			m_owned = false;
			m_pool = switch_core_session_get_pool(session);
		} else {
			m_owned = true;
			switch_core_new_memory_pool(&m_pool);
		}
	}
	
	MemoryPool()
	{
		switch_core_new_memory_pool(&m_pool);
		m_owned = true;
	}
	
	~MemoryPool()
	{
		if (m_owned) {
			switch_core_destroy_memory_pool(&m_pool);
		}
	}
	
	void *alloc(switch_size_t n) 
	{
		return switch_core_alloc(m_pool, n);
	}
	
	switch_memory_pool_t *get_pool() 
	{
		return m_pool;
	}
	
	char *strdup(const char *s)
	{
		return switch_core_strdup(m_pool, s);
	}
	
	char *sprintf(const char *fmt, ...)
	{
		va_list ap;
		char *result = NULL;

		va_start(ap, fmt);
		result = switch_core_vsprintf(m_pool, fmt, ap);
		va_end(ap);
		
		return result;
	}
};


#endif /* OBJECT_SUPPORT_H */