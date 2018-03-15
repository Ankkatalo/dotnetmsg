/*
Nng wrapper

Class Msg




*/

#include "NngExternal.h"
#include "nng.h"
#include "NngInternal.h"
#include <cstring>
#include <cstdint>
#include "nng.h"

namespace Nng {

	Msg::Msg(size_t size)
	{
		nng_msg* newMsg;
		int result = ::nng_msg_alloc(&newMsg, size);
		this->msg = System::UIntPtr(newMsg);
		if (result != 0) throw gcnew NngException(Errno::nomem);
	}

	Msg::Msg(System::UIntPtr ptr)
	{
		this->msg = ptr;
	}

	Msg::~Msg()
	{
		if (this->msg != System::UIntPtr::Zero) {
			::nng_msg_free(getNativeMsg(this));
		}
	}

	Errno Msg::Alloc([Out] Msg^% msg, size_t size)
	{
		nng_msg* newMsg;
		int result = ::nng_msg_alloc(&newMsg, size);
		if (result == 0) {
			msg = gcnew Msg(System::UIntPtr(newMsg));
		}
		else {
			msg = nullptr;
		}
		return static_cast<Errno>(result);
	}

	void Msg::Free()
	{
		::nng_msg_free(getNativeMsg(this));
		this->msg = System::UIntPtr::Zero;
	}

	Errno  Msg::Realloc(size_t size)
	{
		return static_cast<Errno>(::nng_msg_realloc(getNativeMsg(this), size));
	}

	Errno  Msg::Dup([Out] Msg^% msgOut)
	{
		nng_msg* msgPtr = getNativeMsg(this);
		nng_msg* newMsg;
		int result = ::nng_msg_dup(&newMsg, msgPtr);
		if (result == 0) {
			msgOut = gcnew Msg(System::UIntPtr(newMsg));
		}
		else {
			msgOut = nullptr;
		}
		return static_cast<Errno>(result);
	}


	// Concerning the body


	array<System::Byte>^ Msg::Body() {
		nng_msg* msgPtr = getNativeMsg(this);
		void* data = ::nng_msg_body(msgPtr);
		size_t size = ::nng_msg_len(msgPtr);
		if (size > INT32_MAX) throw gcnew OutOfMemoryException();
		auto retVal = gcnew array<System::Byte>(static_cast<int>(size));
		pin_ptr<System::Byte> pin = &retVal[0]; // this might or might not be needed, too lazy to research
		memcpy(pin, data, size);
		return retVal;
	}

	Errno Msg::Append(array<System::Byte>^ data)
	{
		pin_ptr<System::Byte> pin = &data[0]; // this is certainly needed
		return static_cast<Errno>(::nng_msg_append(getNativeMsg(this), pin, data->Length));
	}

	Errno Msg::Insert(array<System::Byte>^ data)
	{
		pin_ptr<System::Byte> pin = &data[0]; // this is certainly needed
		return static_cast<Errno>(::nng_msg_insert(getNativeMsg(this), pin, data->Length));
	}

	Errno  Msg::AppendU32(UInt32 value)
	{
		return static_cast<Errno>(::nng_msg_append_u32(getNativeMsg(this), value));
	}

	Errno  Msg::InsertU32(UInt32 value)
	{
		return static_cast<Errno>(::nng_msg_insert_u32(getNativeMsg(this), value));
	}

	Errno  Msg::TrimU32([Out] UInt32% value)
	{
		UInt32 tempValue;
		int retVal = ::nng_msg_trim_u32(getNativeMsg(this), &tempValue);
		value = tempValue;
		return static_cast<Errno>(retVal);
	}

	Errno  Msg::ChopU32([Out] UInt32% value)
	{
		UInt32 tempValue;
		int retVal = ::nng_msg_chop_u32(getNativeMsg(this), &tempValue);
		value = tempValue;
		return static_cast<Errno>(retVal);
	}

	Errno  Msg::Trim(size_t size)
	{
		return static_cast<Errno>(::nng_msg_trim(getNativeMsg(this), size));
	}

	Errno  Msg::Chop(size_t size)
	{
		return static_cast<Errno>(::nng_msg_chop(getNativeMsg(this), size));
	}

	void Msg::Clear()
	{
		return ::nng_msg_clear(getNativeMsg(this));
	}

	// concerning the header


	array<System::Byte>^ Msg::Header()
	{
		nng_msg* msgPtr = getNativeMsg(this);
		void* data = ::nng_msg_header(msgPtr);
		size_t size = ::nng_msg_header_len(msgPtr);
		if (size > INT32_MAX) throw gcnew OutOfMemoryException();
		auto retVal = gcnew array<System::Byte>(static_cast<int>(size));
		pin_ptr<System::Byte> pin = &retVal[0]; // this might or might not be needed, too lazy to research
		memcpy(pin, data, size);
		return retVal;
	}

	Errno Msg::HeaderAppend(array<System::Byte>^ data)
	{
		pin_ptr<System::Byte> pin = &data[0]; // this is certainly needed
		return static_cast<Errno>(::nng_msg_header_append(getNativeMsg(this), pin, data->Length));
	}

	Errno Msg::HeaderInsert(array<System::Byte>^ data)
	{
		pin_ptr<System::Byte> pin = &data[0]; // this is certainly needed
		return static_cast<Errno>(::nng_msg_header_insert(getNativeMsg(this), pin, data->Length));
	}

	Errno  Msg::HeaderAppendU32(UInt32 value)
	{
		return static_cast<Errno>(::nng_msg_header_append_u32(getNativeMsg(this), value));
	}

	Errno  Msg::HeaderInsertU32(UInt32 value)
	{
		return static_cast<Errno>(::nng_msg_header_insert_u32(getNativeMsg(this), value));
	}

	Errno  Msg::HeaderTrimU32([Out] UInt32% value)
	{
		UInt32 tempValue;
		int retVal = ::nng_msg_header_trim_u32(getNativeMsg(this), &tempValue);
		value = tempValue;
		return static_cast<Errno>(retVal);
	}

	Errno  Msg::HeaderChopU32([Out] UInt32% value)
	{
		UInt32 tempValue;
		int retVal = ::nng_msg_header_chop_u32(getNativeMsg(this), &tempValue);
		value = tempValue;
		return static_cast<Errno>(retVal);
	}

	Errno  Msg::HeaderTrim(size_t size)
	{
		return static_cast<Errno>(::nng_msg_header_trim(getNativeMsg(this), size));
	}

	Errno  Msg::HeaderChop(size_t size)
	{
		return static_cast<Errno>(::nng_msg_header_chop(getNativeMsg(this), size));
	}

	void Msg::HeaderClear()
	{
		return ::nng_msg_header_clear(getNativeMsg(this));
	}

	// misc

	int Msg::GetPipe()
	{
		return ::nng_msg_get_pipe(getNativeMsg(this));
	}

	void Msg::SetPipe(int pipe)
	{
		return ::nng_msg_set_pipe(getNativeMsg(this), pipe);
	}


}