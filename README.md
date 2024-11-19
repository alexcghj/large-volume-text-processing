Two parameters are supplied to the input of the program:
 1 Configuration file. It contains words (instead of a word, it may contain an entire sentence/set of sentences/paragraphs/the entire text) that must be deleted during text processing. The configuration file is not limited in memory and consists of:
a word^another word - replacing a word with another word.
 b word - delete all matches.
 c n%word% — deleting the first n matches of the word.
 d %word%n — delete the last n matches of the word.
 2 The input file to be processed.
Each condition is specified in a separate line in the configuration file. Do not put an empty line at the end.

The program must be run from the console and support key processing:
a -c/--config — key to transfer the path to the configuration file.
b -i/--input — key to transfer the path to the input file.
c -o/--output — key to transfer the path to the output
