#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <stack>

// More info can be found at their repo:
// https://github.com/ibireme/yyjson 

struct yyjson_doc;
struct yyjson_val;
struct yyjson_mut_doc;
struct yyjson_mut_val;

namespace Json
{
	enum class ValueType
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

	enum ReadFlag_
	{
		ReadFlag_None = 0,
		ReadFlag_Insitu = 1 << 0,
		ReadFlag_StopWhenDone = 1 << 1,
		ReadFlag_AllowTrailingCommas = 1 << 2,
		ReadFlag_AllowComments = 1 << 3,
		ReadFlag_AllowInfAndNan = 1 << 4,
		ReadFlag_NumberAsRaw = 1 << 5,
		ReadFlag_AllowInvalidUnicode = 1 << 6,
		ReadFlag_BignumAsRaw = 1 << 7,
		ReadFlag_AllowBom = 1 << 8,
		ReadFlag_AllowExtNumber = 1 << 9,
		ReadFlag_AllowExtEscape = 1 << 10,
		ReadFlag_AllowExtWhitespace = 1 << 11,
		ReadFlag_AllowSingleQuotedStr = 1 << 12,
		ReadFlag_AllowUnquotedKey = 1 << 13
	};

	typedef std::uint32_t ReadFlags;

	class Data
	{
	public:
		Data(const std::filesystem::path& aJsonPath, ReadFlags aFlags);

		bool IsValid() const;

		bool GetBool(const char* aName) const;
		int GetInt(const char* aName) const;
		unsigned int GetUnsigned(const char* aName) const;
		float GetFloat(const char* aName) const;
		double GetDouble(const char* aName) const;
		std::string GetString(const char* aName) const;
		Data GetDataObject(const char* aName) const;

		std::vector<bool> GetBoolArray(const char* aName) const;
		std::vector<int> GetIntArray(const char* aName) const;
		std::vector<unsigned int> GetUnsignedArray(const char* aName) const;
		std::vector<float> GetFloatArray(const char* aName) const;
		std::vector<double> GetDoubleArray(const char* aName) const;
		std::vector<std::string> GetStringArray(const char* aName) const;
		std::vector<Data> GetDataObjectArray(const char* aName) const;

		[[nodiscard]] std::vector<std::string> GetKeys() const;

		ValueType GetType(const char* aName) const;

		/*
		* Note the returned type only reflects the first element in the array.
		* @return
		*/
		ValueType GetArrayType(const char* aName) const;

		bool HasKey(const char* aName) const;

		const std::filesystem::path& GetPath() const;

	private:
		Data();
		Data(const std::shared_ptr<yyjson_doc>& aDocument, yyjson_val* aValue, const std::filesystem::path& aPath);

		std::shared_ptr<yyjson_doc> myDocument;
		yyjson_val* myRoot;

		std::filesystem::path myPath;
	};

	/*
	* The writer class represents a wrapper to write to a json file.
	* @return
	*/

	enum WriteFlag_
	{
		WriteFlag_None = 0,
		WriteFlag_Pretty = 1 << 0,
		WriteFlag_PrettyTwoSpaces = 1 << 1,
		WriteFlag_PrettyTabs = 1 << 2,
		WriteFlag_EscapeUnicode = 1 << 3,
		WriteFlag_LowercaseHex = 1 << 4,
		WriteFlag_EscapeSlashes = 1 << 5,
		WriteFlag_AllowInfAndNan = 1 << 6,
		WriteFlag_InfAndNanAsNull = 1 << 7,
		WriteFlag_AllowInvalidUnicode = 1 << 8,
		WriteFlag_NewlineAtEnd = 1 << 9,
		WriteFlag_FpToFloat = 1 << 10
	};

	typedef std::uint32_t WriteFlags;

	class Writer
	{
	public:
		Writer(const std::filesystem::path& aDestination, WriteFlags aFlags);
		~Writer();

		bool IsValid() const;

		void Save() const;
		void Clear();

		bool BeginObject(const char* aKey);
		bool EndObject();

		bool SetBool(const char* aKey, bool aValue) const;
		bool SetInt(const char* aKey, int aValue) const;
		bool SetUnsigned(const char* aKey, unsigned int aValue) const;
		bool SetFloat(const char* aKey, float aValue) const;
		bool SetDouble(const char* aKey, double aValue) const;
		bool SetString(const char* aKey, const std::string& aValue) const;

		// TODO: Implement
		bool SetBoolArray(const char* aKey, const std::vector<bool>& aValues) const;
		bool SetIntArray(const char* aKey, const std::vector<int>& aValues) const;
		bool SetUnsignedArray(const char* aKey, const std::vector<unsigned int>& aValues) const;
		bool SetFloatArray(const char* aKey, const std::vector<float>& aValues) const;
		bool SetDoubleArray(const char* aKey, const std::vector<double>& aValues) const;
		bool SetStringArray(const char* aKey, const std::vector<std::string>& aValues) const;

		const std::filesystem::path& GetDestination() const;

	private:
		yyjson_mut_val* GetCurrentObject() const;

		yyjson_mut_doc* myDocument;
		yyjson_mut_val* myRoot;

		std::stack<yyjson_mut_val*> myObjectStack;

		std::filesystem::path myDestination;
		WriteFlags myWriteFlags;
	};

	Data Read(const std::filesystem::path& aJsonPath, ReadFlags aFlags = ReadFlag_None);
	Writer Write(const std::filesystem::path& aJsonDestination, WriteFlags aFlags = WriteFlag_None);
}
