#ifndef STUDENTUNDO_H_
#define STUDENTUNDO_H_

#include "Undo.h"
#include <stack>
#include <list>
#include <string>
class StudentUndo : public Undo {
public:

	void submit(Action action, int row, int col, char ch = 0);
	Action get(int& row, int& col, int& count, std::string& text);
	void clear();

private:
    struct undoAction{
        Action action;
        int row;
        int col;
        std::string chars;
    };
    
    std::stack<undoAction*> undoStack;
};

#endif // STUDENTUNDO_H_
