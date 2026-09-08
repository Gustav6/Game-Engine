#pragma once

#include <filesystem>
#include <string_view>

// More info can be found at their repo:
// https://github.com/ibireme/yyjson 

struct yyjson_doc;
struct yyjson_val;

class JsonData
{
public:
	JsonData();
	JsonData(const std::filesystem::path& aJsonPath);
	~JsonData();

	JsonData(const JsonData&) = delete;
	JsonData& operator=(const JsonData&) = delete;

	bool Valid() const;

	bool GetBool(const char* aName) const;
	int GetInt(const char* aName) const;
	float GetFloat(const char* aName) const;
	std::string_view GetString(const char* aName) const;

	JsonData GetObjectValue(const char* aName) const;

	bool HasKey(const char* aName) const;

private:
	JsonData(yyjson_doc* aDocument, yyjson_val* aValue);

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

