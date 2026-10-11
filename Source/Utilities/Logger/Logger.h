#pragma once

/*
 * Basic Logger by Daniel Borgshamar @ TGA
 * ---------------------------------------
 * Handles logging of messages to stderr and a log file with filtering.
 * Syntax inspired by UE's UE_LOG macros.
 *
 * Usage:
 * In a useful Header file (such as the PCH header file of your module) use:
 * DECLARE_LOG_CATEGORY(MyCategoryId, DesiredLogVerbosity)
 * to specify your logging category. Alternatively you can also use:
 * DECLARE_LOG_CATEGORY_WITH_NAME(MyCategoryId, MyCategoryName, DesiredLogVerbosity)
 * to have a label in front of logging messages, useful for showing which module the logging came from.
 *
 * In a handy CPP File (such as the PCH source file of your module) use:
 * DEFINE_LOG_CATEGORY(MyCategoryId);
 * to generate the actual implementation of your log category and then you are ready to go.
 *
 * You can then log with:
 * LOG(MyCategoryId, MessageVerbosity, L"My Message {}", L"A String);
 * to log a message with optional argument.
 *
 * Log Verbosity controls which messages are shown. To enable easy filtering you can use:
 *
 * #if _DEBUG
 * DECLARE_LOG_CATEGORY_WITH_NAME(LogMuninCore, MuninCore, LogVerbosity::Verbose);
 * #else
 * DECLARE_LOG_CATEGORY_WITH_NAME(LogMuninCore, MuninCore, LogVerbosity::Warning);
 * #endif
 *
 * This means that any log messages with Verbose or higher priority will be logged in Debug
 * while only messages with Warning or higher priority will be logged in Release.
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <format>
#include <vector>
#include <unordered_set>

// Define this before including Logger.h to override.
#ifndef LOGGING_PATH
#define LOGGING_PATH L""
#endif

#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif

namespace LogVerbosity
{
	enum Type : uint8_t
	{
		None = 0,
		Fatal,
		Error,
		Warning,
		Log,
		Verbose,		
	};
}

class Logger
{
public:
	struct LogCategoryBase
	{
		std::string Name;
		LogVerbosity::Type Verbosity = LogVerbosity::Warning;
		unsigned Id;

		bool operator==(const LogCategoryBase& aOther) const { return Id == aOther.Id; }

		virtual ~LogCategoryBase();

	protected:
		LogCategoryBase(std::string aName, LogVerbosity::Type aVerbosity);
		
		static unsigned ourNextId;
	};

	template<uint8_t V>
	struct LogCategory : public LogCategoryBase
	{
		__forceinline LogCategory(const std::string& aName)
			: LogCategoryBase(aName, static_cast<LogVerbosity::Type>(V))
		{  }
	};

	struct LogCategoryHash
	{
		size_t operator()(const Logger::LogCategoryBase& x) const noexcept
		{
			return static_cast<size_t>(x.Id);
		}
	};

	class Group
	{
	public:
		template<typename... Args>
		explicit Group(const LogCategoryBase& aCategory, LogVerbosity::Type aVerbosity, const char* aMessage, Args... args)
		{
			if (aCategory.Verbosity >= aVerbosity)
			{
				const std::string s = std::vformat(aMessage, std::make_format_args(args...));
				GroupHeader(aCategory, aVerbosity, s.c_str());
			}

			ourDepth++;
		}

		~Group()
		{
			ourDepth--;

			if (ourDepth == 0)
			{
				QueueGroup();
			}
		}

		Group(const Group&) = delete;
		Group& operator=(const Group&) = delete;

	private:
		static void GroupHeader(const LogCategoryBase& aCategory, LogVerbosity::Type aVerbosity, const char* aMessage);
	};

private:
	Logger();
	~Logger();

	std::string Timestamp(bool aIncludeDate = false) const;

	void LogIntl(const LogCategoryBase& aCategory, LogVerbosity::Type aVerbosity, const char* aMessage);
	void WorkerThread();

	static Logger& Get()
	{
		static Logger thisLogger;
		return thisLogger;
	}

	static void Shutdown()
	{
		Logger& thisLogger = Get();
		thisLogger.myIsRunning = false;
		thisLogger.myLogThread.join();
	}

	struct LogStream
	{
		std::ofstream File;

		template<typename T>
		LogStream& operator<<(const T& aValue)
		{
			std::cerr << aValue;
			File << aValue;
			return *this;
		}

		typedef LogStream& (*MyStreamManipulator)(LogStream&);
	    LogStream& operator<<(MyStreamManipulator manip)
	    {
	        return manip(*this);
	    }

	    static LogStream& endl(LogStream& stream)
	    {
	        // print a new line
	        std::cerr << '\n';
			std::cerr.flush();
			stream.File << '\n';
			stream.File.flush();
	        return stream;
	    }

		~LogStream();
	};

	// Group Methods
	static void QueueGroup();
	static std::string FormatLog(unsigned aDepth, bool aIsLast, const std::vector<bool>& aGroupContinues);

	struct LogEntry
	{
		const LogCategoryBase* Category;
		std::string Message;
		unsigned Depth;
		LogVerbosity::Type Verbosity;
	};

	LogStream myStream;
	std::mutex myQueueMutex;
	std::unordered_set<LogCategoryBase, LogCategoryHash> myFilter;
	std::condition_variable myQueueCV;
	std::filesystem::path myLogFilePath;
	std::queue<LogEntry> myLogQueue;

	std::thread myLogThread;
	void* myStdErrHandle;
	std::atomic_bool myIsRunning;

	static inline unsigned ourDepth = 0;
	static inline std::vector<LogEntry> ourPendingLogs;

public:
	template<typename... Args>
	static void Log(const LogCategoryBase& aCategory, LogVerbosity::Type aVerbosity, const char* aMessage, Args... args)
	{
		if(aCategory.Verbosity >= aVerbosity)
		{
			const std::string s = std::vformat(aMessage, std::make_format_args(args...));
			Log(aCategory, aVerbosity, s.c_str());
		}
	}

	static void Log(const LogCategoryBase& aCategory, LogVerbosity::Type aVerbosity, const char* aMessage);

	static void Flush();

	static void AddFilter(const LogCategoryBase& aCategory);
	static void RemoveFilter(const LogCategoryBase& aCategory);
	static void ClearFilters();
};

#define DECLARE_LOG_CATEGORY(CategoryId, DefaultVerbosity) extern struct LoggerCategory##CategoryId : public Logger::LogCategory<LogVerbosity::DefaultVerbosity> \
	{ \
		FORCEINLINE LoggerCategory##CategoryId() : Logger::LogCategory<LogVerbosity::DefaultVerbosity>(#CategoryId) {} \
	} CategoryId;

#define DECLARE_LOG_CATEGORY_WITH_NAME(CategoryId, CategoryName, DefaultVerbosity) extern struct LoggerCategory##CategoryId : public Logger::LogCategory<LogVerbosity::DefaultVerbosity> \
	{ \
		FORCEINLINE LoggerCategory##CategoryId() : Logger::LogCategory<LogVerbosity::DefaultVerbosity>(#CategoryName) {} \
	} CategoryId;

#define DEFINE_LOG_CATEGORY(CategoryName) LoggerCategory##CategoryName CategoryName
#define INT_LOG_MESSAGE()
#define LOG(Category, Verbosity, Message, ...) Logger::Log(Category, LogVerbosity::Verbosity, Message, ##__VA_ARGS__)

// Group Macros

#define LOGGER_CONCAT_INNER(a, b) a##b
#define LOGGER_CONCAT(a, b) LOGGER_CONCAT_INNER(a, b)

#define LOG_GROUP(Category, Verbosity, Message, ...) \
	Logger::Group LOGGER_CONCAT(logGroup_, __COUNTER__)(Category, LogVerbosity::Verbosity, Message, ##__VA_ARGS__)

