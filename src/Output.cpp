/// Emil Hedemalm
/// 2015-05-20
/// Output to console, managed..

#include "Output.h"

List<TextError> previousTextErrors;

/// o.o
void Output(String text)
{
	TextError newErr(text);
	bool wasThere = false;
	int times = 0;
	for (int i = 0; i < previousTextErrors.Size(); ++i)
	{
		TextError & err = previousTextErrors[i];
		if (err.text == newErr.text)
		{
			times = err.times++;
			wasThere = true;
		}
	}
	if (!wasThere)
		previousTextErrors.AddItem(newErr);
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
	}
}