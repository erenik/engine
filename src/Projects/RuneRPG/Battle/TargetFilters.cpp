/// Emil Hedemalm
/// 2014-09-06
/// o.o

#include "TargetFilters.h"

int ParseTargetFilter(String fromString)
{
	/// If-else should do.
	if (fromString.Contains("Select 1"))
		return TargetFilter::ENEMY;
	else if (fromString.Contains("Self"))
		return TargetFilter::SELF;
	else if (fromString.Contains("World"))
		return TargetFilter::POINT;
	return TargetFilter::NULL_TARGET;
}

