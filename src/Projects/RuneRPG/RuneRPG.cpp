/// Emil Hedemalm
/// 2014-08-29
/// Utility functions and stuff.

#include "RuneRPG.h"
#include "Random/Random.h"

/// Parses an integer from target string. Valid entries may be regular numbers, but also random "Random 1-35".
int RParseInt(String str)
{
	Random rand;
	if (str.Contains("Random"))
	{
		List<String> args = str.Tokenize("- ");
		if (args.Size() < 3)
		{
			std::cout<<"\nBad arguments to random.";
			return 0;
		}
		String arg1 = args[1];
		String arg2 = args[2];
		int min = arg1.ParseInt();
		int max = arg2.ParseInt();
		int v = rand.Randi(max-min) + min;
		assert(v >= min);
		return v;
	}
	else 
		return str.ParseInt();
}
