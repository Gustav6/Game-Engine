#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

// More info can be found at their repo:
// https://github.com/ibireme/yyjson 

struct yyjson_doc;
struct yyjson_val;

/*
* The data class represents a classic object in json.
* @return
*/
class JsonData
{
public:
	JsonData();
	JsonData(const std::filesystem::path& aJsonPath);
	~JsonData();

	JsonData(const JsonData&) = delete;
	JsonData& operator=(const JsonData&) = delete;

	bool IsValid() const;

	bool GetBool(const char* aName) const;
	int GetInt(const char* aName) const;
	float GetFloat(const char* aName) const;
	std::string_view GetString(const char* aName) const;

	std::vector<bool> GetBoolArray(const char* aName) const;
	std::vector<int> GetIntArray(const char* aName) const;
	std::vector<float> GetFloatArray(const char* aName) const;
	std::vector<std::string_view> GetStringArray(const char* aName) const;

	JsonData GetObjectValue(const char* aName) const;

	bool HasKey(const char* aName) const;

private:
	// Send path to send clear debug messages
	JsonData(yyjson_doc* aDocument, yyjson_val* aValue, std::filesystem::path aPath);

	yyjson_doc* myDocument;
	yyjson_val* myRoot;

	std::filesystem::path myPath;

	bool myOwnsDocument;
};

class Json
{
public:
	static JsonData Read(const std::filesystem::path& aJsonPath);

	// TODO: Implement
	static void Write();
};

