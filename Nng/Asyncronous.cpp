/*
Nng wrapper

aio




*/

#include "NngExternal.h"
#include "nng.h"
#include "NngInternal.h"
#include <cstring>
#include <vcclr.h>
#include "protocol/reqrep0/req.h"
#include "protocol/reqrep0/rep.h"

#undef ENOMEM

namespace Nng {

	/*
	The aio wrapper is not entirely trivial as it has to bridge between the gc-heap and the native C++ heap.
	a gcroot<> wrapper is needed to point to the gc-heap from native memory. Allocating the wrapper increases
	the reference count of the object on the gc-heap and destroying the wrapper decreases it.

	However, there is the added complication of Application Domains. If the caller is in a different application domain
	than the Nng Assembly, the gcroot cannot be used in the Application Domain of the Assembly.

	For this reason a delegate called Aio::callbackDelegate is created, because a delegate can jump into
	the correct AppDomain, more specifically into the one from which it was created from.

	Shoutout to: http://www.lenholgate.com/blog/2009/07/error-cannot-pass-a-gchandle-across-appdomains.html

	*/

	
	// this is our help object on the native heap
	struct aio_help_object {
		gcroot<Aio^> managedAio; // do not access this while in a callback and still on the native side!
		nng_aio* unmanagedAio;
		void (__stdcall *callback)(void); // The wrapper is __stdcall
	};

	// forward the callback, from native to managed, call will end up in Aio::CallbackEntry
	static void __cdecl aioCallbackFunction(void* context) {
		aio_help_object* aioHelper = static_cast<aio_help_object*>(context);
		aioHelper->callback();
	}

	void Aio::CallbackEntry(void)
	{
		// We are in managed code, in the correct AppDomain!
		this->dotNetCallback(this->dotNetCallbackContext);
	}

	extern nng_aio* getNativeAio(Aio^ aio) {
		aio_help_object* helpPtr = reinterpret_cast<aio_help_object*>(aio->aio.ToPointer());
		return helpPtr->unmanagedAio;
	}
	extern nng_msg* getNativeMsg(Msg^ msg) {
		return reinterpret_cast<nng_msg*>(msg->msg.ToPointer());
	}

	// this constructor doesn't initialize anything, only to be used internally
	Aio::Aio()
	{
	}

	Errno Aio::Initialize(Aio^ aio, System::Action<System::Object^>^ callback, System::Object^ object)
	{
		auto aioHelper = new aio_help_object();
		if (aio == nullptr || aioHelper == nullptr) {
			delete aioHelper;
			return Errno::nomem;
		}

		// We are still in the application domain of the caller. Setting something here harmless
		aio->dotNetCallback = callback;
		aio->dotNetCallbackContext = object;
		aio->aio = System::UIntPtr(aioHelper);

		// create a delegate on the managed heap, it must have a long lifetime
		aio->callbackDelegate = gcnew CallbackEntryDelegate(aio, &CallbackEntry);

		nng_aio * newAio;
		int result = ::nng_aio_alloc(&newAio, aioCallbackFunction, aioHelper);
		if (result == 0) {
			aioHelper->unmanagedAio = newAio;
			aioHelper->managedAio = aio; // this is just to prevent gc. We don't access it from unmanaged code

			// And now for the magic trick. Retrieve a C++ function pointer which will do the actual
			// unmanaged -> managed jump, including the AppDomain change
			aioHelper->callback = (void(*)(void)) Marshal::GetFunctionPointerForDelegate(aio->callbackDelegate).ToPointer();
		}
		else {
			aio = nullptr;
			delete aioHelper;
		}
		return static_cast<Errno>(result);

	}

	Aio::Aio(System::Action<System::Object^>^ callback, System::Object^ object)
	{
		Errno err = Initialize(this, callback, object);
		if (err != Errno::ok) { throw gcnew NngException(err); }
	}

	Errno Aio::Alloc([Out] Aio^% aio, System::Action<System::Object^>^ callback, System::Object^ object)
	{
		aio = gcnew Aio();
		Errno err = Initialize(aio, callback, object);
		return err;
	}

	void Aio::Free()
	{
		if (this->aio != UIntPtr::Zero) { // UIntPtr is a value class
			aio_help_object* helpPtr = reinterpret_cast<aio_help_object*>(this->aio.ToPointer());
			::nng_aio_free(helpPtr->unmanagedAio);
			delete helpPtr; // this also decreases the ref count on the managed heap, as the gcroot is destroyed
			this->callbackDelegate = nullptr; // I don't think this actually does anything useful
			this->aio = UIntPtr::Zero; // mark the aio as unused
		}
	}

	Msg^ Aio::GetMsg()
	{
		aio_help_object* helpPtr = reinterpret_cast<aio_help_object*>(this->aio.ToPointer());
		nng_msg* newMsg = ::nng_aio_get_msg(helpPtr->unmanagedAio);
		auto retVal = gcnew Msg(System::UIntPtr(newMsg));
		return retVal;
	}

	void Aio::Stop()
	{
		::nng_aio_stop(getNativeAio(this));
	}
	void Aio::Cancel()
	{
		::nng_aio_cancel(getNativeAio(this));
	}
	void Aio::Wait()
	{
		::nng_aio_wait(getNativeAio(this));
	}
	void Aio::SetMsg(Msg^ msg)
	{
		::nng_aio_set_msg(getNativeAio(this), getNativeMsg(msg));
	}
	void Aio::SetTimeout(Int32 duration)
	{
		::nng_aio_set_timeout(getNativeAio(this), duration);
	}
	Aio::~Aio()
	{
		if (this->aio != UIntPtr::Zero) {
			Free();
		}
	}
}

