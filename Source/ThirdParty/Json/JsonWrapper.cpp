#include "JsonWrapper.h"

#include "yyjson/yyjson.h"

#include "Logger/Logger.h"

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
		spdlog::error("File does not exist [{}]!", aJsonPath.string());
		return;
	}

	yyjson_read_err error;

	yyjson_doc* document = yyjson_read_file(aJsonPath.string().c_str(), 0, nullptr, &error);

	if (error.code)
	{
		spdlog::error("Could not read json [{}]! Error [{}] : [{}] at position: [{}]!", aJsonPath.string(), error.code, error.msg, error.pos);
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return false;
	}

	if (!yyjson_is_bool(data))
	{
		spdlog::warn("Key [{}] exists but is not a bool, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return false;
	}

	return yyjson_get_bool(data);
}

int JsonData::GetInt(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_int(data))
	{
		spdlog::warn("Key [{}] exists but is not a int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return yyjson_get_int(data);
}

unsigned int JsonData::GetUnsigned(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_uint(data))
	{
		spdlog::warn("Key [{}] exists but is not an unsigned int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return static_cast<unsigned int>(yyjson_get_uint(data));
}

float JsonData::GetFloat(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		spdlog::warn("Key [{}] exists but is not a float / double, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return static_cast<float>(yyjson_get_num(data));
}

double JsonData::GetDouble(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		spdlog::warn("Key [{}] exists but is not a double, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return yyjson_get_num(data);
}

std::string JsonData::GetString(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return "";
	}

	if (!yyjson_is_str(data))
	{
		spdlog::warn("Key [{}] exists but is not a string, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return "";
	}

	return yyjson_get_str(data);
}

JsonData JsonData::GetObjectValue(const char* aName) const
{
	yyjson_val* object = yyjson_obj_get(myRoot, aName);

	if (object == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());

		return JsonData();
	}

	if (!yyjson_is_obj(object))
	{
		spdlog::warn("Key [{}] exists but is not an object, type: [{}] in [{}]!", aName, yyjson_get_type_desc(object), myPath.string());

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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			spdlog::warn("Array [{}] contains a non-bool value in [{}]!", aName, myPath.string());
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			spdlog::warn("Array [{}] contains a non-integer value in [{}]!", aName, myPath.string());
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_uint(value))
		{
			values.push_back(static_cast<unsigned int>(yyjson_get_uint(value)));
		}
		else
		{
			spdlog::warn("Array [{}] contains a non-unsigned-integer value in [{}]!", aName, myPath.string());
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			spdlog::warn("Array [{}] contains a non-float / double value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<double> JsonData::GetDoubleArray(const char* aName) const
{
	std::vector<double> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			spdlog::warn("Array [{}] contains a non-double value in [{}]!", aName, myPath.string());
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			spdlog::warn("Array [{}] contains a non-string value in [{}]!", aName, myPath.string());
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
		spdlog::warn("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		spdlog::warn("Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_obj(value))
		{
			JsonData objectData(myDocument, value, myPath);

			values.emplace_back(std::move(objectData));
		}
		else
		{
			spdlog::warn("Array [{}] contains a non-object value in [{}]!", aName, myPath.string());
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

const std::filesystem::path& JsonData::GetPath() const
{
	return myPath;
}

JsonWriter::JsonWriter(const std::filesystem::path& aDestination)
{
	myDestination = aDestination;

	myDocument = yyjson_mut_doc_new(NULL);

	if (myDocument != nullptr)
	{
		myRoot = yyjson_mut_obj(myDocument);

		if (myRoot != nullptr)
		{
			yyjson_mut_doc_set_root(myDocument, myRoot);
		}
		else
		{
			spdlog::error("Could not create root object for json [{}]!", myDestination.string());
		}
	}
	else
	{
		spdlog::error("Could not create json document for [{}]!", myDestination.string());
	}
}

JsonWriter::~JsonWriter()
{
	yyjson_mut_doc_free(myDocument);
}

bool JsonWriter::IsValid() const
{
	if (myRoot == nullptr || myDocument == nullptr)
	{
		return false;
	}

	return true;
}

void JsonWriter::Save()
{
	if (myDocument == nullptr)
	{
		spdlog::error("Cannot save json [{}], document is invalid!", myDestination.string());
		return;
	}

	yyjson_mut_val* object = yyjson_mut_doc_get_root(myDocument);

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(object, &iter);
	yyjson_mut_val* key, * val;
	while ((key = yyjson_mut_obj_iter_next(&iter)))
	{
		val = yyjson_mut_obj_iter_get_val(key);

		if (yyjson_mut_is_null(val))
		{
			spdlog::warn("Removing null value for key [{}] when saving [{}]!", yyjson_mut_get_str(key), myDestination.string());
			yyjson_mut_obj_iter_remove(&iter);
		}
	}

	yyjson_write_err error;

	if (!yyjson_mut_write_file(myDestination.string().c_str(), myDocument, YYJSON_WRITE_NOFLAG, NULL, &error))
	{
		if (error.code)
		{
			spdlog::error("Could not write json [{}]! Error [{}] : [{}]!", myDestination.string(), error.code, error.msg);
		}
	}
}

void JsonWriter::Clear()
{
	if (myDocument != nullptr)
	{
		yyjson_mut_doc_free(myDocument);
	}

	myDocument = yyjson_mut_doc_new(nullptr);

	if (myDocument != nullptr)
	{
		myRoot = yyjson_mut_obj(myDocument);

		if (myRoot != nullptr)
		{
			yyjson_mut_doc_set_root(myDocument, myRoot);
		}
		else
		{
			spdlog::error("Could not create root object for json [{}]!", myDestination.string());
		}
	}
	else
	{
		spdlog::error("Could not create json document for [{}]!", myDestination.string());
	}
}

bool JsonWriter::AddBool(const char* aKey, bool aValue) const
{
	return yyjson_mut_obj_add_bool(myDocument, myRoot, aKey, aValue);
}

bool JsonWriter::AddInt(const char* aKey, int aValue) const
{
	return yyjson_mut_obj_add_int(myDocument, myRoot, aKey, aValue);
}

bool JsonWriter::AddUnsigned(const char* aKey, unsigned int aValue) const
{
	return yyjson_mut_obj_add_uint(myDocument, myRoot, aKey, aValue);
}

bool JsonWriter::AddFloat(const char* aKey, float aValue) const
{
	return yyjson_mut_obj_add_float(myDocument, myRoot, aKey, aValue);
}

bool JsonWriter::AddDouble(const char* aKey, double aValue) const
{
	return yyjson_mut_obj_add_double(myDocument, myRoot, aKey, aValue);
}

bool JsonWriter::AddString(const char* aKey, const std::string& aValue) const
{
	return yyjson_mut_obj_add_str(myDocument, myRoot, aKey, aValue.c_str());
}

const std::filesystem::path& JsonWriter::GetDestination() const
{
	return myDestination;
}

JsonData Json::Read(const std::filesystem::path& aJsonPath)
{
	return { aJsonPath };
}

JsonWriter Json::Write(const std::filesystem::path& aJsonDestination)
{
	return { aJsonDestination };
}