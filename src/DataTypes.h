/// Emil Hedemalm
/// 2014-08-21
/// Simple enumeration. Used to specify type of included data in various simpler messages.

#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include "System/DataTypes.h"

namespace DataType 
{
	enum dataTypes 
	{
		NO_TYPE = -1,
		NULL_TYPE = -1,
		UNSIGNED_CHAR,
		INTEGER,
		FLOAT,
		BOOLEAN,
		STRING, // Defined in /Util/String/AEString.h
		VECTOR3F, // Defined in /MathLib/Vector3f.h
		VECTOR2F, // Defined in /MathLib/Vector2f.h
		QUATERNION, // Defined in /MathLib/Quaternion.h
		EXPRESSION, // Defined in /MathLib/Expression.h
		ARGUMENT_LIST, // Part of Expression calculation as defined in /MathLib/Expression/ If commas are used, it is assumed that it is part of an argument list.
	};
};

#endif
