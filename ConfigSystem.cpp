// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#ifdef ENABLE_CONFIG
#define NOMINMAX
#include "ConfigSystem.h"
#include "Variables.h"
#include <ShlObj.h>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <thread>

//here go all the variables you want to be saved into a config file
//can be customized with a switch(config_type) to have different types of configs (removed)
void CConfig::Setup()
{
	ints.clear();
	bools.clear();
	floats.clear();
	chars.clear();
	strings.clear();

	SetupValue(g_Options.LyricVars.DefaultFontSize, 64, "LyricVars", "DefaultFontSize");
	SetupValue(g_Options.LyricVars.DefaultFontDuration, 250, "LyricVars", "DefaultFontDuration");
	SetupValue(g_Options.LyricVars.DefaultTextDuration, 1000, "LyricVars", "DefaultTextDuration");
	SetupValue(g_Options.LyricVars.DurType, DUR_LENGTH, "LyricVars", "DurType");
}

//check the dedicated config folder and load the files if any exist
//setting a specialised extension might help against loading up random files
void CConfig::GetAllConfigsInFolder(std::vector<std::string>& files, const std::string& path, const std::string& ext)
{
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
		for (auto& el : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied))
			if (std::filesystem::is_regular_file(el) && el.path().extension() == ext)
			{
				std::string tmp_f_name = el.path().filename().string();
				size_t pos = tmp_f_name.rfind(ext);
				std::string fName = (std::string::npos == pos) ? tmp_f_name : tmp_f_name.substr(0, pos);

				files.emplace_back(fName);
			}
}

void CConfig::LoadConfigs() {

	auto& filespace = configFiles;
	if ((int)filespace.size())
		filespace.clear();

	GetAllConfigsInFolder(filespace, pathToConfigs, extension);
	std::sort(filespace.begin(), filespace.end());
}

//parse and save each variable type into its respective container
void CConfig::SetupValue(int& value, int def, const std::string& category, const std::string& name)
{
	value = def;
	ints.emplace_back(category, name, &value, &def);
}

void CConfig::SetupValue(float& value, float def, const std::string& category, const std::string& name)
{
	value = def;
	floats.emplace_back(category, name, &value, &def);
}

void CConfig::SetupValue(bool& value, bool def, const std::string& category, const std::string& name)
{
	value = def;
	bools.emplace_back(category, name, &value, &def);
}

void CConfig::SetupValue(char* value, const char* def, const std::string& category, const std::string& name)
{
	strncpy_s(value, 128, def, _TRUNCATE);
	chars.emplace_back(category, name, value, def);
}

void CConfig::SetupValue(std::string& value, const std::string& def, const std::string& category, const std::string& name)
{
	value = def;
	strings.emplace_back(category, name, &value, &def);
}

//deletes file completely instead of sending to recycle bin
const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	size_t outSize;
	mbstowcs_s(&outSize, wc, cSize, c, cSize - 1);

	return wc;
}

void deleteItem(const char* item)
{
	HRESULT hr;
	IFileOperation* pfo;
	PCWSTR itemPCWSTR = GetWC(item);
	hr = CoCreateInstance(CLSID_FileOperation,
		NULL,
		CLSCTX_ALL,
		//IID_IFileOperation,
		IID_PPV_ARGS(&pfo));
	if (!SUCCEEDED(hr))
	{
		delete[] itemPCWSTR;
		return;
	}
	hr = pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_NOCONFIRMATION);
	if (!SUCCEEDED(hr))
	{
		delete[] itemPCWSTR;
		return;
	}
	IShellItem* deleteItem = NULL;
	hr = SHCreateItemFromParsingName(itemPCWSTR,
		NULL,
		IID_PPV_ARGS(&deleteItem));
	if (!SUCCEEDED(hr))
	{
		delete[] itemPCWSTR;
		return;
	}
	hr = pfo->DeleteItem(deleteItem, NULL);
	if (deleteItem != NULL)
	{
		deleteItem->Release();
	}
	if (!SUCCEEDED(hr))
	{
		delete[] itemPCWSTR;
		return;
	}
	hr = pfo->PerformOperations();
	if (!SUCCEEDED(hr))
	{
		delete[] itemPCWSTR;
		return;
	}
	pfo->Release();
	delete[] itemPCWSTR;
}

void CConfig::Delete()
{
	deleteItem((pathToConfigs + configFiles.at(g_Options.Menu.ConfigFile) + extension).c_str());
	LoadConfigs();
}

//creates a separate thread to prevent stalling the main app
void CConfig::Save(bool filenameOverride)
{
	auto& config_file = configFiles;
	auto& config_opt = g_Options.Menu.ConfigFile;
	auto* config_ptr = g_Options.Menu.ConfigName;
	std::string config_name = config_ptr;
	memset(&config_ptr[0], 0, sizeof(*config_ptr));

	if (!config_name.empty() && filenameOverride && std::find(config_file.begin(), config_file.end(), config_name) == config_file.end())
	{
	}
	else if ((int)config_file.size() > config_opt && !filenameOverride)
		config_name = config_file.at(config_opt);
	else
		return;

	std::string folder = pathToConfigs;
	StartSave(folder + config_name + extension);
}

void CConfig::SaveToPath()
{
	while (!CanLoad)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

	const std::string file = file_to_save;

	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> output_maps{};

	for (const auto& value : ints)
	{
		auto found_map = output_maps.find(value.category);
		if (found_map != output_maps.end())
			found_map->second.try_emplace(value.name, std::to_string(*value.value));
		else
		{
			std::unordered_map<std::string, std::string> new_map{};
			new_map.try_emplace(value.name, std::to_string(*value.value));
			output_maps.try_emplace(value.category, new_map);
		}
	}

	for (const auto& value : floats)
	{
		auto found_map = output_maps.find(value.category);
		if (found_map != output_maps.end())
			found_map->second.try_emplace(value.name, std::to_string(*value.value));
		else
		{
			std::unordered_map<std::string, std::string> new_map{};
			new_map.try_emplace(value.name, std::to_string(*value.value));
			output_maps.try_emplace(value.category, new_map);
		}
	}

	for (const auto& value : bools)
	{
		auto found_map = output_maps.find(value.category);
		if (found_map != output_maps.end())
			found_map->second.try_emplace(value.name, std::to_string(*value.value));
		else
		{
			std::unordered_map<std::string, std::string> new_map{};
			new_map.try_emplace(value.name, std::to_string(*value.value));
			output_maps.try_emplace(value.category, new_map);
		}
	}

	for (const auto& value : chars)
		if (strlen(value.value))
		{
			auto found_map = output_maps.find(value.category);
			if (found_map != output_maps.end())
				found_map->second.try_emplace(value.name, value.value);
			else
			{
				std::unordered_map<std::string, std::string> new_map{};
				new_map.try_emplace(value.name, value.value);
				output_maps.try_emplace(value.category, new_map);
			}
		}

	for (const auto& value : strings)
		if (value.value->length())
		{
			auto found_map = output_maps.find(value.category);
			if (found_map != output_maps.end())
				found_map->second.try_emplace(value.name, *value.value);
			else
			{
				std::unordered_map<std::string, std::string> new_map{};
				new_map.try_emplace(value.name, *value.value);
				output_maps.try_emplace(value.category, new_map);
			}
		}

	std::ofstream output_file(file, std::ios::out | std::ios::trunc);
	if (output_file.is_open())
	{
		for (const auto& category : output_maps)
		{
			output_file << "[" << category.first << "]" << "\n";
			for (const auto& key : category.second)
				output_file << key.first << "=" << key.second << "\n";
		}
		output_file.close();
	}

	LoadConfigs();

	CanSave = true;
}

unsigned __stdcall CConfig::StaticSavePath(void* Param)
{
	CConfig* This = (CConfig*)Param;
	This->SaveToPath();

	return 0;
}

void CConfig::StartSave(const std::string& file)
{
	if (!CanSave || !CanLoad)
		return;

	CanSave = false;
	file_to_save = file;

	auto main_handle = (HANDLE)_beginthreadex(NULL, 0, &StaticSavePath, this, 0, nullptr);
	if (main_handle)
	{
		SetThreadPriority(main_handle, THREAD_PRIORITY_HIGHEST);
		CloseHandle(main_handle);
	}
}

//creates a separate thread to prevent stalling the main app
void CConfig::Load()
{
	while (!CanSave)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));

	g_Options.Menu.AssignedConfig = configFiles.at(g_Options.Menu.ConfigFile);

	StartLoad(pathToConfigs + g_Options.Menu.AssignedConfig + extension);
	while (!CanSave || !CanLoad)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void CConfig::LoadFromPath()
{
	while (!CanSave)
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	std::string file = file_to_load;

	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> input_maps{};

	std::string line;
	std::ifstream input_file(file);
	if (input_file.is_open())
	{
		std::string current_cat{};
		std::unordered_map<std::string, std::string> current_map{};

		while (getline(input_file, line))
		{
			size_t open_br = line.find('[');
			size_t close_br = line.find(']');
			if (!open_br && close_br != std::string::npos)
			{
				if (current_cat.length() && current_map.size())
				{
					input_maps.try_emplace(current_cat, current_map);
					current_map.clear();
				}
				current_cat = line.substr(1, line.length() - 2);
			}
			else
			{
				size_t sep = line.find('=');
				std::string key = line.substr(0, sep);
				std::string val = line.substr(sep + 1);
				current_map.try_emplace(key, val);
			}
		}
		if (current_cat.length() && current_map.size())
			input_maps.try_emplace(current_cat, current_map);

		input_file.close();
	}
	else
	{
		CanLoad = true;
		return;
	}

	for (const auto& value : ints)
	{
		*value.value = *value.def;

		auto map = input_maps.find(value.category);
		if (map != input_maps.end())
		{
			auto key = map->second.find(value.name);
			if (key != map->second.end())
				if (key->second.length())
					*value.value = atoi(key->second.c_str());
		}
	}

	for (const auto& value : floats)
	{
		*value.value = *value.def;

		auto map = input_maps.find(value.category);
		if (map != input_maps.end())
		{
			auto key = map->second.find(value.name);
			if (key != map->second.end())
				if (key->second.length())
					*value.value = (float)atof(key->second.c_str());
		}
	}

	for (const auto& value : bools)
	{
		*value.value = *value.def;

		auto map = input_maps.find(value.category);
		if (map != input_maps.end())
		{
			auto key = map->second.find(value.name);
			if (key != map->second.end())
				if (key->second.length())
					*value.value = (bool)atoi(key->second.c_str());
		}
	}

	for (const auto& value : chars)
	{
		strncpy_s(value.value, 128, value.def, _TRUNCATE);

		auto map = input_maps.find(value.category);
		if (map != input_maps.end())
		{
			auto key = map->second.find(value.name);
			if (key != map->second.end())
				if (key->second.length())
					strncpy_s(value.value, std::min(128, (int)key->second.length() + 1), key->second.c_str(), _TRUNCATE);
		}
	}

	for (const auto& value : strings)
	{
		*value.value = *value.def;

		auto map = input_maps.find(value.category);
		if (map != input_maps.end())
		{
			auto key = map->second.find(value.name);
			if (key != map->second.end())
				if (key->second.length())
					*value.value = key->second;
		}
	}

	CanLoad = true;
}

unsigned __stdcall CConfig::StaticLoadPath(void* Param)
{
	CConfig* This = (CConfig*)Param;
	This->LoadFromPath();

	return 0;
}

void CConfig::StartLoad(const std::string& file)
{
	if (!CanLoad || !CanSave)
		return;

	CanLoad = false;
	file_to_load = file;

	auto main_handle = (HANDLE)_beginthreadex(NULL, 0, &StaticLoadPath, this, 0, nullptr);
	if (main_handle)
	{
		SetThreadPriority(main_handle, THREAD_PRIORITY_HIGHEST);
		CloseHandle(main_handle);
	}
}

std::string pathToConfigs;
std::vector<std::string> configFiles;

CConfig* ConfigSys = new CConfig();
#endif