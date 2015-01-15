/// Emil Hedemalm
/// 2014-08-11
/// Setting class for Computer Vision processing filters

#include "CVFilterSetting.h"
#include <fstream>

// Default constructor only used for when loading from file.
CVFilterSetting::CVFilterSetting()
{
	// Set to true so settings may be reacted to on start-up.
	settingChanged = true;
}

/// Creates a simple button!
CVFilterSetting::CVFilterSetting(String name)
	: name(name), type(CVSettingType::BUTTON)
{
	// Set boolean value, since it might function as a trigger-boolean until the filter has done something.. enough.
	bValue = false;
	settingChanged = true;
}	

CVFilterSetting::CVFilterSetting(String name, int settingType, String value)
	: name(name), type(settingType)
{
	switch(settingType)
	{
		case CVSettingType::STRING:
			sValue = value;
			break;
		default:
			assert(false);
	}
}


CVFilterSetting::CVFilterSetting(String name, String value)
	: name(name), sValue(value), type(CVSettingType::STRING)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, bool value)
	: name(name), bValue(value), type(CVSettingType::BOOL)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, float initialValue)
	: name(name), fValue(initialValue), type(CVSettingType::FLOAT)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, int value)
	: name(name), iValue(value), type(CVSettingType::INT)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, Vector2i value)
	: name(name), vec2iData(value), type(CVSettingType::VECTOR_2I)
{

}

CVFilterSetting::CVFilterSetting(String name, Vector2f value)
	: name(name), vec2fData(value), type(CVSettingType::VECTOR_2F)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, Vector3f value)
	: name(name), vec3fData(value), type(CVSettingType::VECTOR_3F)
{
	settingChanged = true;
}

CVFilterSetting::CVFilterSetting(String name, Vector4f value)
	: name(name), vec3fData(value), type(CVSettingType::VECTOR_4F)
{
	settingChanged = true;
}

#define CV_FILTER_SETTING_VERSION_1	1
/// Save/load
bool CVFilterSetting::WriteTo(std::fstream & file)
{
	int version = CV_FILTER_SETTING_VERSION_1;
	file.write((char*)&version, sizeof(int));
	file.write((char*)&type, sizeof(int));
	name.WriteTo(file);
	switch(type)
	{
		case CVSettingType::INT:
			file.write((char*)&iValue, sizeof(int));
			break;
		case CVSettingType::FLOAT:
			file.write((char*)&fValue, sizeof(float));
			break;
		case CVSettingType::VECTOR_2I:
			vec2iData.WriteTo(file);
			break;
		case CVSettingType::VECTOR_2F:
			vec2fData.WriteTo(file);
			break;
		case CVSettingType::VECTOR_3F:
			vec3fData.WriteTo(file);
			break;
		case CVSettingType::VECTOR_4F:
			vec4fData.WriteTo(file);
			break;
		case CVSettingType::BUTTON:
			break;
		case CVSettingType::BOOL:
			file.write((char*)&bValue, sizeof(bool));
			break;
		case CVSettingType::STRING:
			sValue.WriteTo(file);
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}
bool CVFilterSetting::ReadFrom(std::fstream & file)
{
	int version = CV_FILTER_SETTING_VERSION_1;
	file.read((char*)&version, sizeof(int));
	assert(version == CV_FILTER_SETTING_VERSION_1);
	if (version != CV_FILTER_SETTING_VERSION_1)
		return false;
	file.read((char*)&type, sizeof(int));
	name.ReadFrom(file);
	switch(type)
	{
		case CVSettingType::INT:
			file.read((char*)&iValue, sizeof(int));
			break;
		case CVSettingType::FLOAT:
			file.read((char*)&fValue, sizeof(float));
			break;
		case CVSettingType::VECTOR_2I:
			vec2iData.ReadFrom(file);
			break;
		case CVSettingType::VECTOR_2F:
			vec2fData.ReadFrom(file);
			break;
		case CVSettingType::VECTOR_3F:
			vec3fData.ReadFrom(file);
			break;
		case CVSettingType::VECTOR_4F:
			vec4fData.ReadFrom(file);
			break;
		case CVSettingType::BUTTON:
			break;
		case CVSettingType::BOOL:
			file.read((char*)&bValue, sizeof(bool));
			break;
		case CVSettingType::STRING:
			sValue.ReadFrom(file);
			break;
		default:
			assert(false && "Implement");
	}
	return true;
}

// Setters
void CVFilterSetting::SetString(String value)
{
	sValue = value;
	settingChanged = true;
}
void CVFilterSetting::SetFloat(float value)
{
	this->fValue = value;
	settingChanged = true;
}
void CVFilterSetting::SetInt(int value)
{
	this->iValue = value;
	settingChanged = true;
}
void CVFilterSetting::SetBool(bool value)
{
	bValue = value;
	settingChanged = true;
}

void CVFilterSetting::SetVec2i(Vector2i value, bool silentChange /*= false*/)
{
	this->vec2iData = value;
	if (!silentChange)
		this->settingChanged = true;
}
void CVFilterSetting::SetVec2f(Vector2f value, bool silentChange)
{
	this->vec2fData = value;
	if (!silentChange)
		this->settingChanged = true;
}
void CVFilterSetting::SetVec3f(Vector3f value, bool silentChange)
{
	this->vec3fData = value;
	if (!silentChange)
		this->settingChanged = true;
}

void CVFilterSetting::SetVec4f(Vector4f value, bool silentChange /* = false*/)
{
	this->vec4fData = value;
	if (!silentChange)
		this->settingChanged = true;
}

// Checks if it has changed since last call to it. Resets the flag to false for next call.
bool CVFilterSetting::HasChanged()
{
	bool hadChanged = settingChanged;
	settingChanged = false;
	return hadChanged; 
}

