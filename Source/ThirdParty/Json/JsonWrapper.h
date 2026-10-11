#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "CommonUtilities/Math/Vector2.hpp"
#include "CommonUtilities/Math/Vector3.hpp"
#include "CommonUtilities/Math/Vector4.hpp"

// More info can be found at their repo:
// https://github.com/ibireme/yyjson 

struct yyjson_doc;
struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

enum class JsonValueType
{
	Unknown,
	Bool,
	Int,
	Unsigned,
	Float,
	String,
	Array,
	Object
};

/*
* The data class represents a classic object in json.
* @return
*/
class JsonData
{
public:
	JsonData(const std::filesystem::path& aJsonPath);

	bool IsValid() const;

	bool GetBool(const char* aName) const;
	int GetInt(const char* aName) const;
	unsigned int GetUnsigned(const char* aName) const;
	float GetFloat(const char* aName) const;
	double GetDouble(const char* aName) const;
	std::string GetString(const char* aName) const;
	JsonData GetObjectValue(const char* aName) const;

	std::vector<bool> GetBoolArray(const char* aName) const;
	std::vector<int> GetIntArray(const char* aName) const;
	std::vector<unsigned int> GetUnsignedArray(const char* aName) const;
	std::vector<float> GetFloatArray(const char* aName) const;
	std::vector<double> GetDoubleArray(const char* aName) const;
	std::vector<std::string> GetStringArray(const char* aName) const;
	std::vector<JsonData> GetObjectValueArray(const char* aName) const;

	std::vector<std::string> GetKeys() const;

	JsonValueType GetType(const char* aName) const;
	JsonValueType GetArrayType(const char* aName) const;

	bool HasKey(const char* aName) const;

	std::string GetPath() const;

private:
	JsonData();
	JsonData(std::shared_ptr<yyjson_doc> aDocument, yyjson_val* aValue, std::filesystem::path aPath);

	std::shared_ptr<yyjson_doc> myDocument;
	yyjson_val* myRoot;

	std::filesystem::path myPath;
};

class JsonWriter
{
public:
	JsonWriter(const std::filesystem::path& aJsonDestination);

	// TODO: Add methods to write

private:
	yyjson_mut_doc* myDocument;
	yyjson_mut_val* myRoot;
};

class Json
{
public:
	static JsonData Read(const std::filesystem::path& aJsonPath);
	static JsonWriter Write(const std::filesystem::path& aJsonDestination);
};

