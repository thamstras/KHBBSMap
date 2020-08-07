#include <exception>
#include <stdexcept>

#include "Common.h"
#include "BTypes.h"
#include "BBSTypes.h"
#include "TM2.h"

using namespace BBS;

TM2::TM2(uint8 *data)
{
	TM2_HEADER *tm2Header = (TM2_HEADER *)(data);
	if (memcmp(tm2Header->fileID, "TIM2", 4) != 0)
	{
		throw new std::invalid_argument("Argument 'data' does not point to a valid TM2 Header.");
	}
}