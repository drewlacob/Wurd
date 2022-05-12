# Wurd

## Details
#### Text Editor
- Text editor with data structures such as a list of strings to maintain a text document. 
- Main functionaly is insertion or deletion of characters, return, and backspace.
- The text editor can save and load any specified text file.
- Additional capabilities: undo and spell check.

#### Undo
- Text that is changed is tracked by a stack and this allows for undo capability.
- Using Ctrl-Z, undo action is performed and the editor will undo whatever the latest action was.

#### Spell Check
- Spell check was implemented using a custom-built Trie, or prefix tree.
- Spell checker uses a given dictionary file that lists viable words to create Trie.
- Spelling suggestions are given by the editor on the bottom command line.
- When a word is misspeled its text color is changed to red. If the user then hovers over this word then spelling suggestions will be shown.
