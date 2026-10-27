#include "FileWatcherWrapper.h"

#include "efsw.hpp"

namespace FileWatcher
{
	std::unique_ptr<efsw::FileWatcher> ourWatcher;
}

void FileWatcher::Initialize()
{
	if (ourWatcher != nullptr)
	{
		return;
	}

	ourWatcher = std::make_unique<efsw::FileWatcher>();
}

void FileWatcher::Shutdown()
{
	if (ourWatcher != nullptr)
	{
		return;
	}

	RemoveAllWatchers();	
	
	ourWatcher.reset();
}

FileWatcher::WatcherID FileWatcher::AddWatcher(const std::filesystem::path& aDirectory, Callback aCallback)
{
}

void FileWatcher::RemoveWatcher(WatcherID aID)
{
}

void FileWatcher::RemoveWatcher(const std::filesystem::path& aDirectory)
{
}

void FileWatcher::RemoveAllWatchers()
{
}

void FileWatcher::DispatchEvents()
{
}
