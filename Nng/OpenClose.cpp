/*
  Nng wrapper

  Functions for classes Socket, Dialer, Listener and Protocols,
  for opening and closing sockets.




*/

#include "NngExternal.h"
#include "nng.h"
#include "NngInternal.h"
#include <cstring>
#include "protocol/reqrep0/req.h"
#include "protocol/reqrep0/rep.h"
#include "protocol/bus0/bus.h"
#include "protocol/pair0/pair.h"
#include "protocol/pipeline0/pull.h"
#include "protocol/pipeline0/push.h"
#include "protocol/pubsub0/pub.h"
#include "protocol/pubsub0/sub.h"
#include "protocol/survey0/respond.h"
#include "protocol/survey0/survey.h"



namespace Nng {

	Socket::Socket()
	{
		this->NngSocket = 0;
		this->IsClosed = false;
	}

	void Socket::Fini(void) {
		return ::nng_fini();
	}

	Errno Socket::Close() {
		IsClosed = true;
		return static_cast<Errno>(::nng_close(this->NngSocket));
	}

   // Note that CloseAll is a function only to be used in very rare circumstances
	void Socket::CloseAll(void)
	{
		return ::nng_closeall();
	}

   // The destructor will not be directly used, as .NET languages doen't have destructors, but it will be automatically converted to
   // the IDisposable interface
	Socket::~Socket()
	{
		if (!IsClosed) {
			Close();
		}
	}

// deal with UTF-16. Declare a "const char *a" pointing to string bafter a conversion to UTF-8
#define DECLARE_CONST_STRING(a,b) \
	array<System::Byte>^ a##bytes; \
  {\
		long len = 4*b->Length; /*System::Text::Encoding::UTF8->GetByteCount(b); Faster to allocate just the maximum, 4 bytes per char */ \
	  a##bytes = gcnew array<System::Byte>(len + 1); \
		len = System::Text::Encoding::UTF8->GetBytes(b, 0, b->Length, a##bytes, 0); \
		a##bytes[len] = 0; \
	}\
	pin_ptr<System::Byte> a##pin = &a##bytes[0]; \
  const char* a = (const char*) a##pin;

	Errno Socket::SetOpt(System::String^ str, array<System::Byte>^ a)
	{
		DECLARE_CONST_STRING(ip1, str);
		pin_ptr<System::Byte> ip2 = &a[0];
		return static_cast<Errno>(::nng_setopt(this->NngSocket, (const char*)ip1, (void*)ip2, a->Length));
	}

	Errno Socket::SetOptInt(System::String^ str, int i) {
		DECLARE_CONST_STRING(s, str);
		return static_cast<Errno>(::nng_setopt_int(this->NngSocket, s, i));
	}

	Errno Socket::SetOptMs(System::String^ str, Int32 i) {
		DECLARE_CONST_STRING(s, str);
		return static_cast<Errno>(::nng_setopt_ms(this->NngSocket, s, i));
	}

	Errno Socket::SetOptSize(System::String^ str, size_t i) {
		DECLARE_CONST_STRING(s, str);
		return static_cast<Errno>(::nng_setopt_size(this->NngSocket, s, i));
	}
	Errno Socket::SetOptUInt64(System::String^ str, UInt64 i) {
		DECLARE_CONST_STRING(s, str);
		return static_cast<Errno>(::nng_setopt_uint64(this->NngSocket, s, i));
	}

	Errno Socket::SetOptString(System::String^ str, System::String^ str2) {
		DECLARE_CONST_STRING(s, str);
		DECLARE_CONST_STRING(s2, str2);
		return static_cast<Errno>(::nng_setopt_string(this->NngSocket, s, s2));
	}

	Errno Socket::GetOpt(System::String^ str, [Out] array<System::Byte>^% b) {
		DECLARE_CONST_STRING(s, str);
		return Errno::ok;
	}

	Errno Socket::GetOptInt(System::String^ str, [Out] Int32% i) {
		DECLARE_CONST_STRING(s, str);
		Int32 i2;
		int result = ::nng_getopt_int(this->NngSocket, s, &i2);
		i = i2;
		return static_cast<Errno>(result);
	}

	Errno Socket::GetOptMs(System::String^ str, [Out] Int32% i) {
		DECLARE_CONST_STRING(s, str);
		Int32 i2;
		int result = ::nng_getopt_ms(this->NngSocket, s, &i2);
		i = i2;
		return static_cast<Errno>(result);
	}

	Errno Socket::GetOptSize(System::String^ str, [Out] UInt64% i) {
		DECLARE_CONST_STRING(s, str);
		UInt64 i2;
		int result = ::nng_getopt_size(this->NngSocket, s, &i2);
		i = i2;
		return static_cast<Errno>(result);

	}
	Errno Socket::GetOptUInt64(System::String^ str, [Out] UInt64% i) {
		DECLARE_CONST_STRING(s, str);
		UInt64 i2;
		int result = ::nng_getopt_uint64(this->NngSocket, s, &i2);
		i = i2;
		return static_cast<Errno>(result);
	}

	Listener::Listener(Socket^ sock, System::String^ str, [Optional] Nullable<Flag> flags)
	{
		DECLARE_CONST_STRING(s, str);
		::nng_listener l;
		int result = ::nng_listen(sock->NngSocket, s, &l, (flags.HasValue ? (int)(Flag)flags : 0));
		if (result != 0) throw gcnew NngException(static_cast<Errno>(result));
		this->NngListener = l;
	}

	Errno Listener::Listen(Socket^ sock, System::String^ str, [Out] Listener^% listener, [Optional] Nullable<Flag> flags)
	{
		DECLARE_CONST_STRING(s, str);
		::nng_listener l;
		int result = ::nng_listen(sock->NngSocket, s, &l, (flags.HasValue ? (int)(Flag)flags : 0));
		listener = gcnew Listener();
		listener->NngListener = l;
		return static_cast<Errno>(result);
	}

	Dialer::Dialer(Socket^ sock, System::String^ str, [Optional] Nullable<int> flags)
	{
		DECLARE_CONST_STRING(s, str);
		::nng_dialer d;
		int result = ::nng_dial(sock->NngSocket, s, &d, (flags.HasValue ? (int)flags : 0));
		if (result != 0) throw gcnew NngException(static_cast<Errno>(result));
		this->NngDialer = d;
	}
	Errno Dialer::Dial(Socket^ sock, System::String^ str, [Out] Dialer^% dialer, [Optional] Nullable<Flag> flags)
	{
		DECLARE_CONST_STRING(s, str);
		::nng_dialer d;
		int result = ::nng_dial(sock->NngSocket, s, &d, (flags.HasValue ? (int)(Flag)flags : 0));
		dialer = gcnew Dialer();
		dialer->NngDialer = d;
		return static_cast<Errno>(result);
	}

	Errno Dialer::Close()
	{
		IsClosed = true;
		return static_cast<Errno>(::nng_dialer_close(this->NngDialer));
	}

	Dialer::Dialer()
	{
		IsClosed = false;
	}

	Dialer::~Dialer()
	{
		if (!IsClosed) {
			Close();
		}
	}

	Listener::Listener()
	{
		IsClosed = false;
	}

	Errno Listener::Close()
	{
		IsClosed = true;
		return static_cast<Errno>(::nng_listener_close(this->NngListener));
	}

	Listener::~Listener()
	{
		if (!IsClosed) {
			Close();
		}
	}

	Errno Dialer::Create([Out] Dialer^% dialer, Socket^ socket, System::String^ url)
	{
		DECLARE_CONST_STRING(s, url);
		::nng_dialer d;
		int result = ::nng_dialer_create(&d, socket->NngSocket, s);
		dialer = gcnew Dialer();
		dialer->NngDialer = d;
		return static_cast<Errno>(result);
	}

	Errno Listener::Create([Out] Listener^% listener, Socket^ socket, System::String^ url) {
		DECLARE_CONST_STRING(s, url);
		::nng_listener l;
		int result = ::nng_listener_create(&l, socket->NngSocket, s);
		listener = gcnew Listener();
		listener->NngListener = l;
		return static_cast<Errno>(result);
	}

	Errno Dialer::Start([Optional] Nullable<Flag> flags)
	{
		return static_cast<Errno>(::nng_dialer_start(this->NngDialer, (flags.HasValue ? (int)(Flag)flags : 0)));
	}

	Errno Listener::Start([Optional] Nullable<Flag> flags)
	{
		return static_cast<Errno>(::nng_listener_start(this->NngListener, (flags.HasValue ? (int)(Flag)flags : 0)));
	}


	static int nng_any_open([Out] Socket^% socket, int(*func)(::nng_socket*))
	{
		::nng_socket sock;
		socket = nullptr;
		int result = func(&sock);
		if (result == 0) {
			socket = gcnew Socket();
			socket->NngSocket = sock;
		}
		return result;
	}

	Errno Protocols::Rep0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_rep0_open));
	}
	Errno Protocols::Req0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_req0_open));
	}
	Errno Protocols::Bus0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_bus0_open));
	}
	Errno Protocols::Pair0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_pair0_open));
	}
	Errno Protocols::Pull0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_pull0_open));
	}
	Errno Protocols::Push0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_push0_open));
	}
	Errno Protocols::Pub0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_pub0_open));
	}
	Errno Protocols::Sub0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_sub0_open));
	}
	Errno Protocols::Respondent0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_respondent0_open));
	}
	Errno Protocols::Surveyor0([Out] Socket^% socket)
	{
		return static_cast<Errno>(nng_any_open(socket, ::nng_surveyor0_open));
	}

}