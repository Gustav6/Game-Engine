#pragma once

#include "Interface/ICommand.h"

class CommandHistory
{
public:
    void Execute(std::unique_ptr<ICommand> aCommand);
    bool Undo();
    bool Redo();
    void Clear();

private:
	std::vector<std::unique_ptr<ICommand>> myUndoStack;
	std::vector<std::unique_ptr<ICommand>> myRedoStack;
};