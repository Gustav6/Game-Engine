#include "JsonWrapper.h"

#include "yyjson/yyjson.h"

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(JSONLog, JSON, Verbose);

DEFINE_LOG_CATEGORY(JSONLog);

JsonData::JsonData(const std::filesystem::path& aJsonPath)
{
	myRoot = nullptr;
	myDocument = nullptr;
	myPath = aJsonPath;

	if (!std::filesystem::exists(aJsonPath))
	{
		LOG(JSONLog, Error, "File does not exist {}!", aJsonPath.string());
		return;
	}

	yyjson_read_err error;
	myDocument = yyjson_read_file(aJsonPath.string().data(), 0, nullptr, &error);

	if (myDocument == nullptr)
	{
		LOG(JSONLog, Error, "Could not read json {}! Error [{}] : {} at position: {}!", aJsonPath.string(), error.code, error.msg, error.pos);
		return;
	}

	myRoot = yyjson_doc_get_root(myDocument);
}

JsonData::~JsonData()
{
	yyjson_doc_free(myDocument);
}

bool JsonData::Valid() const
{
	if (myRoot == nullptr)
	{
		return false;
	}

	return true;
}

bool JsonData::GetBool(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key {} not found in {}!", aName, myPath.string());
		return false;
	}

	if (!yyjson_is_bool(data))
	{
		LOG(JSONLog, Warning, "Key {} exists but is not a bool, type: {} in {}!", aName, yyjson_get_type_desc(data), myPath.string());
		return false;
	}

	return yyjson_get_bool(data);
}

int JsonData::GetInt(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key {} not found in {}!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_int(data))
	{
		LOG(JSONLog, Warning, "Key {} exists but is not a int, type: {} in {}!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return yyjson_get_int(data);
}

float JsonData::GetFloat(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key {} not found in {}!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		LOG(JSONLog, Warning, "Key {} exists but is not a double, type: {} in {}!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return static_cast<float>(yyjson_get_num(data));
}

std::string_view JsonData::GetString(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key {} not found in {}!", aName, myPath.string());
		return "";
	}

	if (!yyjson_is_str(data))
	{
		LOG(JSONLog, Warning, "Key {} exists but is not a string, type: {} in {}!", aName, yyjson_get_type_desc(data), myPath.string());
		return "";
	}

	return yyjson_get_str(data);
}

bool JsonData::HasKey(const char* aName) const
{
	if (yyjson_obj_get(myRoot, aName) != nullptr)
	{
		return true;
	}

	return false;
}

JsonData Json::Read(const std::filesystem::path& aJsonPath)
{
	return JsonData(aJsonPath);
}