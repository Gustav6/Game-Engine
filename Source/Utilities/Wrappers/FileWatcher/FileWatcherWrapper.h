#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

// More info can be found at their repo:
// https://github.com/SpartanJ/efsw

namespace FileWatcher
{
	using WatcherID = long;

	constexpr WatcherID InvalidWatchID = -1;

	enum class FileAction
	{
		None,
		Added,
		Deleted,
		Modified,
		Moved
	};

	struct FileEvent
	{
		FileAction Action = FileAction::None;
		std::filesystem::path Directory;
		std::filesystem::path FileName;
		std::filesystem::path OldFileName; // Only set when Action == Moved
	};

	using Callback = std::function<void(const FileEvent& aEvent)>;

	void Initialize();
	void Shutdown();

	WatcherID AddWatcher(const std::filesystem::path& aDirectory, Callback aCallback);
	void RemoveWatcher(WatcherID aID);
	void RemoveWatcher(const std::filesystem::path& aDirectory);
	void RemoveAllWatchers();

	void DispatchEvents();
}