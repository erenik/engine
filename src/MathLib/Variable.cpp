/// Emil Hedemalm
/// 2014-08-22
/// A variable in an expression or function.

#include "Variable.h"

#include "DataTypes.h"

/// o.o prepends the names of all variables in the list.
void PrependVariables(List<Variable> & list, String withString)
{
	for (int i = 0; i < list.Size(); ++i)
	{
		Variable & var = list[i];
		var.name = withString+var.name;
	}
}


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
	: name(name), iValue(iValue), type(DataType::INTEGER)
{
}	

Variable::Variable(String name, float fValue)
	: name(name), fValue(fValue), type(DataType::FLOAT)
{

}

Variable::Variable(String name, Vector3f vec3f)
	: name(name), vec3fValue(vec3f), type(DataType::VECTOR_3F)
{
}

Variable::Variable(String name, Vector4f vec4f)
	: name(name), vec4fValue(vec4f), type(DataType::VECTOR_4F)
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
		case DataType::VECTOR_3F: s = "Vector3f"; break;
		case DataType::VECTOR_4F: s = "Vector4f"; break;
		default: s = "Bad/no type"; break;
	}
	if (s.Length())
		std::cout<<"\n"<<s;
	return s;
};



