#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>

/**
 * Function that uses the passed in line (string) and returns a vector of words (strings) delimited by white-space
 * @note Code was provided and has been modified to be more readable
 * @param inputLine - Line passed in as a string that will be split
 * @returns res - Vector of words as strings split based on white-space from the passed in string
 */
std::vector<std::string>
split(const std::string &inputLine) {
    // String variable that will store the current line from stdin with a white-space at the end to allow the parsing to stop when all input was parsed
    auto lineToParse = inputLine + " ";

    // Vector that will store all the words in the current line as a string
    std::vector<std::string> wordVector;

    // Boolean used to keep track of when a word ends (so white-space occurs)
    bool whitespaceHasNotOccurred = false;

    // String variable that will store the set of characters forming the word currently being parsed
    std::string currentWord = "";

    // Loops through all characters in the passed in line
    for (auto currentCharacter : lineToParse) {
        // Checks to see if the current character being parsed is a white-space
        if (isspace(currentCharacter)) {
            // If the last character being parsed was not a white-space, stores the current word being examined to the word vector (otherwise the current word is invalid and thrown away)
            if (whitespaceHasNotOccurred)
                // Stores the current word to the word vector
                wordVector.push_back(currentWord);

            // Resets the boolean keeping track of when a word ends (white-space occurs)
            whitespaceHasNotOccurred = false;

            // Resets the current word (so a new word can be formed from the next set of characters)
            currentWord = "";
        } else {
            // Appends the current character to the current word being parsed
            currentWord.push_back(currentCharacter);

            // Updates the boolean to reflect that a white-space has not occurred when parsing the previous character (so the current word is valid and should be stored when a white-space occurs)
            whitespaceHasNotOccurred = true;
        }
    }

    // Returns the populated word vector
    return wordVector;
}

// Global variables that will be used to define the buffer array for reading input in
char bufferArray[1024 * 1024];   // Buffer size of 1MB
int bufferSize = 0;  // Number of characters being stored in the buffer array
int bufferCurrentPosition = 0;   // Index in the buffer array for the next character to be read

/**
 * Function that reads in one character from stdin into the buffer array and refills the buffer array if necessary
 * @note Code was provided and has been modified to be more readable
 * @returns int - Next character in the buffer array (-1 is returned if end of file)
 */
int
fast_read_one_character_from_stdin() {
    // Checks to see if the buffer array is empty
    if (bufferCurrentPosition >= bufferSize) {
        // Refills the buffer array as it is empty and stores the number of characters in the buffer array
        bufferSize = read(STDIN_FILENO, bufferArray, sizeof(bufferArray));

        // Checks to see if the buffer array is still empty (signifying end of file) and returns -1 if it is
        if (bufferSize <= 0) return -1;

        // Resets the index from where the current character will be passed from within the buffer array
        bufferCurrentPosition = 0;
    }

    // Increments the index of the current character being read from the buffer array to read the next character
    bufferCurrentPosition++;

    // Returns the next character from the buffer array
    return bufferArray[bufferCurrentPosition];
}

/**
 * Function that reads in a line from the buffer array until end of file is reached or a linebreak is encountered
 * @note Code was provided and has been modified to be more readable
 * @returns string - String of the next line from the buffer array (composed of stdin) and is empty if end of file is reached
 */
std::string
stdin_readline() {
    // Creates a blank string variable that will store the fully parsed line string that will be returned
    std::string compiledStringResult;

    // Loops through the buffer as long as it is not empty or as long as a linebreak has not been encountered
    while (true) {
        // Stores the number of characters currently in the buffer array
        int remainingBufferSize = fast_read_one_character_from_stdin();

        // If the buffer array is empty (signaling end of file), stops the read
        if (remainingBufferSize == -1) break;

        // If a line break is encountered then stops the read (if the compiled string is not empty), otherwise proceeds past the line break and attempts to compile the resulting string
        if (remainingBufferSize == '\n') {
            // If the compiled string is not empty then breaks the read loop to return it
            if (compiledStringResult.size() > 0) break;
        } else {
            // Appends the current character from the buffer into the compiled string
            compiledStringResult.push_back(remainingBufferSize);
        }
    }

    // Returns the full compiled string (returns an empty string if end of file occurs)
    return compiledStringResult;
}

/**
 * Function that returns a boolean of whether or not the passed in word (string) is a palindrome (case insensitive)
 * @note Code was provided and has been modified to be more readable
 * @param inputString - Word passed in as a string that will be checked
 * @returns bool - Boolean where True = the passed in word is a palindrome and False = the passed in word is not a palindrome
 */
bool
is_palindrome(const std::string &inputString) {
    // Loop that iterates for half the length of the passed in string times
    for (size_t loopCounter = 0; loopCounter < inputString.size() / 2; loopCounter++)
        // Checks (case insensitive) to see if the character located at x index from the start of the string is not the same as the character located at x index from the end of the string
        if (tolower(inputString[loopCounter]) != tolower(inputString[inputString.size() - loopCounter - 1]))
            // Returns false as the requirement of being a palindrome failed
            return false;
    // Returns true as the requirement of being a palindrome succeeded
    return true;
}

/**
 * Function that returns the longest palindrome from stdin (returns the first longest one found in the case of a tie) where white-space (based on isspace()) is the delimiter for each word (string)
 * @note Code was provided and has been modified to be more readable
 * @returns string - String that is the longest palindrome found from stdin
 */
std::string
get_longest_palindrome() {
    // String that will store the longest palindrome found (if any)
    std::string longestPalindrome;

    // Loops through all the possible words
    while (1) {
        // Stores the current line (string) to parse
        std::string line = stdin_readline();

        // If the line being checked is empty, breaks the loop
        if (line.size() == 0) break;

        // Splits all the words in the line delimited by white-space, and stores the result in a vector
        auto wordVector = split(line);

        // Loops through the resulting vector
        for (auto currentWord : wordVector) {
            // Checks to see if the current word being checked is longer than the longest palindrome stored so far and skips the check to see if the current word is a palindrome if it is shorter
            if (currentWord.size() <= longestPalindrome.size()) continue;
            // Checks to see if the current word is a palindrome and stores it as the new longest palindrome encountered so far
            if (is_palindrome(currentWord))
                // Stores the current word as the new longest palindrome
                longestPalindrome = currentWord;
        }
    }

    // Results the longest palindrome encountered through the entire stdin
    return longestPalindrome;
}

/**
 * Function that requests the longest palindrome from stdin to be found (if it exists) and prints it to the console
 * @note Code was provided and has been modified to be more readable
 * @return int - 0 = Success
 */
int
main() {
    // Calls the function to get the longest palindrome (from stdin) and stores the resulting string
    std::string longestPalindrome = get_longest_palindrome();

    // Prints out the longest palindrome to the console
    printf("Longest palindrome: %s\n", longestPalindrome.c_str());

    // Returns 0 to signify the program executed successfully
    return 0;
}
