/// Emil Hedemalm
/// 2014-08-22
/** A symbolic character or string of characters that have some significance in a mathematical expression.
	http://en.wikipedia.org/wiki/Expression_%28mathematics%29
	"Symbols can designate numbers (constants), variables, operations, functions, and other mathematical symbols, as well as punctuation, symbols of grouping, and other syntactic symbols. "
*/

#include "Symbol.h"

Symbol::Symbol()
{
	type = BAD_TYPE;
}

Symbol::Symbol(String text, int type)
	: text(text), type(type)
{
}

Symbol::Symbol(List<String> list, int type)
: list(list), type(type)
{
}
	


void PrintSymbols(List<Symbol> symbols)
{
	for (int i = 0; i < symbols.Size(); ++i)
	{
		Symbol & symbol = symbols[i];
		std::cout<<"\nSymbol "<<i<<": "<<symbol.text;
	}
}

