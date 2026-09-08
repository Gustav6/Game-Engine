#include "JsonWrapper.h"

#include "yyjson/yyjson.h"

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(JSONLog, JSON, Verbose);

DEFINE_LOG_CATEGORY(JSONLog);

JsonData::JsonData()
{
	myRoot = nullptr;
	myDocument = nullptr;
	myOwnsDocument = false;
}

JsonData::JsonData(const std::filesystem::path& aJsonPath)
{
	myRoot = nullptr;
	myDocument = nullptr;
	myOwnsDocument = true;
	myPath = aJsonPath;

	if (!std::filesystem::exists(aJsonPath))
	{
		LOG(JSONLog, Error, "File does not exist [{}]!", aJsonPath.string());
		return;
	}

	yyjson_read_err error;
	myDocument = yyjson_read_file(aJsonPath.string().data(), 0, nullptr, &error);

	if (myDocument == nullptr)
	{
		LOG(JSONLog, Error, "Could not read json [{}]! Error [{}] : [{}] at position: [{}]!", aJsonPath.string(), error.code, error.msg, error.pos);
		return;
	}

	myRoot = yyjson_doc_get_root(myDocument);
}

JsonData::JsonData(yyjson_doc* aDocument, yyjson_val* aValue, std::filesystem::path aPath)
{
	myDocument = aDocument;
	myRoot = aValue;
	myPath = aPath;
	myOwnsDocument = false;
}

JsonData::~JsonData()
{
	if (myOwnsDocument)
	{
		yyjson_doc_free(myDocument);
	}
}

bool JsonData::IsValid() const
{
	if (myRoot == nullptr || myDocument == nullptr)
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
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return false;
	}

	if (!yyjson_is_bool(data))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not a bool, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return false;
	}

	return yyjson_get_bool(data);
}

int JsonData::GetInt(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_int(data))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not a int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return yyjson_get_int(data);
}

float JsonData::GetFloat(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not a double, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return static_cast<float>(yyjson_get_num(data));
}

std::string_view JsonData::GetString(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return "";
	}

	if (!yyjson_is_str(data))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not a string, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return "";
	}

	return yyjson_get_str(data);
}

std::vector<bool> JsonData::GetBoolArray(const char* aName) const
{
	std::vector<bool> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		LOG(JSONLog, Warning, "Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_bool(value))
		{
			values.push_back(yyjson_get_bool(value));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-bool value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<int> JsonData::GetIntArray(const char* aName) const
{
	std::vector<int> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		LOG(JSONLog, Warning, "Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_int(value))
		{
			values.push_back(yyjson_get_int(value));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-integer value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<float> JsonData::GetFloatArray(const char* aName) const
{
	std::vector<float> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		LOG(JSONLog, Warning, "Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_num(value))
		{
			values.push_back(yyjson_get_num(value));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-double value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<std::string_view> JsonData::GetStringArray(const char* aName) const
{
	std::vector<std::string_view> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		LOG(JSONLog, Warning, "Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_str(value))
		{
			values.push_back(yyjson_get_str(value));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-string value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

JsonData JsonData::GetObjectValue(const char* aName) const
{
	yyjson_val* object = yyjson_obj_get(myRoot, aName);

	if (object == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());

		return JsonData();
	}

	if (!yyjson_is_obj(object))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not an object, type: [{}] in [{}]!", aName, yyjson_get_type_desc(object), myPath.string());

		return JsonData();
	}

	return JsonData(myDocument, object, myPath);
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