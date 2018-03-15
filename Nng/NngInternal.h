#pragma once

namespace Nng {
	extern nng_aio* getNativeAio(Aio^ aio);
	extern nng_msg* getNativeMsg(Msg^ msg);
}
