#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"

#include <string>
#include <vector>
#include <fstream>
//#include <iostream>

const int numCharacters = 27;

class StudentSpellCheck : public SpellCheck {
public:
    StudentSpellCheck();
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
    //trie implementation
    struct trieNode{
        trieNode();
        ~trieNode();
        void insert(std::string word);
        bool search(std::string word);
        void freeTree(trieNode* nodePtr);
        trieNode* nodes[numCharacters];
        bool isEnd;
    };
    
    trieNode* trieRoot;
};

#endif  // STUDENTSPELLCHECK_H_
