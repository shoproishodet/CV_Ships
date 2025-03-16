#include <process.h>
//--------------------------------------------------------------------------------------------------
struct ThreadFunc{
    virtual ~ThreadFunc() {}
    virtual void run() = 0;
};

template <typename T> struct ThreadFunctor : ThreadFunc{
    T m_functor;
    ThreadFunctor(T functor) : m_functor(functor) {}
    virtual void run() {m_functor();}
};

template <typename F, typename A> struct ThreadFunctorWithArg : ThreadFunc{
 F m_function;
 A m_arg;
 ThreadFunctorWithArg(F function, A arg) : m_function(function), m_arg(arg) {}
 virtual void run() {m_function(m_arg);}
};

template <typename C>struct ThreadMemberFunc : ThreadFunc{
    C* m_object;
    ThreadMemberFunc(void(C::*function)(), C* object) : m_function(function), m_object(object) {}
    virtual void run() {(m_object->*m_function)();}
    void(C::*m_function)();
};
//--------------------------------------------------------------------------------------------------
struct Thread;

struct ThreadImpl{
 HANDLE m_thread;
 unsigned int m_threadId;

 ThreadImpl(Thread* owner);
 ~ThreadImpl();
 void wait();
 void terminate();
 void end();
 static unsigned int __stdcall entryPoint(void* userData);
};
//--------------------------------------------------------------------------------------------------
struct Thread{friend struct ThreadImpl;
 ThreadImpl *m_impl;     
 ThreadFunc *m_entryPoint;
 bool EndTask;

 void run(){m_entryPoint->run();}
 template <typename F> Thread(F functor) :m_impl(NULL),m_entryPoint(new ThreadFunctor<F>(functor)){}
 template <typename F, typename A> Thread(F function, A argument) :m_impl(NULL),m_entryPoint(new ThreadFunctorWithArg<F, A>(function, argument)){}
 template <typename C> Thread(void(C::*function)(), C* object) :m_impl(NULL),m_entryPoint(new ThreadMemberFunc<C>(function, object)){}
 ~Thread(){wait();delete m_entryPoint;}
void launch(){EndTask=false;
    wait();
    m_impl = new ThreadImpl(this);
}
void wait(){
    if (m_impl){
        m_impl->wait();
        delete m_impl;
        m_impl = NULL;
    }
}
void end(){CON.write(0,"Thread::end");EndTask=true;};
void terminate(){
    if (m_impl){
        m_impl->terminate();
        delete m_impl;
        m_impl = NULL;
    }
}
};
//--------------------------------------------------------------------------------------------------
ThreadImpl::ThreadImpl(Thread *owner){CON.write(0,"Thread::launch");
    m_thread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &ThreadImpl::entryPoint, owner, 0, &m_threadId));
}
ThreadImpl::~ThreadImpl(){if(m_thread)CloseHandle(m_thread);}
void ThreadImpl::wait(){CON.write(0,"Thread::wait");
    if(m_thread && m_threadId!=GetCurrentThreadId()){
    }
}
void ThreadImpl::terminate(){CON.write(0,"Thread::terminate");if(m_thread)TerminateThread(m_thread, 0);}
unsigned int __stdcall ThreadImpl::entryPoint(void* userData){
    Thread *owner = static_cast<Thread*>(userData);// The Thread instance is stored in the user data
    owner->run();// Forward to the owner
    _endthreadex(0);// Optional, but it is cleaner
    return 0;
}
//--------------------------------------------------------------------------------------------------
struct MutexImpl{
    CRITICAL_SECTION m_mutex; ///< Win32 handle of the mutex
    MutexImpl(){InitializeCriticalSection(&m_mutex);}
    ~MutexImpl(){DeleteCriticalSection(&m_mutex);};
    void lock(){EnterCriticalSection(&m_mutex);};
    void unlock(){LeaveCriticalSection(&m_mutex);};
};

struct Mutex{
    MutexImpl *m_mutexImpl; ///< OS-specific implementation
    Mutex(){m_mutexImpl = new MutexImpl;};
    ~Mutex(){delete m_mutexImpl;};
    void lock(){m_mutexImpl->lock();};
    void unlock(){m_mutexImpl->unlock();};
};
