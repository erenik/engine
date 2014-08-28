/// Emil Hedemalm
/// 2014-08-22
/** A symbolic character or string of characters that have some significance in a mathematical expression.
	http://en.wikipedia.org/wiki/Expression_%28mathematics%29
	"Symbols can designate numbers (constants), variables, operations, functions, and other mathematical symbols, as well as punctuation, symbols of grouping, and other syntactic symbols. "
*/

#ifndef SYMBOL_H
#define SYMBOL_H

#include "String/AEString.h"

class Symbol 
{
public:
	Symbol();
	Symbol(String text, int type);
	/// Types.
	enum {
		BAD_TYPE = -1,
		CONSTANT,
		VARIABLE,
		OPERATOR,
		BEGIN_PARENTHESIS,
		END_PARENTHESIS,
	};
	/// Type, see above.
	int type;
	/// In raw-text.
	String text;
	/// Only applicable for 
	String name;
};


#endif


