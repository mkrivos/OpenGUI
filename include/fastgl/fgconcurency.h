#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

/**
	The types of mutexes.
	@ingroup Enums
*/
enum ENUM_MUTEX_TYPE
{
	fastMutEx=1,
	recursiveMutEx
};

/**
	Synchronisation object - MUTEX. Used when multithrading
	support is enabled.
*/
class FGMutex
{
//		pthread_mutex_t m_mutex;
//		CRITICAL_SECTION m_mutex;
		int				type;
		void*			m_mutex;
#ifdef FG_THREADED
		void init(ENUM_MUTEX_TYPE typ);
#endif
	public:
		//! Create a MUTEX. You can choose from fast or recursive one.
		FGMutex(ENUM_MUTEX_TYPE typ = fastMutEx)
#ifdef FG_THREADED
		;
#else
		{}
#endif

		FGMutex(const FGMutex& old)
#ifdef FG_THREADED
		;
#else
		{}
#endif

		~FGMutex()
#ifdef FG_THREADED
		;
#else
		{}
#endif
		//! Lock the object for others.
		void Lock()
#ifdef FG_THREADED
		;
#else
		{}
#endif
		//! Unlock the object for others.
		void Unlock()
#ifdef FG_THREADED
		;
#else
		{}
#endif
		//! Try lock the object for others. Returns false if one is already locked.
		bool TryLock()
#ifdef FG_THREADED
		;
#else
		{ return true; }
#endif
};

#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

