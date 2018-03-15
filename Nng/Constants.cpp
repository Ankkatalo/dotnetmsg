/*
Nng wrapper

Error strings and option strings




*/

#include "NngExternal.h"
#include "nng.h"
#include "NngInternal.h"
#include <cstring>

namespace Nng {

	System::String^ Constants::StrError(Errno error)
	{
		char* s = (char*) ::nng_strerror((int)error);
		if (s == nullptr) return nullptr;
		return System::Text::Encoding::UTF8->GetString((unsigned char*)s, (long) ::strlen(s));
	}
	System::String^ Constants::Option(Nng::Option option)
	{
		switch (option) {

		    case Option::sockname:   return "socket-name";
				case Option::domain:		 return "compat:domain";
				case Option::raw:				 return "raw";
				case Option::linger:		 return "linger";
				case Option::recvbuf:		 return "recv-buffer";
				case Option::sendbuf:		 return "send-buffer";
				case Option::recvfd:		 return "recv-fd";
				case Option::sendfd:		 return "send-fd";
				case Option::recvtimeo:	 return "recv-timeout";
				case Option::sendtimeo:	 return "send-timeout";
				case Option::locaddr:		 return "local-address";
				case Option::remaddr:		 return "remote-address";
				case Option::url:				 return "url";
				case Option::maxttl:		 return "ttl-max";
				case Option::protocol:	 return "protocol";
				case Option::transport:	 return "transport";
				case Option::recvmaxsz:	 return "recv-size-max";
				case Option::reconnmint: return "reconnect-time-min";
				case Option::reconnmaxt: return "reconnect-time-max";
				default: return "";
		}
	}
}