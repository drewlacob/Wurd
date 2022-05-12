#include "StudentUndo.h"

Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) {
    //make the new undoAction
    undoAction* latestAction = new undoAction;
    //invert the action (what needs to be done to undo the actual action)
    if (action == INSERT)
        latestAction->action = DELETE;
    if (action == DELETE)
        latestAction->action = INSERT;
    if (action == JOIN)
        latestAction->action = SPLIT;
    if (action == SPLIT)
        latestAction->action = JOIN;
    
    //set data for undoAction
    latestAction->row = row;
    latestAction->col = col;
    latestAction->chars = ch;
    
    //if possible batch together insertion, backspace, and delete
    if (!undoStack.empty()){
        undoAction* topAction = undoStack.top();
        
        //batch together deletion of inserted characters
        if(latestAction->action == DELETE && topAction->action == DELETE){
            if (latestAction->col == topAction->col+1){
                undoStack.top()->chars += latestAction->chars;
                undoStack.top()->col++;
                return;
            }
        }
        
        if(latestAction->action == INSERT && topAction->action == INSERT){
            
            //batch together reinsertion of deleted characters
            if (latestAction->col == topAction->col){
                undoStack.top()->chars += latestAction->chars;
                return;
            }
            //batch together reinsertion of backspaced characters
            if (latestAction->col == topAction->col-1){
                undoStack.top()->chars = latestAction->chars + undoStack.top()->chars;
                undoStack.top()->col--;
                return;
            }
        }
        
    }
    //if this point is reached, no batching possible so simply add to the stack
    undoStack.push(latestAction);
}

StudentUndo::Action StudentUndo::get(int& row, int& col, int& count, std::string& text) {
    //if the stack is empty do nothing and return error
    if (undoStack.empty())
        return Action::ERROR;
    
    //get the undoAction off the top of the stack
    undoAction* actionToUndo = undoStack.top();
    row = actionToUndo->row;
    col = actionToUndo->col;
    Action action = actionToUndo->action;
    
    //if action was delete
    //give count of characters
    //else give 1
    if (action == DELETE){
    count = (int)actionToUndo->chars.length();
    } else {
        count = 1;
    }
    
    //if action was insert give the text
    //else give empty string
    if (action == INSERT){
        text = actionToUndo->chars;
    } else{
        text = "";
    }
    
    //remove it from the stack and delete it
    undoStack.pop();
    delete actionToUndo;
    
    //return what kind of action it was
    switch (action){
        case ERROR:
            return Action::ERROR;
        case INSERT:
            return Action::INSERT;
        case SPLIT:
            return Action::SPLIT;
        case DELETE:
            return Action::DELETE;
        case JOIN:
            return Action::JOIN;
    }

    //if this point was reach there is a problem so return error
    return Action::ERROR;
}

void StudentUndo::clear() {
    //go through the stack, pop and delete each item
    while (!undoStack.empty()){
        undoAction* curr = undoStack.top();
        delete curr;
        undoStack.pop();
    }
}
