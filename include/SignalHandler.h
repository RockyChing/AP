#ifndef _LINUX_SIGNAL_HANDLER_H
#define _LINUX_SIGNAL_HANDLER_H
#include <RefBase.h>

/**
 * This helper class simplifies the handling of POSIX signals.
 * The class provides a signal handler (installed with
 * installHandlers()) that translates certain POSIX
 * signals (SIGILL, SIGBUS, SIGSEGV, SIGSYS) into 
 * C++ exceptions.
 * 

	///

	/// Internally, a stack of sigjmp_buf structs is maintained for

	/// each thread. The constructor pushes a new sigjmp_buf onto

	/// the current thread's stack. The destructor pops the sigjmp_buf

	/// from the stack.

	///

	/// The poco_throw_on_signal macro creates an instance of SignalHandler

	/// on the stack, which results in a new sigjmp_buf being created.

	/// The sigjmp_buf is then set-up with sigsetjmp().

	///

	/// The handleSignal() method, which is invoked when a signal arrives,

	/// checks if a sigjmp_buf is available for the current thread.

	/// If so, siglongjmp() is used to jump out of the signal handler.

	///

	/// Typical usage is as follows:

	///

	///     try

	///     {

	///          poco_throw_on_signal;

	///          ...

	///     }

	///     catch (Poco::SignalException&)

	///     {

	///         ...

	///     }

	///

	/// The best way to deal with a SignalException is to log as much context

	/// information as possible, to aid in debugging, and then to exit.

	///

	/// The SignalHandler can be disabled globally by compiling POCO and client

	/// code with the POCO_NO_SIGNAL_HANDLER macro defined.
 */
class SignalHandler :public RefBase {
public:
      SignalHandler();
    ~SignalHandler();

protected:
    static void handler(int signal);

private:
    SignalHandler(const SignalHandler&);
    SignalHandler& operator=(const SignalHandler&);

private:
    void onFirstRef();
};

#endif

