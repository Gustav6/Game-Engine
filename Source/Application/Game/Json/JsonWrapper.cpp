#include "JsonWrapper.h"

#include "yyjson/yyjson.h"

#include "Logger/Logger.h"

DECLARE_LOG_CATEGORY_WITH_NAME(JSONLog, JSON, Verbose);

DEFINE_LOG_CATEGORY(JSONLog);

JsonData::JsonData()
{
	myRoot = nullptr;
	myDocument = nullptr;
}

JsonData::JsonData(const std::filesystem::path& aJsonPath)
{
	myRoot = nullptr;
	myDocument = nullptr;
	myPath = aJsonPath;

	if (!std::filesystem::exists(aJsonPath))
	{
		LOG(JSONLog, Error, "File does not exist [{}]!", aJsonPath.string());
		return;
	}

	yyjson_read_err error;

	yyjson_doc* document = yyjson_read_file(aJsonPath.string().c_str(), 0, nullptr, &error);

	if (document == nullptr)
	{
		LOG(JSONLog, Error, "Could not read json [{}]! Error [{}] : [{}] at position: [{}]!", aJsonPath.string(), error.code, error.msg, error.pos);
		return;
	}

	myDocument = std::shared_ptr<yyjson_doc>(document, yyjson_doc_free);

	myRoot = yyjson_doc_get_root(myDocument.get());
}

JsonData::JsonData(std::shared_ptr<yyjson_doc> aDocument, yyjson_val* aValue, std::filesystem::path aPath)
{
	myDocument = aDocument;
	myRoot = aValue;
	myPath = aPath;
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

unsigned int JsonData::GetUnsigned(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		LOG(JSONLog, Warning, "Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_uint(data))
	{
		LOG(JSONLog, Warning, "Key [{}] exists but is not a int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return yyjson_get_uint(data);
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

std::string JsonData::GetString(const char* aName) const
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

std::vector<unsigned int> JsonData::GetUnsignedArray(const char* aName) const
{
	std::vector<unsigned int> values;

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
		if (yyjson_is_uint(value))
		{
			values.push_back(yyjson_get_uint(value));
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
			values.push_back(static_cast<float>(yyjson_get_num(value)));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-double value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<std::string> JsonData::GetStringArray(const char* aName) const
{
	std::vector<std::string> values;

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

std::vector<JsonData> JsonData::GetObjectValueArray(const char* aName) const
{
	std::vector<JsonData> values;

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
		if (yyjson_is_obj(value))
		{
			JsonData data(myDocument, value, myPath);

			values.emplace_back(std::move(data));
		}
		else
		{
			LOG(JSONLog, Warning, "Array [{}] contains a non-object value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<std::string> JsonData::GetKeys() const
{
	std::vector<std::string> keys;

	if (!yyjson_is_obj(myRoot))
	{
		return keys;
	}

	yyjson_val* key;
	yyjson_val* value;
	size_t idx;
	size_t max;

	yyjson_obj_foreach(myRoot, idx, max, key, value)
	{
		keys.emplace_back(yyjson_get_str(key));
	}

	return keys;
}

JsonValueType JsonData::GetType(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		return JsonValueType::Unknown;
	}

	if (yyjson_is_bool(data))
	{
		return JsonValueType::Bool;
	}
	if (yyjson_is_int(data))
	{
		return JsonValueType::Int;
	}
	if (yyjson_is_uint(data))
	{
		return JsonValueType::Unsigned;
	}
	if (yyjson_is_num(data))
	{
		return JsonValueType::Float;
	}
	if (yyjson_is_str(data))
	{
		return JsonValueType::String;
	}
	if (yyjson_is_arr(data))
	{
		return JsonValueType::Array;
	}
	if (yyjson_is_obj(data))
	{
		return JsonValueType::Object;
	}

	return JsonValueType::Unknown;
}

JsonValueType JsonData::GetArrayType(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr || !yyjson_is_arr(data))
	{
		return JsonValueType::Unknown;
	}

	yyjson_val* value = yyjson_arr_get_first(data);

	if (value == nullptr)
	{
		return JsonValueType::Unknown;
	}

	if (yyjson_is_bool(value))
	{
		return JsonValueType::Bool;
	}
	if (yyjson_is_int(value))
	{
		return JsonValueType::Int;
	}
	if (yyjson_is_uint(value))
	{
		return JsonValueType::Unsigned;
	}
	if (yyjson_is_num(value))
	{
		return JsonValueType::Float;
	}
	if (yyjson_is_str(value))
	{
		return JsonValueType::String;
	}
	if (yyjson_is_arr(value))
	{
		return JsonValueType::Array;
	}
	if (yyjson_is_obj(value))
	{
		return JsonValueType::Object;
	}

	return JsonValueType::Unknown;
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