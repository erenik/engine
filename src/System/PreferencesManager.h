/// Emil Hedemalm
/// 2013-11-18
/// Updated 2014-01-24
/// Manager class for preferences

#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <String/AEString.h>
#include "ApplicationDefaults.h"

/// Possible value types of settings.
enum settingTypes
{
    SETTING_INT,
    SETTING_FLOAT,
    SETTING_STRING,
	SETTING_BOOL,
};

/// Structure for a saved setting.
struct Setting
{
    int type;
    String name;
    
    String sData;
    float fData;
    int iData;
	bool bData;
};

/// Easy-access macro
#define Preferences (*PreferencesManager::Instance())

/// Singleton for handling save/load of a series of values on startup/shutdown etc.
class PreferencesManager
{
private:
	static PreferencesManager * preferencesManager;
        
	PreferencesManager();
	PreferencesManager(const PreferencesManager&);
	~PreferencesManager();
	PreferencesManager& operator = (const PreferencesManager&);
public:
	/// Given name will decide where in ~/.config/ the preferences are kept.
	static void Allocate();
	static void Deallocate();
    /// Get the singelton instance.
	static PreferencesManager * Instance();
	/// Creates folder as needed.
	void EnsurePreferencesFolderExists();
	/// Saves to default location.
	bool Save();
    /// Load form the default location
	bool Load();

	/// Getter, returns false if it does not exist
	bool GetFloat(String name, float * destination);
	/// Setter, creates if it doesn't exist.
	void SetFloat(String name, float source);
	/// Getter, returns false if it does not exist
	bool GetString(String name, String * destination);
	/// Setter, creates if it doesn't exist.
	void SetString(String name, String stringData);
	/// Getter for boolean preferences.
	bool GetBool(String name, bool * destination);
	/// Setter for boolean preferences.
	void SetBool(String name, bool value);
private:
	Setting * GetSetting(String name);

	String preferencesPath;
	List<Setting*> settings;
};

#endif
