#pragma once
#include<stdint.h>
#include<queue>
#include<assert.h>
#include<unordered_map>
#include<vector>
#include<memory>
///Create a Table to store element, can obtain the element by offset and uinque_id



template<typename t1, typename t2>
inline t1 align(t1 numberToAlign, t2 alignment)
{
	if (alignment)
	{
		t1 align1 = numberToAlign % (t1)alignment;
		if (!align1) { align1 += (t1)alignment; }
		numberToAlign += t1(alignment) - align1;
	}
	return numberToAlign;
}

namespace tEngine {


	


}