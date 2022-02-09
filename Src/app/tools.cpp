#include "tools.h"

namespace tools
{
	uint32_t GetAlinedSize(uint32_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}


}