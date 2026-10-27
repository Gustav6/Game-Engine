#include "JsonWrapper.h"

#include "yyjson/yyjson.h"

#include "Wrappers/Logger/LoggerWrapper.h"

Json::Data::Data()
{
	myRoot = nullptr;
	myDocument = nullptr;
}

Json::Data::Data(const std::filesystem::path& aJsonPath, ReadFlags aFlags)
{
	myRoot = nullptr;
	myDocument = nullptr;
	myPath = aJsonPath;

	if (!std::filesystem::exists(aJsonPath))
	{
		SPDLOG_ERROR("File does not exist [{}]!", aJsonPath.string());
		return;
	}

	yyjson_read_err error;

	yyjson_doc* document = yyjson_read_file(aJsonPath.string().c_str(), aFlags, nullptr, &error);

	if (error.code)
	{
		SPDLOG_ERROR("Could not read json [{}]! Error [{}] : [{}] at position: [{}]!", aJsonPath.string(), error.code, error.msg, error.pos);
		return;
	}

	myDocument = std::shared_ptr<yyjson_doc>(document, yyjson_doc_free);

	myRoot = yyjson_doc_get_root(myDocument.get());
}

Json::Data::Data(const std::shared_ptr<yyjson_doc>& aDocument, yyjson_val* aValue, const std::filesystem::path& aPath)
{
	myDocument = aDocument;
	myRoot = aValue;
	myPath = aPath;
}

bool Json::Data::IsValid() const
{
	if (myRoot == nullptr || myDocument == nullptr)
	{
		return false;
	}

	return true;
}

bool Json::Data::GetBool(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return false;
	}

	if (!yyjson_is_bool(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not a bool, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return false;
	}

	return yyjson_get_bool(data);
}

int Json::Data::GetInt(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_int(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not a int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return yyjson_get_int(data);
}

unsigned int Json::Data::GetUnsigned(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0;
	}

	if (!yyjson_is_uint(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not an unsigned int, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0;
	}

	return static_cast<unsigned int>(yyjson_get_uint(data));
}

float Json::Data::GetFloat(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not a float / double, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return static_cast<float>(yyjson_get_num(data));
}

double Json::Data::GetDouble(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return 0.0f;
	}

	if (!yyjson_is_num(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not a double, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return 0.0f;
	}

	return yyjson_get_num(data);
}

std::string Json::Data::GetString(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return "";
	}

	if (!yyjson_is_str(data))
	{
		SPDLOG_WARN("Key [{}] exists but is not a string, type: [{}] in [{}]!", aName, yyjson_get_type_desc(data), myPath.string());
		return "";
	}

	return yyjson_get_str(data);
}

Json::Data Json::Data::GetDataObject(const char* aName) const
{
	yyjson_val* object = yyjson_obj_get(myRoot, aName);

	if (object == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());

		return Data();
	}

	if (!yyjson_is_obj(object))
	{
		SPDLOG_WARN("Key [{}] exists but is not an object, type: [{}] in [{}]!", aName, yyjson_get_type_desc(object), myPath.string());

		return Data();
	}

	return Data(myDocument, object, myPath);
}

std::vector<bool> Json::Data::GetBoolArray(const char* aName) const
{
	std::vector<bool> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-bool value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<int> Json::Data::GetIntArray(const char* aName) const
{
	std::vector<int> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-integer value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<unsigned int> Json::Data::GetUnsignedArray(const char* aName) const
{
	std::vector<unsigned int> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-unsigned-integer value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<float> Json::Data::GetFloatArray(const char* aName) const
{
	std::vector<float> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-float / double value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<double> Json::Data::GetDoubleArray(const char* aName) const
{
	std::vector<double> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-double value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<std::string> Json::Data::GetStringArray(const char* aName) const
{
	std::vector<std::string> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
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
			SPDLOG_WARN("Array [{}] contains a non-string value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<Json::Data> Json::Data::GetDataObjectArray(const char* aName) const
{
	std::vector<Data> values;

	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		SPDLOG_WARN("Key [{}] not found in [{}]!", aName, myPath.string());
		return values;
	}

	if (!yyjson_is_arr(data))
	{
		SPDLOG_WARN("Key [{}] is not an array in [{}]!", aName, myPath.string());
		return values;
	}

	yyjson_val* value;
	size_t idx, max;

	yyjson_arr_foreach(data, idx, max, value)
	{
		if (yyjson_is_obj(value))
		{
			Data objectData(myDocument, value, myPath);

			values.emplace_back(std::move(objectData));
		}
		else
		{
			SPDLOG_WARN("Array [{}] contains a non-object value in [{}]!", aName, myPath.string());
		}
	}

	return values;
}

std::vector<std::string> Json::Data::GetKeys() const
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

Json::ValueType Json::Data::GetType(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr)
	{
		return Json::ValueType::Unknown;
	}

	if (yyjson_is_bool(data))
	{
		return Json::ValueType::Bool;
	}
	if (yyjson_is_int(data))
	{
		return Json::ValueType::Int;
	}
	if (yyjson_is_uint(data))
	{
		return Json::ValueType::Unsigned;
	}
	if (yyjson_is_num(data))
	{
		return Json::ValueType::Float;
	}
	if (yyjson_is_str(data))
	{
		return Json::ValueType::String;
	}
	if (yyjson_is_arr(data))
	{
		return Json::ValueType::Array;
	}
	if (yyjson_is_obj(data))
	{
		return Json::ValueType::Object;
	}

	return Json::ValueType::Unknown;
}

Json::ValueType Json::Data::GetArrayType(const char* aName) const
{
	yyjson_val* data = yyjson_obj_get(myRoot, aName);

	if (data == nullptr || !yyjson_is_arr(data))
	{
		return Json::ValueType::Unknown;
	}

	yyjson_val* value = yyjson_arr_get_first(data);

	if (value == nullptr)
	{
		return Json::ValueType::Unknown;
	}

	if (yyjson_is_bool(value))
	{
		return Json::ValueType::Bool;
	}
	if (yyjson_is_int(value))
	{
		return Json::ValueType::Int;
	}
	if (yyjson_is_uint(value))
	{
		return Json::ValueType::Unsigned;
	}
	if (yyjson_is_num(value))
	{
		return Json::ValueType::Float;
	}
	if (yyjson_is_str(value))
	{
		return Json::ValueType::String;
	}
	if (yyjson_is_arr(value))
	{
		return Json::ValueType::Array;
	}
	if (yyjson_is_obj(value))
	{
		return Json::ValueType::Object;
	}

	return Json::ValueType::Unknown;
}

bool Json::Data::HasKey(const char* aName) const
{
	if (yyjson_obj_get(myRoot, aName) != nullptr)
	{
		return true;
	}

	return false;
}

const std::filesystem::path& Json::Data::GetPath() const
{
	return myPath;
}

Json::Writer::Writer(const std::filesystem::path& aDestination, WriteFlags aFlags)
{
	myDestination = aDestination;
	myWriteFlags = aFlags;

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
			SPDLOG_ERROR("Could not create root object for json [{}]!", myDestination.string());
		}
	}
	else
	{
		SPDLOG_ERROR("Could not create json document for [{}]!", myDestination.string());
	}
}

Json::Writer::~Writer()
{
	yyjson_mut_doc_free(myDocument);
}

bool Json::Writer::IsValid() const
{
	if (myRoot == nullptr || myDocument == nullptr)
	{
		return false;
	}

	return true;
}

void Json::Writer::Save() const
{
	if (myDocument == nullptr)
	{
		SPDLOG_ERROR("Cannot save json [{}], document is invalid!", myDestination.string());
		return;
	}

	yyjson_mut_val* object = yyjson_mut_doc_get_root(myDocument);

	yyjson_mut_obj_iter iter;
	yyjson_mut_obj_iter_init(object, &iter);
	yyjson_mut_val* key;
	while ((key = yyjson_mut_obj_iter_next(&iter)))
	{
		yyjson_mut_val* val = yyjson_mut_obj_iter_get_val(key);

		if (yyjson_mut_is_null(val))
		{
			SPDLOG_WARN("Removing null value for key [{}] when saving [{}]!", yyjson_mut_get_str(key), myDestination.string());
			yyjson_mut_obj_iter_remove(&iter);
		}
	}

	yyjson_write_err error;

	if (!yyjson_mut_write_file(myDestination.string().c_str(), myDocument, myWriteFlags, nullptr, &error))
	{
		if (error.code)
		{
			SPDLOG_ERROR("Could not write json [{}]! Error [{}] : [{}]!", myDestination.string(), error.code, error.msg);
		}
	}
}

void Json::Writer::Clear()
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
			SPDLOG_ERROR("Could not create root object for json [{}]!", myDestination.string());
		}
	}
	else
	{
		SPDLOG_ERROR("Could not create json document for [{}]!", myDestination.string());
	}
}

//bool Json::Writer::BeginObject(const char* aKey)
//{
//	return true;
//}
//
//bool Json::Writer::EndObject()
//{
//	return true;
//}

bool Json::Writer::SetBool(const char* aKey, bool aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set Bool for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_bool(myDocument, aValue);

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate Bool for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set Bool for [{}]!", myDestination.string());
	return false;
}

bool Json::Writer::SetInt(const char* aKey, int aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set Int for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_int(myDocument, aValue);

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate Int for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set Int for [{}]!", myDestination.string());
	return false;
}

bool Json::Writer::SetUnsigned(const char* aKey, unsigned int aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set Unsigned Int for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_uint(myDocument, aValue);

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate Unsigned Int for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set Unsigned Int for [{}]!", myDestination.string());
	return false;
}

bool Json::Writer::SetFloat(const char* aKey, float aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set Float for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_float(myDocument, aValue);

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate Float for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set Float to [{}]!", myDestination.string());
	return false;
}

bool Json::Writer::SetDouble(const char* aKey, double aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set Double for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_double(myDocument, aValue);

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate Double for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set Double to [{}]!", myDestination.string());
	return false;
}

bool Json::Writer::SetString(const char* aKey, const std::string& aValue) const
{
	if (!IsValid())
	{
		SPDLOG_ERROR("Could not set String for [{}], Writer is not valid!", myDestination.string());
		return false;
	}

	yyjson_mut_val* key = yyjson_mut_str(myDocument, aKey);
	yyjson_mut_val* value = yyjson_mut_strncpy(myDocument, aValue.data(), aValue.size());

	if (key == nullptr || value == nullptr)
	{
		SPDLOG_ERROR("Could not allocate String for [{}]!", myDestination.string());
		return false;
	}

	if (yyjson_mut_obj_put(myRoot, key, value))
	{
		return true;
	}

	SPDLOG_ERROR("Could not set String to [{}]!", myDestination.string());
	return false;
}

const std::filesystem::path& Json::Writer::GetDestination() const
{
	return myDestination;
}

Json::Data Json::Read(const std::filesystem::path& aJsonPath, ReadFlags aFlags)
{
	return { aJsonPath, aFlags };
}

Json::Writer Json::Write(const std::filesystem::path& aJsonDestination, WriteFlags aFlags)
{
	return { aJsonDestination, aFlags };
}