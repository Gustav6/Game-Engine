#pragma once

#include <filesystem>

struct yyjson_doc;
struct yyjson_val;

class JsonData
{
public:
	JsonData(const std::filesystem::path& aJsonPath);
	~JsonData();

	JsonData(const JsonData&) = delete;
	JsonData& operator=(const JsonData&) = delete;

	bool Valid() const;

	bool GetBool(const char* aName) const;
	int GetInt(const char* aName) const;
	float GetFloat(const char* aName) const;
	std::string_view GetString(const char* aName) const;

	bool HasKey(const char* aName) const;

private:
	yyjson_doc* myDocument;
	yyjson_val* myRoot;

	std::filesystem::path myPath;
};

class Json
{
public:
	static JsonData Read(const std::filesystem::path& aJsonPath);

	// TODO: Implement
	static void Write();
};

