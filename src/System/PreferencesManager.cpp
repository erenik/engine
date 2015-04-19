// Emil Hedemalm
// 2013-11-18
// Singleton for handling save/load of a series of values on startup/shutdown etc.

#include "PreferencesManager.h"
#include "Application/Application.h"
#include <cassert>
#include <fstream>
#include "File/FileUtil.h"
#include "OS/OSUtil.h"

#define FILENAME "/preferences.dat"

PreferencesManager * PreferencesManager::preferencesManager = NULL;

PreferencesManager::PreferencesManager()
{
}

PreferencesManager::~PreferencesManager()
{
	while (settings.Size()){
		Setting * s = settings[0];
		settings.Remove(s);
		delete s;
	}
}

/// Given name will decide where in ~/.config/ the preferences are kept.
void PreferencesManager::Allocate()
{
	assert(preferencesManager == NULL);
	preferencesManager = new PreferencesManager();
	preferencesManager->EnsurePreferencesFolderExists();
}

void PreferencesManager::Deallocate()
{
	assert(preferencesManager);
	delete preferencesManager;
	preferencesManager = NULL;
}

PreferencesManager * PreferencesManager::Instance() { 
	assert(preferencesManager);
	return preferencesManager; 
};

/// Creates folder as needed.
void PreferencesManager::EnsurePreferencesFolderExists()
{
//	std::cout << "\nApplication name: "<<Application::name;
	assert(Application::name.Length());
	/// TODO: Add home path somehow.
	String homePath = OSUtil::GetHomeDirectory();
	homePath.Replace('\\', '/');
	String dirPath = homePath;
	dirPath.Add("/.config/");
	dirPath.Add(Application::name);
	std::wcout << "\nEnsure preferences folder exists: "<<dirPath;
	if (!PathExists(dirPath))
	{
		std::cout<<"\nCreating preferences directory.";
		/// Creates directories until the entire path is valid.
		CreateDirectoriesForPath(dirPath);	
	}
	preferencesPath = dirPath;
}

/// Saves to default location.
bool PreferencesManager::Save()
{
	String path = preferencesPath;
	path += FILENAME;
	std::fstream file;
	file.open(path.c_str(), std::ios_base::out | std::ios_base::binary);
	if (!file.is_open())
		return false;
	int numSettings = settings.Size();
	file.write((char*)&numSettings, sizeof(int));
	for (int i = 0; i < numSettings; ++i)
    {
		Setting * setting = settings[i];
		std::cout << "Saving setting "<<setting->name<<"..";
        
		file.write((char*)&setting->type, sizeof(int));
		setting->name.WriteTo(file);
		switch(setting->type){
			case SETTING_FLOAT:
				file.write((char*)&setting->fData, sizeof(float));
				break;
			case SETTING_INT:
				file.write((char*)&setting->iData, sizeof(int));
				break;
			case SETTING_STRING:
				setting->sData.WriteTo(file);	
				break;
			case SETTING_BOOL:
				file.write((char*)&setting->bData, sizeof(bool));
				break;
			default:
				assert(false && "Unsupported.");
		}
	}
	file.close();
}

bool PreferencesManager::Load()
{
	String path = preferencesPath;
	path += FILENAME;
	
	std::fstream file;
	const char * pathCStr = path.c_str();
	file.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
	std::cout << "Path: "<<pathCStr;
	if (!file.is_open())
		return false;
	int numSettings;
	file.read((char*)&numSettings, sizeof(int));
	if (numSettings > 5000){
		std::cout<<"Number of settings invalid. Aborting load-procedure.";
		file.close();
		return false;
	}
	for (int i = 0; i < numSettings; ++i)
    {
		Setting * setting = new Setting();
		file.read((char*)&setting->type, sizeof(int));
		setting->name.ReadFrom(file);
		std::cout << "Loading setting "<<setting->name;
		switch(setting->type)
        {
			case SETTING_FLOAT:
				file.read((char*)&setting->fData, sizeof(float));
				break;
			case SETTING_INT:
				file.read((char*)&setting->iData, sizeof(int));
				break;
			case SETTING_STRING:
				setting->sData.ReadFrom(file);
				break;
			case SETTING_BOOL:
				file.read((char*)&setting->bData, sizeof(bool));
				break;
			default:
				assert(false && "Unsupported.");
		}
		settings.Add(setting);
	}
	file.close();
}


/// Getter/setters, returns false if it does not exist
bool PreferencesManager::GetFloat(String name, float * destination){
	Setting * setting = GetSetting(name);
	if (!setting)
		return false;
	*destination = setting->fData;
	return true;
}

/// Setter, creates if it doesn't exist.
void PreferencesManager::SetFloat(String name, float fValue){
	Setting * setting = GetSetting(name);
	if (!setting){
		setting = new Setting();
		setting->type = SETTING_FLOAT;
		setting->name = name;
		settings.Add(setting);
	}
	setting->fData = fValue;
}
/// Getter, returns false if it does not exist
bool PreferencesManager::GetString(String name, String * destination){
	Setting * setting = GetSetting(name);
	if (!setting)
		return false;
	*destination = setting->sData;
	return true;		
}
/// Setter, creates if it doesn't exist.
void PreferencesManager::SetString(String name, String source){
	Setting * setting = GetSetting(name);
	if (!setting){
		setting = new Setting();
		setting->type = SETTING_STRING;
		setting->name = name;
		settings.Add(setting);
	}
	setting->sData = source;
}

/// Getter for boolean preferences.
bool PreferencesManager::GetBool(String name, bool * destination)
{
	Setting * setting = GetSetting(name);
	if (!setting){
		return false;
	}
	if (setting->type != SETTING_BOOL)
		return false;
	*destination = setting->bData;
	return true;
}
/// Setter for boolean preferences.
void PreferencesManager::SetBool(String name, bool value)
{
	Setting * setting = GetSetting(name);
	if (!setting){
		setting = new Setting();
		setting->type = SETTING_BOOL;
		setting->name = name;
		settings.Add(setting);
	}
	setting->bData = value;
}

Setting * PreferencesManager::GetSetting(String name){
	for (int i = 0; i < settings.Size(); ++i){
		Setting * s = settings[i];
		if (s->name == name)
			return s;
	}
	return NULL;
}

