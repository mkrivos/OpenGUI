#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__sun__) || defined (__sun)
#include <pthread.h>
#endif

#if defined(__linux__)
#include <linux/version.h>
#include <pthread.h>
#define PTHREAD_MUTEX_NORMAL	PTHREAD_MUTEX_TIMED_NP
#define PTHREAD_MUTEX_RECURSIVE	PTHREAD_MUTEX_RECURSIVE_NP
#ifdef __BORLANDC__
#pragma link "libpthread.so"
#endif
#endif // linux

#include "fgconcurency.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/*
 * FGMutex
 */
#ifdef FG_THREADED

#ifdef __linux__
extern "C" int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);
#endif

void FGMutex::init(ENUM_MUTEX_TYPE typ)
{
#ifdef _PTHREAD_H
	m_mutex = (pthread_mutex_t *) new pthread_mutex_t;
	type = typ;
	pthread_mutexattr_t     mattrDetails;
	pthread_mutexattr_init(&mattrDetails);
	switch (type)
	{
		default:
		case fastMutEx:
			pthread_mutexattr_settype(&mattrDetails, PTHREAD_MUTEX_NORMAL);
			break;
		case recursiveMutEx:
			pthread_mutexattr_settype(&mattrDetails, PTHREAD_MUTEX_RECURSIVE);
			break;
	}
	pthread_mutex_init((pthread_mutex_t *)m_mutex, &mattrDetails);
	pthread_mutexattr_destroy(&mattrDetails);
#endif
#ifdef _WIN32
	m_mutex = (CRITICAL_SECTION *) new CRITICAL_SECTION;
	InitializeCriticalSection((CRITICAL_SECTION *)m_mutex);
#endif
}

FGMutex::FGMutex(ENUM_MUTEX_TYPE typ)
{
#ifdef _WIN32
	init((ENUM_MUTEX_TYPE)0);
#else
	init((ENUM_MUTEX_TYPE)typ);
#endif
}


FGMutex::FGMutex(const FGMutex& old)
{
#ifdef _WIN32
	init((ENUM_MUTEX_TYPE)0);
#else
	init((ENUM_MUTEX_TYPE)old.type);
#endif
}


FGMutex::~FGMutex()
{
#ifdef _PTHREAD_H
	int val = pthread_mutex_destroy((pthread_mutex_t *)m_mutex);
	if (val!=0)
	{
//		printf("mutex_destroy [%x] - error %d\n", this, val);
	}
	delete (pthread_mutex_t *) m_mutex;
#endif
#ifdef _WIN32
	LeaveCriticalSection((CRITICAL_SECTION *)m_mutex);
#endif
}

void FGMutex::Lock()
{
#ifdef _PTHREAD_H
	pthread_mutex_lock((pthread_mutex_t *)m_mutex);
#endif
#ifdef _WIN32
	EnterCriticalSection((CRITICAL_SECTION *)m_mutex);
#endif
}

void FGMutex::Unlock()
{
#ifdef _PTHREAD_H
	pthread_mutex_unlock((pthread_mutex_t *)m_mutex);
#endif
#ifdef _WIN32
	LeaveCriticalSection((CRITICAL_SECTION *)m_mutex);
#endif
}

bool FGMutex::TryLock()
{
	int retval;
#ifdef __linux__
	retval = pthread_mutex_trylock((pthread_mutex_t *)m_mutex);
	if (retval < 0 && retval != EBUSY)
		return false;
	else
#endif
#ifdef _WIN32
	return TryEnterCriticalSection((CRITICAL_SECTION *)m_mutex);
#endif
	return true;
}

#endif

#ifdef FG_NAMESPACE
}
#endif
