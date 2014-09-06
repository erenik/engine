/// Emil Hedemalm
/// 2014-08-22
/** A symbolic character or string of characters that have some significance in a mathematical expression.
	http://en.wikipedia.org/wiki/Expression_%28mathematics%29
	"Symbols can designate numbers (constants), variables, operations, functions, and other mathematical symbols, as well as punctuation, symbols of grouping, and other syntactic symbols. "
*/

#include "Symbol.h"
#include "DataTypes.h"
#include "Expression.h"

Symbol::Symbol()
{
	type = BAD_TYPE;
}

Symbol::Symbol(String text, int type)
	: text(text), type(type)
{
}

Symbol::Symbol(List<Symbol> symbols, int type)
: symbols(symbols), type(type)
{
}
	
// Copy constructor o.o
Symbol::Symbol(const ExpressionResult & result)
{
	switch(result.type)
	{
		case DataType::FLOAT:
		case DataType::INTEGER:
			type = Symbol::CONSTANT;
			text = result.text;
			break;
		default:
			type = -1;
			assert(false);
	}
}


void PrintSymbolsInALine(List<Symbol> symbols)
{
	String str;
	if (symbols.Size())
		str += "\nSymbols: "+symbols[0].text;
	for (int i = 1; i < symbols.Size(); ++i)
	{
		Symbol & symbol = symbols[i];
		str += " "+symbol.text;
	}
	std::cout<<str;
}


void PrintSymbols(List<Symbol> symbols)
{
	String str;
	for (int i = 0; i < symbols.Size(); ++i)
	{
		Symbol & symbol = symbols[i];
		str += "\nSymbol "+String(i)+": "+symbol.text;
	}
	std::cout<<str;
}

