/// Emil Hedemalm
/// 2015-05-20
/// Output to console, managed..

#include "Output.h"

/// o.o
bool Output(String text, List<TextError> * previousErrors)
{
	TextError newErr(text);
	bool wasThere = false;
	int times = 0;
	if (previousErrors)
	{
		for (int i = 0; i < previousErrors->Size(); ++i)
		{
			TextError & err = (*previousErrors)[i];
			if (err.text == newErr.text)
			{
				times = err.times++;
				wasThere = true;
			}
		}
		if (!wasThere)
			previousErrors->AddItem(newErr);
	}
	int skipFactor = 10;
	if (times > 100)
	{
		// Just skip it now then?
		skipFactor = 100;
	}
	if (times % skipFactor == 0)
	{
		std::cout<<"\n"<<text;
		if (times > 10)
			std::cout<<" "<<times<<"th occurance";
		return true;
	}
	return false;
}