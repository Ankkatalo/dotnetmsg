/*
Nng wrapper

Sending and receiving data




*/

#include "NngExternal.h"
#include "nng.h"
#include "NngInternal.h"
#include <cstring>

// provide definitions for struct, just to make the linker shut up
extern "C" struct nng_aio {};
extern "C" struct nng_msg {};

namespace Nng {

	Errno Socket::Send(array<Byte>^ data, [Optional] Nullable<Flag> flags)
	{
		pin_ptr<Byte> pin = &data[0];
		
		int flags2 = (int) Flag::nonblock; 
		if (flags.HasValue) flags2 = (int)(Flag)flags;
		int result = ::nng_send(this->NngSocket, (void*)pin, data->LongLength, flags2);
		return static_cast<Errno>(result);
	}

	Errno Socket::Receive([Out] array<Byte>^% data, [Optional] Nullable<Flag> flags)
	{
		int flags2 = 0;
		if (flags.HasValue) flags2 = (int)(Flag)flags;
		flags2 |= NNG_FLAG_ALLOC;

		void* buf;
		size_t size;
		int result = ::nng_recv(this->NngSocket, &buf, &size, flags2);
		if (result == 0) {
			if (size > INT32_MAX) throw gcnew OutOfMemoryException();
			data = gcnew array<Byte>(static_cast<int>(size));
			pin_ptr<Byte> pin = &data[0];
			memcpy(pin, buf, size);
			nng_free(buf, size);
		}
		else {
			data = nullptr;
		}
		return static_cast<Errno>(result);
	}

	Errno Socket::Send(Msg^ msg, [Optional] Nullable<Flag> flags)
	{
		nng_msg* msgPtr = reinterpret_cast<nng_msg*>(msg->msg.ToPointer());
		return static_cast<Errno>(::nng_sendmsg(this->NngSocket, msgPtr, (flags.HasValue ? (int)(Flag)flags : NNG_FLAG_NONBLOCK)));
	}

	Errno Socket::Receive([Out] Msg^% msg, [Optional] Nullable<Flag> flags)
	{
		nng_msg* newMsg;
		int result = ::nng_recvmsg(this->NngSocket, &newMsg, (flags.HasValue ? (int)(Flag)flags : 0));
		if (result == 0) {
			msg = gcnew Msg(System::UIntPtr(newMsg));
		}
		else {
			msg = nullptr;
		}
		return static_cast<Errno>(result);
	}

	void Socket::Send(Aio^ aio)
	{
		::nng_send_aio(this->NngSocket, getNativeAio(aio));
	}

	void Socket::Receive(Aio^ aio)
	{
		return ::nng_recv_aio(this->NngSocket, getNativeAio(aio));
	}

	Errno Aio::Result()
	{
		return static_cast<Errno>(::nng_aio_result(getNativeAio(this)));
	}

}