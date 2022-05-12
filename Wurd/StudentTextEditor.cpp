#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <vector>

TextEditor* createTextEditor(Undo* un)
{
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
 : TextEditor(undo) {
     m_row = 0;
     m_col = 0;
     listStrings.push_back("\0");
     currentRowIt = listStrings.begin();
}

StudentTextEditor::~StudentTextEditor()
{
    listStrings.clear();
}

bool StudentTextEditor::load(std::string file) {
    //set up infile
    std::ifstream infile(file);
    if (!infile){
        std::cout << "cannot setup infile" << std::endl;
        return false;
    }
    //discard current text
    reset();
    //read in from the file
    std::string s;
    while (getline(infile, s)){
        //get rid of \r characters
        //old
        //s.erase( std::remove(s.begin(), s.end(), '\r'), s.end() );
        //new combined solution to remove \r and \n
        for (int i = 0; i < s.length(); i++){
            if (s[i] == '\r' || s[i] == '\n')
                s = s.substr(0, i) + s.substr(i+1, s.length()-1-i);
        }
        //OLD
        //s.erase( std::remove(s.begin(), s.end(), '\n'), s.end() );
        //add each line to list
        listStrings.push_back(s);
    }
    //close the file and initialize current row iterator
    infile.close();
    //check if the file we loaded was empty
    if (!listStrings.empty()){
    currentRowIt = listStrings.begin();
    } else {
        listStrings.push_back("\0");
        currentRowIt = listStrings.begin();
    }
    //file loaded
    return true;
}

bool StudentTextEditor::save(std::string file) {
    //set up outfile and check for error
    std::ofstream outfile(file);
    if (!outfile)
        return false;
    
    //output each line from list to the file
    for (auto it = listStrings.begin(); it != listStrings.end(); it++){
        outfile << *it << std::endl;
    }
    //close the file
    outfile.close();
    return true;
}

void StudentTextEditor::reset() {
    //discard everything and go to defaults
    listStrings.clear();
    m_row = 0;
    m_col = 0;
}

void StudentTextEditor::move(Dir dir) {
    switch (dir){
            //handle moving upwards
        case UP:
            //if not at top row move up
            if (m_row > 0){
                m_row--;
                currentRowIt--;
                //if went past the end of the line reset column
                if (m_col > (*currentRowIt).length())
                    m_col = (int)(*currentRowIt).length();
            }
            break;
            //handle moving down
        case DOWN:
            //if not at bottom move down
            if (m_row < listStrings.size()-1 ){ //changed from -2 to -1
                m_row++;
                currentRowIt++;
                //if went past the end of the line reset column
                if (m_col > (*currentRowIt).length())
                    m_col = (int)(*currentRowIt).length();
                //if at the end
            }
            break;
            //handle moving to left
        case LEFT:
            //if at the top left of the entire text do nothing
            if (m_row == 0 && m_col == 0)
                return;
            else if (m_col == 0){
                //if at the beginning of a line go to previous line
                m_row--;
                currentRowIt--;
                m_col = (int)(*currentRowIt).length();
            } else {
                //else just move to the left in the current line
                m_col--;
            }
            break;
            //handle moving right
        case RIGHT:
            //if at the very end of text do nothing
            if ((m_row == listStrings.size() -1) && (m_col == (*currentRowIt).length())){
                return;
            } else if (m_col == (*currentRowIt).length()){
                //else if at the end of the line
                //go to the next line below if there is one
                m_col = 0;
                currentRowIt++;
                m_row++;
            } else if (m_col < (*currentRowIt).length()){
                //else just move to the right in the current line
                m_col++;
            }
            break;
        case HOME:
            m_col = 0;
            break;
        case END:
            //set column to end of current line
            m_col = (int)(*currentRowIt).length();
            break;
    }
        
    
}

void StudentTextEditor::del() {
    //if cursor after last character on last line do nothing
    if (m_row == listStrings.size()-1 && m_col == (*currentRowIt).length()){
        return;
       //if the cursor after last character in line
       //and a line exists after the current line
       //then merge the next line with the current line
    } else if (m_col == (*currentRowIt).length() && m_row < listStrings.size()){
        auto curLine = currentRowIt;
        auto nextLine = currentRowIt;
        nextLine++;
        (*curLine) += (*nextLine);
        listStrings.erase(nextLine);
        //tell undo to track it
        getUndo()->submit(Undo::JOIN, m_row, m_col, '\0');
        //else delete the current character if within line bounds
    } else if (m_col < (*currentRowIt).length()){
        std::string line = *currentRowIt;
        char ch = line[m_col];
        (*currentRowIt) = (*currentRowIt).erase(m_col, 1);
        
        //tell undo to track this deletion
        getUndo()->submit(Undo::DELETE, m_row, m_col, ch);
    }
}

void StudentTextEditor::backspace() {
    if (m_col > 0){
        //if not in first column
        std::string line = *currentRowIt;
        char ch = line[m_col-1];
        //erase the character at position before cursor
        (*currentRowIt) = (*currentRowIt).erase(m_col-1, 1);
        m_col--;
        //move back one
        //tell undo to track this deletion
        getUndo()->submit(Undo::DELETE, m_row, m_col, ch);
    } else if ((m_col == 0 || (*currentRowIt).empty()) && m_row > 0){
        //we need to combine the lines
        auto curLine = currentRowIt;
        auto prevLine = currentRowIt;
        //get to the previous line
        prevLine--;
        m_col = (int)(*prevLine).length();
        //combine the lines
        (*prevLine) += (*curLine);
        //move where we are to the previous line
        currentRowIt--;
        //erase the current line since it got merged
        listStrings.erase(curLine);
        //move cursor up
        m_row--;
        
        //tell undo to track this join
        getUndo()->submit(Undo::JOIN, m_row, m_col, '\0');
    }
}

void StudentTextEditor::insert(char ch) {
    std::string chs;
    chs += ch;
    //input 4 spaces for tab character and move cursor 4 spaces
    if (ch == '\t'){
        //submits each space in tab individually
        //since these are consecutive the spaces
        //will be batched together so that undoing undoes the whole tab
        //also since they are consecutive it means other characters
        //that were inserted can also be batched in the same undo
        for (int i = 0; i < 4; i++){
        (*currentRowIt).insert(m_col, " ");
        m_col += 1;
        //tell undo stack to track each space inserted
        getUndo()->submit(Undo::INSERT, m_row, m_col-1, ch);
        }
    }else {
        //else put in the character and move cursor once
        (*currentRowIt).insert(m_col, chs);
        m_col++;
        //tell undo stack to track this insertion
        getUndo()->submit(Undo::INSERT, m_row, m_col-1, ch);
    }
}

void StudentTextEditor::enter() {
    if (listStrings.size() == 0){
        //if somehow we have an empty list add an empty line
        //this should not happen
        //may be unnecessary
        listStrings.push_back("\0");
        currentRowIt = listStrings.begin();
        return;
    }
    //tell undo to track before we add the new line
    getUndo()->submit(Undo::SPLIT, m_row, m_col, 's');
    
    //find the next line
    auto nextLineIt = currentRowIt;
    nextLineIt++;
    
    //add everything after the cursor from the current line to the next line
    std::string curLine = *currentRowIt;
    std::string nextLine = curLine.substr(m_col);
    //cut current line to remove everything after cursor
    (*currentRowIt) = (*currentRowIt).substr(0, m_col);
    //insert the next line into the list
    listStrings.insert(nextLineIt, nextLine);
    //move down and column to 0, adjust iterator
    m_row++;
    m_col = 0;
    currentRowIt++;
}

void StudentTextEditor::getPos(int& row, int& col) const {
    row = m_row;
    col = m_col;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const {
    //traverse from our current line to find an iterator to the startRow line
    auto startRowIt = currentRowIt;
    int rowsToTraverse = startRow - m_row;
    //cases where the startRow > the current row
    if (rowsToTraverse > 0){
        for (int i = 0; i < rowsToTraverse; i++)
            startRowIt++;
    } else if (rowsToTraverse < 0){
        //handles cases where startRow < the current row
        rowsToTraverse = abs(rowsToTraverse);
        for (int i = 0; i < rowsToTraverse; i++)
            startRowIt--;
    }
    
    
    int rowsPrinted = 0;
    int printingThisRow = startRow;
    //while we havent reached the end of the list
    //and havent printed numRows rows
    while(startRowIt != listStrings.end() && rowsPrinted < numRows && printingThisRow <= listStrings.size()-1){
        //add the current line to the vector
        lines.push_back(*startRowIt);
        //increment variables by one
        rowsPrinted++;
        printingThisRow++;
        startRowIt++;
    }
    //return how many rows we ended up printing
    return rowsPrinted;
}

void StudentTextEditor::undo() {
    //variables to be passed by reference
    int action;
    int row;
    int col;
    int count;
    std::string chars;
    action = getUndo()->get(row, col, count, chars);
    
    //move our current row iterator to the correct row
    //same as above
    auto startRowIt = currentRowIt;
    int rowsToTraverse = row - m_row;
    if (rowsToTraverse > 0){
        for (int i = 0; i < rowsToTraverse; i++)
            startRowIt++;
    } else if (rowsToTraverse < 0){
        rowsToTraverse = abs(rowsToTraverse);
        for (int i = 0; i < rowsToTraverse; i++)
            startRowIt--;
    }
    currentRowIt = startRowIt;
    //move cursor to where it was before
    m_row = row;
    m_col = col;
    
    switch (action){
        //the undo action is deletion
        case Undo::DELETE:
            //delete count # of characters
            for (int i = 0; i < count; i++){
                (*currentRowIt) = (*currentRowIt).erase(m_col, 1);
                if(m_col > 0)
                    m_col--;
            }
            break;
        case Undo::INSERT:
            //reinsert the characters
            (*currentRowIt).insert(m_col, chars);
            break;
        case Undo::JOIN: {
            //rejoin the lines
            auto curLine = currentRowIt;
            auto nextLine = currentRowIt;
            nextLine++;
            (*curLine) += (*nextLine);
            listStrings.erase(nextLine);
            break;
        }
        case Undo::SPLIT:{
            //split the lines once more
            auto nextLineIt = currentRowIt;
            nextLineIt++;
            
            //add everything after the cursor from the current line to the next line
            std::string curLine = *currentRowIt;
            std::string nextLine = curLine.substr(m_col);
            //cut current line to remove everything after cursor
            (*currentRowIt) = (*currentRowIt).substr(0, m_col);
            //insert the next line into the list
            listStrings.insert(nextLineIt, nextLine);
            //move down and column to 0, adjust iterator
            m_row++;
            m_col = 0;
            currentRowIt++;
            break;
        }
        case Undo::ERROR:
            break;
        
    }
    
}
