#include "StudentSpellCheck.h"
#include <string>
#include <vector>

//HELPER FUNCTION
//USED BY TRIE AND STUDENTSPELLCHECK
std::string processWord(std::string word){
    //make all lower case
    for (int i = 0; i < word.length(); i++){
        word[i] = tolower(word[i]);
    }
    std::string processedWord;
    //remove all non alpha or ' characters
    for (int i = 0; i < word.length(); i++){
        if (isalpha(word[i]) || word[i] ==  '\'')
            processedWord += word[i];
    }
    //return the processed word
    return processedWord;
}

//SPELL CHECK IMPLEMENTATIONS
SpellCheck* createSpellCheck()
{
	return new StudentSpellCheck;
}

StudentSpellCheck::StudentSpellCheck(){
    trieRoot = new trieNode;
}

StudentSpellCheck::~StudentSpellCheck() {
    if (trieRoot != nullptr){
        //use free tree function defined below
        trieRoot->freeTree(trieRoot);
    }
}

bool StudentSpellCheck::load(std::string dictionaryFile) {
    std::ifstream inDictionary(dictionaryFile);
    //if we failed to find the dictionary return false
    if (!inDictionary)
        return false;
    
    //discard old dictionary
    trieRoot->freeTree(trieRoot);
    trieRoot = new trieNode;
    
    std::string s;
    //read in each line and insert it into the trie
    while(getline(inDictionary, s)){
        //insert each word, this function will process the word so
        //no need to process in advance
        trieRoot->insert(s);
    }
    
    //close - good practice?
    inDictionary.close();
    return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) {
    //no need to process before search, search function processes within
    bool inDictionary = trieRoot->search(word);
    //if we found the word in the dictionary it is spelled correctly so just return
    if (inDictionary){
        return true;
    } else{
        //the word is not spelled correctly
        suggestions.clear();
        int suggested = 0;
        std::string processedWord = processWord(word);
        //these are the possible characters to exchange with each
        //character of the word
        std::string possibleChars = "abcdefghijklmnopqrstuvwxyz'";
        //for each letter in the word
        for (int i = 0; i < processedWord.length(); i++){
            //for each possible character (27), test that word
            for (int j = 0; j < possibleChars.length(); j++){
                //exchange the possible character with the character in the word
                std::string newWord = processedWord;
                newWord[i] = possibleChars[j];
                //and search for the new word created in the dictionary
                bool newWordInDictionary = trieRoot->search(newWord);
                //if it is a word then add that to suggestions list
                if (newWordInDictionary){
                    suggestions.push_back(newWord);
                    suggested++;
                }
                //if we reached the max suggestions we can stop
                if (suggested == max_suggestions)
                    return false;
            }
        }
        
        //we didnt reach max suggestions but the word is still spelled wrong
        //so return false
        return false;
    }
}

void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) {
	
    //declare and initialize values
    int start = -1;
    int end = -1;
    int pos = 0;
    
    //while we're not at the end of the line
    while (pos < line.length()){
        
        //if its a valid character for a word
        if ( isalpha(line[pos]) || line[pos] == '\'' ){
            start = pos;
            end = pos;
            //loop until we find the end of the word
            //or hit the end of the line
            while ( isalpha(line[end]) || line[end] == '\'' ){
                if (end < line.length()){
                end++;
                } else{
                    break;
                }
            }
            //make a new position object with the start
            //and end we just found
            Position newWord;
            newWord.start = start;
            newWord.end = end-1;
            std::string wordToCheck = line.substr(start, end-start);
            //check if the word we just found is spelled correctly
            bool inDictionary = trieRoot->search(wordToCheck);
            //if not then it is a problem so add the position to the problems vector
            if (!inDictionary){
            problems.push_back(newWord);
            }
            //set the position we're at in the string to the end
            pos = end;
        } else{
            //else continue moving through the string to find the next word
            pos++;
        }
        
    }
}

//TRIE NODE IMPLEMENTATIONS
StudentSpellCheck::trieNode::trieNode(){
    //on creation sell all child nodes to be null since
    //nothing is there so far
    for (int i = 0; i < numCharacters; i++){
        nodes[i] = nullptr;
    }
    //this is not the end of a word
    isEnd = false;
}

StudentSpellCheck::trieNode::~trieNode(){
    //calls helper free tree so that we can use recursion
    //to delete the entire tree
    //freeTree(this);
    //delete this;
}

void StudentSpellCheck::trieNode::freeTree(trieNode* nodePtr){
    //if null dont do anything, reached leaf or no tree
    if (nodePtr == nullptr){
        return;
    }
    //for every child that is not null, free that child
    for(int i = 0; i < numCharacters; i++){
        if (nodePtr->nodes[i] != nullptr)
        freeTree(nodePtr->nodes[i]);
    }
    
    //delete the current node since we already deleted all children
    delete nodePtr;
}

void StudentSpellCheck::trieNode::insert(std::string word){
    //process the word
    std::string processedWord = processWord(word);
    trieNode* current = this;
    int pos;
    
    //for each letter in the word
    for (int i = 0; i < processedWord.length(); i++){
        //compute the position in the child arrray
        pos = processedWord[i] - 'a';
        //catch for ' since we want it to use position 26
        if (pos < 0)
            pos = 26;
        //if there isnt a node there (letter not there)
        //then make one
        if(current->nodes[pos] == nullptr)
            current->nodes[pos] = new trieNode();
        
        //the current node goes through the child node and continues
        current = current->nodes[pos];
    }
    //we reached the end of a word so mark it as a word here
    current->isEnd = true;
}

bool StudentSpellCheck::trieNode::search(std::string word){
    //process the word
    trieNode* current = this;
    std::string processedWord = processWord(word);
    int pos;
    //for each letter in the word
    for (int i = 0; i < processedWord.length(); i++){
        //find the position in the array
        pos = processedWord[i] - 'a';
        //catch for '
        if (pos < 0)
            pos = 26;
        
        //if there is not the child node associated
        //with the letter then we dont have the word
        //so return false since its not in the trie
        if(current->nodes[pos] == nullptr)
            return false;
        //other wise continue moving down the child nodes
        current = current->nodes[pos];
        
    }
    //make sure we didnt end at a null ptr and that this is a word
    return (current != nullptr && current->isEnd);
}
