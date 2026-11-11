#include "Tools/Tools.pch.h"
#include "CommandHistory.h"

void CommandHistory::Execute(std::unique_ptr<ICommand> aCommand)
{
	if (aCommand == nullptr)
	{
		SPDLOG_ERROR("Attempted to execute a null command.");
		return;
	}

	aCommand->Execute();
	myRedoStack.clear();
	myUndoStack.push_back(std::move(aCommand));
}

bool CommandHistory::Undo()
{
	if (!myUndoStack.empty())
	{
		auto command = std::move(myUndoStack.back());
		myUndoStack.pop_back();
		command->Undo();
		myRedoStack.push_back(std::move(command));
		return true;
	}

	return false;
}

bool CommandHistory::Redo()
{
	if (!myRedoStack.empty())
	{
		auto command = std::move(myRedoStack.back());
		myRedoStack.pop_back();
		command->Execute();
		myUndoStack.push_back(std::move(command));
		return true;
	}

	return false;
}

void CommandHistory::Clear()
{
	myUndoStack.clear();
	myRedoStack.clear();
}
