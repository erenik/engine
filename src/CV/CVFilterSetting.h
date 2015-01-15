/// Emil Hedemalm
/// 2014-08-11
/// Setting class for Computer Vision processing filters

#ifndef CV_FILTER_SETTING_H
#define CV_FILTER_SETTING_H

#include "String/AEString.h"
#include "MathLib.h"
#include "System/DataTypes.h"

/// Possible value types of settings.
namespace CVSettingType {
enum settingTypes
{
    INT = 1,
    FLOAT,
    STRING,
	BOOL,
	VECTOR_2I,
	VECTOR_2F,
	VECTOR_3F,
	VECTOR_4F,
	BUTTON, // A bool that is only activated once at a time, not saved.
};};

/// Structure for a saved setting.
class CVFilterSetting
{
	friend class CVFilter;
public:
	// Default constructor only used for when loading from file.
	CVFilterSetting();
	/// Creates a simple button!
	CVFilterSetting(String name);
	// Regular constructors.
	CVFilterSetting(String name, int settingType, String value);
	CVFilterSetting(String name, String value);
	CVFilterSetting(String name, bool value);
	CVFilterSetting(String name, float initialValue);
	CVFilterSetting(String name, int value);
	CVFilterSetting(String name, Vector2i value);
	CVFilterSetting(String name, Vector2f value);
	CVFilterSetting(String name, Vector3f value);
	CVFilterSetting(String name, Vector4f value);
	
	/// Save/load
	bool WriteTo(std::fstream & file);
	bool ReadFrom(std::fstream & file);

    int type;
    String name;
	
	// Setters
	void SetString(String value);
	void SetFloat(float value);
	void SetInt(int value);
	void SetBool(bool value);
	void SetVec2i(Vector2i value, bool silentChange = false);
	void SetVec2f(Vector2f value, bool silentChange = false);
	void SetVec3f(Vector3f value, bool silentChange = false);
	void SetVec4f(Vector4f value, bool silentChange = false);

	// Checks if it has changed since last call to it. Resets the flag to false for next call.
	bool HasChanged();

	// Getters
	String GetString() { return sValue;};
	float GetFloat(){return fValue;};
	int GetInt(){return iValue;};
	bool GetBool(){return bValue;};
	Vector2i GetVec2i(){return vec2iData;};
	Vector2f GetVec2f(){return vec2fData;};
	Vector3f GetVec3f(){return vec3fData;};
	Vector4f GetVec4f(){return vec4fData;};
private:
    String sValue;
    float fValue;
    int iValue;
	bool bValue;
	Vector2i vec2iData;
	Vector2f vec2fData;
	Vector3f vec3fData;
	Vector4f vec4fData;

	/// If a value is adjusted this should set to true. Filters may react to it, re-adjusting the flag.
	bool settingChanged;
};

#endif
