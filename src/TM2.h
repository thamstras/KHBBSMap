#pragma once
#include "Common.h"
#include "BTypes.h"

namespace BBS
{
	struct TM2_PICTURE;
	
	class TM2
	{
	public:
		TM2(uint8 *data);
		~TM2();
	};

	struct TM2_PICTURE
	{
		TM2_PICTURE_HEADER *header;
	};
}