#include "FileWatcherWrapper.h"

#include <memory>

#include "efsw/efsw.hpp"

namespace FileWatcher
{
	std::unique_ptr<efsw::FileWatcher> Watcher;
}

void FileWatcher::Initialize()
{
	if (Watcher != nullptr)
	{
		return;
	}

	Watcher = std::make_unique<efsw::FileWatcher>();
}

void FileWatcher::Shutdown()
{
	if (Watcher == nullptr)
	{
		return;
	}

	RemoveAllWatchers();
	
	Watcher.reset();
}

//FileWatcher::WatcherID FileWatcher::AddWatcher(const std::filesystem::path& aDirectory, Callback aCallback)
//{
//}

void FileWatcher::RemoveWatcher(WatcherID aID)
{
	Watcher->removeWatch(aID);
}

void FileWatcher::RemoveWatcher(const std::filesystem::path& aDirectory)
{
	Watcher->removeWatch(aDirectory.string());
}

void FileWatcher::RemoveAllWatchers()
{
	// Won't matter if it's slow since it will only be called once

	for (const auto& directory : Watcher->directories())
	{
		Watcher->removeWatch(directory);
	}
}

void FileWatcher::DispatchEvents()
{
}
