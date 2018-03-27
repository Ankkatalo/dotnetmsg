// NngExternal.h

// While this is the external public header file of the Nng wrapper, it will not actually be used, as the
// .NET consumers of the assembly will use the metadata from the assembly.

// Most functions are not throwing exceptions but return an error code. Only some constructors
// throw them, but there are alternative functions available

// Objects must usually be closed or disposed of to avoid the loss of unmanaged handles and memory.
// The .net IDispose merely calls the Close() or Free() functions, it doesn't matter which way you 
// close sockets. There are no Finalizers.

#pragma once

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Nng {

	ref class Msg;
	ref class Aio;
	ref class Protocols;
	enum class Errno : int;

	/// <summary>Flags for send and receive operations</summary>
	[Flags]
	public enum class Flag : int {
		none = 0,
		alloc = 0x00000001,
		nonblock = 0x00000002
	};

	/// <summary>
	/// NngException is thrown only from Constructors, because they cannot
	/// return error codes. Note that the use of constructors is optional, 
	/// factory functions with error codes are available
  /// </summary>
	public ref class NngException : System::Exception
	{
	public:
		Errno errno;
		NngException(Errno errno) { this->errno = errno; }
	};

	/// <summary>
	/// Socket, the central class of Nng. To construct a socket, use
	/// a member of the Protocols <see cref="Protocols">class</see>
	/// </summary>
	public ref class Socket : IDisposable {
	private:
		property bool IsClosed;
	internal:
		Socket();
		property UInt32 NngSocket;
	public:
		/// <summary>Close the Socket. Same as Dispose</summary>
		Errno Close();
		/// <summary>Close all Sockets. Should not be used.</summary>
		static void CloseAll(void);
		/// <summary>Nng Shutdown for memory leak tests. Should not be used.</summary>
		static void Fini(void);

		/// <summary>Set an option to a binary/blob value</summary>
		Errno SetOpt(System::String^ optionName, array<System::Byte>^);
      /// <summary>Set an option to a bool value</summary>
      Errno SetOptBool(System::String^ optionName, Boolean);
      /// <summary>Set an option to an integer value</summary>
		Errno SetOptInt(System::String^ optionName, int);
		/// <summary>Set an option to time value, in milliseconds</summary>
		Errno SetOptMs(System::String^ optionName, Int32);
		/// <summary>Set an option to a size value</summary>
		Errno SetOptSize(System::String^ optionName, size_t);
		/// <summary>Set an option to a uint64 value</summary>
		Errno SetOptUInt64(System::String^ optionName, UInt64);
		/// <summary>Set an option to a string value</summary>
		Errno SetOptString(System::String^ optionName, System::String^);
		
		/// <summary>Get an option as a binary/blob value.</summary>
		Errno GetOpt(System::String^ optionName, [Out] array<System::Byte>^%);
      /// <summary>Get an option as an bool value.</summary>
      Errno GetOptBool(System::String^ optionName, [Out] Boolean%);
      /// <summary>Get an option as an integer value.</summary>
		Errno GetOptInt(System::String^ optionName, [Out] Int32%);
		/// <summary>Get an option as a time value, in milliseconds.</summary>
		Errno GetOptMs(System::String^ optionName, [Out] Int32%);
		/// <summary>Get an option as a size value.</summary>
		Errno GetOptSize(System::String^ optionName, [Out] UInt64%);
		/// <summary>Get an option as a uint64 value.</summary>
		Errno GetOptUInt64(System::String^ optionName, [Out] UInt64%);
		
		/// <summary>send some data</summary><param name="flags">defaults to nonblock</param>
		Errno  Send(array<Byte>^ data, [Optional] Nullable<Flag> flags);
		/// <summary>receive data, blocking</summary><param name="flags">defaults to 0</param>
		Errno  Receive([Out] array<Byte>^% data, [Optional] Nullable<Flag> flags);
		/// <summary>send some data</summary><param name="msg">message to be sent</param><param name="flags">defaults to nonblock</param>
		Errno  Send(Msg^ msg, [Optional] Nullable<Flag> flags);
		/// <summary>receive data, blocking</summary><param name="flags">defaults to 0</param>
		Errno  Receive([Out] Msg^% msg, [Optional] Nullable<Flag> flags);
		/// <summary>send some data, returns immediately, result by callback</summary>
		void   Send(Aio^ aio);
		/// <summary>receive some data, returns immediately, result by callback</summary>
		void   Receive(Aio^ aio);

		// This will be converted to IDispose
		~Socket();

	};

	/// <summary>
	/// Listener in the BSD socket sense, it is passive and waits for incoming connections
	/// </summary>
	public ref class Listener : IDisposable {
	private:
		Listener();
		property bool IsClosed;
	internal:
		property UInt32 NngListener;
	public:
		/// <summary>
		/// Create and start a listener
		/// </summary>
		/// <param name = "flags">defaults to 0</param>
		/// <exception cref="NngException">Throws exceptions, a non-throwing variant is avalable <see cref="Listen"/></exception>
		Listener(Socket^ socket, System::String^ url, [Optional] Nullable<Flag> flags); // throws exceptions
		/// <summary>
		/// Create and start a listener
		/// </summary>
		/// <param name="flags">defaults to 0</param>
		/// <returns>Errno::ok on success</returns>
		static Errno Listen(Socket^ socket, System::String^ url, [Out] Listener^% listener, [Optional] Nullable<Flag> flags);
		/// <summary>
		/// Create a listener
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		static Errno Create([Out] Listener^% listener, Socket^ socket, [Optional] System::String^ url);
		/// <summary>
		/// Start a listener
		/// </summary>
		/// <param name="flags">defaults to 0</param>
		/// <returns>Errno::ok on success</returns>
		Errno Start([Optional] Nullable<Flag> flags);
		/// <summary>
		/// Stops and Closes a listener. Same as Dispose
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Close();
		~Listener();
	};

	/// <summary>
	/// Dialer in the BSD socket sense, it is active and connects to a listener
	/// </summary>
	public ref class Dialer : IDisposable {
	private:
		Dialer();
		property UInt32 NngDialer;
		property bool IsClosed;
	internal:
	public:
		/// <summary>
		/// Create and start a Dialer
		/// </summary>
		/// <param name="flags">defaults to 0</param>
		/// <exception cref="NngException">Throws exceptions, a non-throwing variant is avalable <see cref="Dial"/></exception>
		Dialer(Socket^ socket, System::String^ url, [Optional] Nullable<int> flags); // throws exceptions
		/// <summary>
		/// Create and start a Dialer
		/// </summary>
		/// <param name="flags">defaults to 0</param>
		/// <returns>Errno::ok on success</returns>
		static Errno Dial(Socket^ socket, System::String^ url, [Out] Dialer^% dialer, [Optional] Nullable<Flag> flags);
		/// <summary>
		/// Create a Dialer
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		static Errno Create([Out] Dialer^% dialer, Socket^ socket, System::String^ url);
		/// <summary>
		/// Start a Dialer
		/// </summary>
		/// <param name="flags">defaults to 0</param>
		/// <returns>Errno::ok on success</returns>
		Errno Start([Optional] Nullable<Flag> flags);
		Errno Close();
		~Dialer();
	};

	/// <summary>
	/// The message
	/// </summary>
	public ref class Msg : IDisposable {
	internal:
		property UIntPtr msg;
		Msg(System::UIntPtr ptr);
	public:
		/// <summary>
		/// Allocates a message already with space for data
		/// </summary>
		/// <exception cref="NngException">Throws Errno::nonmem</exception>
		Msg(size_t size);
		/// <summary>
		/// Allocates a empty message
		/// </summary>
		/// <exception cref="NngException">Throws Errno::nonmem</exception>
		Msg() : Msg(0) {};
		/// <summary>
		/// Allocates a message already with space for data
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Alloc([Out] Msg^% msg, size_t size);
		/// <summary>
		/// Frees a message. Same as Dispose
		/// </summary>
		void  Free();
		/// <summary>
		/// Reallocates a message already with space for data
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Realloc(size_t size);
		/// <summary>
		/// Duplicates a message (deep copy)
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Dup([Out] Msg^% msgOut);

		/// <summary>
		/// Returns the entire body of the message
		/// </summary>
		array<System::Byte>^ Body();
		/// <summary>
		/// Append data to a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Append(array<System::Byte>^ data);
		/// <summary>
		/// Insert data at the beginning of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Insert(array<System::Byte>^ data);
		/// <summary>
		/// Append data to a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno AppendU32(UInt32 value);
		/// <summary>
		/// Insert data at the beginning of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno InsertU32(UInt32 value);
		/// <summary>
		/// Remove data at the beginning of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno TrimU32([Out] UInt32% value);
		/// <summary>
		/// Remove data at the end of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno ChopU32([Out] UInt32% value);
		/// <summary>
		/// Remove data at the beginning of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Trim(size_t size);
		/// <summary>
		/// Remove data at the end of a message
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno Chop(size_t size);
		/// <summary>
		/// Clear the body of the message
		/// </summary>
		void  Clear();

		/// <summary>
		/// Returns the entire header of the message
		/// </summary>
		array<System::Byte>^ Header();
		/// <summary>
		/// Append data to a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderAppend(array<System::Byte>^ data);
		/// <summary>
		/// Insert data at the beginning of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderInsert(array<System::Byte>^ data);
		/// <summary>
		/// Append data to a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderAppendU32(UInt32 value);
		/// <summary>
		/// Insert data at the beginning of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderInsertU32(UInt32 value);
		/// <summary>
		/// Remove data at the beginning of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderTrimU32([Out] UInt32% value);
		/// <summary>
		/// Remove data at the end of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderChopU32([Out] UInt32% value);
		/// <summary>
		/// Remove data at the beginning of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderTrim(size_t size);
		/// <summary>
		/// Remove data at the end of a header
		/// </summary>
		/// <returns>Errno::ok on success</returns>
		Errno HeaderChop(size_t size);
		/// <summary>
		/// Clear the header of the message
		/// </summary>
		void  HeaderClear();

		/// <summary>
		/// Pipe number (do not use this function)
		/// </summary>
		int   GetPipe();
		/// <summary>
		/// set pipe number (do not use this function)
		/// </summary>
		void  SetPipe(int pipe);

		// The destructor will be automatically turned into IDispose
		~Msg();
	};

	public ref class SnapShot {
	public:
		property UIntPtr snapShot;
	};
	public ref class Stat {
	public:
		property UIntPtr stat;
	};
	private ref class Pipe { // not currently used
	public:
		property UInt32 pipe;
	};
	
	public ref class Aio : IDisposable {
	internal:
		property UIntPtr aio;
		property System::Action<System::Object^>^ dotNetCallback;
		property System::Object^ dotNetCallbackContext;
	public:
		Aio(System::Action<System::Object^>^ callback, Object^ object); // throws
		static Errno Alloc([Out] Aio^% aio, System::Action<System::Object^>^ callback, Object^ object);
		void  Free();
		Errno Result();
		void  Stop();
		void  Cancel();
		void  Wait();
		void  SetMsg(Msg^ msg);
		Msg^  GetMsg();
		void  SetTimeout(Int32 duration);
		~Aio();
	private:
		Aio();
		static Errno Initialize(Aio^ aio, System::Action<System::Object^>^ callback, System::Object^ object);
		void  CallbackEntry(void);
		delegate void CallbackEntryDelegate(void);
		CallbackEntryDelegate^ callbackDelegate;
	};

	/// <summary>
	/// Socket factory
	/// </summary>
	public ref class Protocols abstract sealed {
		public:
	  /// <summary>The client side on an RPC pattern. Should dial. Multiple dialers are supported</summary>
		static Errno Req0([Out] Socket^% socket);
		/// <summary>The server side on an RPC pattern. Should listen. Multiple listeners are supported</summary>
		static Errno Rep0([Out] Socket^% socket);
		/// <summary>A bus sends messages to all directly connected peers. Can listen and dial, also mixed</summary>
		static Errno Bus0([Out] Socket^% socket);
		/// <summary>A bidirectional connection. Should dial or listen</summary>
		static Errno Pair0([Out] Socket^% socket);
		/// <summary>The receiving end of a unidirectional connection. Should dial or listen</summary>
		static Errno Pull0([Out] Socket^% socket);
		/// <summary>The sending end of a unidirectional connection. Should dial or listen</summary>
		static Errno Push0([Out] Socket^% socket);
		/// <summary>The server side on an publisher/subscriber pattern. Should listen</summary>
		static Errno Pub0([Out] Socket^% socket);
		/// <summary>The client side on an publisher/subscriber pattern. Should dial</summary>
		static Errno Sub0([Out] Socket^% socket);
		/// <summary>The client side on an voting pattern. Should dial</summary>
		static Errno Respondent0([Out] Socket^% socket);
		/// <summary>The server side on an voting pattern. Should listen</summary>
		static Errno Surveyor0([Out] Socket^% socket);
	};

	/// <summary>The errors</summary>
	public enum class Errno : int {
		ok = 0,
		intr = 1,
		nomem = 2,
		inval = 3,
		busy = 4,
		timedout = 5,
		connrefused = 6,
		closed = 7,
		again = 8,
		notsup = 9,
		addrinuse = 10,
		state = 11,
		noent = 12,
		proto = 13,
		unreachable = 14,
		addrinval = 15,
		perm = 16,
		msgsize = 17,
		connaborted = 18,
		connreset = 19,
		canceled = 20,
		nofiles = 21,
		nospc = 22,
		exist = 23,
		readonly = 24,
		writeonly = 25,
		crypto = 26,
		peerauth = 27,
		internal = 1000,
		syserr = 0x10000000,
		tranerr = 0x20000000,
	};

	/// <summary>The options. The options are not really thought through, currently you have to retrieve the string for an options and use the string</summary>
	public enum class Option {
		sockname,
		domain,
		raw,
		linger,
		recvbuf,
		sendbuf,
		recvfd,
		sendfd,
		recvtimeo,
		sendtimeo,
		locaddr,
		remaddr,
		url,
		maxttl,
		protocol,
		transport,
		recvmaxsz,
		reconnmint,
		reconnmaxt
	};

	public ref class Constants abstract sealed {
	public:
		static System::String^ StrError(Errno error);
		static System::String^ Option(Option option);
		static constexpr size_t MaxAddrLen{ 128 };
	};

}

