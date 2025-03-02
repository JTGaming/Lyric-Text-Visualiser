#pragma once
#ifdef ENABLE_CONFIG
#include <vector>
#include <string>

extern std::string pathToConfigs;
extern std::vector<std::string> configFiles;

//object holding the name of a config entry, the category for grouping in output file,
//	pointer to the variable, and a possible default entry which could be used to reset a variable value (removed)
template <typename T>
class ConfigValue
{
public:
	ConfigValue(const std::string& category_, const std::string& name_, T* value_, const T* const deflt) : category(category_), name(name_), value(value_)
	{
		if (!def)
		{
			def = new T;
			*def = deflt ? *deflt : *value_;
		}
	}

	ConfigValue(const std::string& category_, const std::string& name_, T* value_, const T& deflt) : category(category_), name(name_), value(value_)
	{
		if (!def)
		{
			def = new T;
			*def = deflt;
		}
	}

	std::string category, name;
	T* value, * def = nullptr;
};

class CConfig
{
protected:
	//supported variable types
	std::vector<ConfigValue<int>> ints;
	std::vector<ConfigValue<bool>> bools;
	std::vector<ConfigValue<float>> floats;
	std::vector<ConfigValue<char>> chars;
	std::vector<ConfigValue<std::string>> strings;

private:
	void SetupValue(int&, int, const std::string&, const std::string&);
	void SetupValue(bool&, bool, const std::string&, const std::string&);
	void SetupValue(float&, float, const std::string&, const std::string&);
	void SetupValue(char*, const char*, const std::string&, const std::string&);
	void SetupValue(std::string&, const std::string&, const std::string&, const std::string&);

	static unsigned __stdcall StaticLoadPath(void* Param);
	static unsigned __stdcall StaticSavePath(void* Param);
	void StartSave(const std::string&);
	void StartLoad(const std::string&);

	void SaveToPath();
	void LoadFromPath();
	void GetAllConfigsInFolder(std::vector<std::string>& files, const std::string& path, const std::string& ext);

public:
	CConfig()
	{
		Setup();
	}

	void Setup();
	void LoadConfigs();
	void Save(bool FNO = false);
	void Load();
	void Delete();

protected:
	std::string file_to_load;
	std::string file_to_save;
	volatile bool CanSave = true;
	volatile bool CanLoad = true;

	const char* extension = ".txt";
};

extern CConfig* ConfigSys;
#endif