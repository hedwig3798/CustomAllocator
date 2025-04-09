#pragma once
#include "RenameType.h"
namespace PointerMath
{
	inline uint8 GetForwardAdjustment(uintptr _ptr, uint8 _alignment)
	{
		uint8 adjustment = _alignment - (_ptr & (_alignment - 1));

		if (adjustment == _alignment)
		{
			return 0;
		}
		return adjustment;
	}

	inline uint8 GetForwardAdjustment(uintptr _ptr, uint8 _alignment, uint8 _headerSize)
	{
		uint8 adjustment = GetForwardAdjustment(_ptr, _alignment);
		
		if (adjustment < _headerSize)
		{
			_headerSize -= adjustment;
			adjustment += _alignment * (_headerSize / _alignment);
			if ((_headerSize % _alignment) > 0)
			{
				adjustment += _alignment;
			}
		}

		return adjustment;
	}
}