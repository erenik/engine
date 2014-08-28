/// Emil Hedemalm
/// 2014-08-22
/// A variable in an expression or function.

#include "Variable.h"

#include "DataTypes.h"

Variable::Variable()
{
	type = DataType::NO_TYPE;
}

Variable::Variable(String name) 
	: name(name) 
{
	type = DataType::NO_TYPE;
}

Variable::Variable(String name, int iValue)
	: name(name), iData(iValue), type(DataType::INTEGER)
{
}	

Variable::Variable(String name, float fValue)
	: name(name), fData(fValue), type(DataType::FLOAT)
{

}

String Variable::PrintType() 
{
	String s;
	switch(type)
	{
		case DataType::INTEGER: s = "Integer"; break;
		case DataType::FLOAT: s = "Float"; break;
		case DataType::STRING: s = "String"; break;
		case DataType::VECTOR3F: s = "Vector3f"; break;
		default: s = "Bad/no type"; break;
	}
	if (s.Length())
		std::cout<<"\n"<<s;
	return s;
};



