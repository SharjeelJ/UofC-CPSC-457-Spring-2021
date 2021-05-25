/// =========================================================================
/// Written by pfederl@ucalgary.ca in 2020, for CPSC457.
/// =========================================================================
/// You need to edit this file.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, but you need to implement the getDirStats() function as
/// defined in "getDirStats.h".

#include "getDirStats.h"
#include "digester.h"
#include <sys/stat.h>
#include <dirent.h>
#include <fstream>
#include <unordered_map>
#include <algorithm>

using namespace std;

static bool
is_dir(const std::string &path) {
    struct stat buff;
    if (0 != stat(path.c_str(), &buff)) return false;
    return S_ISDIR(buff.st_mode);
}

// Custom comparator to only compare the second value of pairs (will be used to sort in descending order for the most occurring file types or for the most occurring words)
bool
fileTypeOrWordsComparator(const pair<string, int> &firstElement, const pair<string, int> &secondElement) {
    return firstElement.second > secondElement.second;
}

// Custom comparator to only compare the size of the vectors (will be used to sort in descending order for the most occurring duplicate files)
bool
fileDigestComparator(const vector<string> &firstElement,
                     const vector<string> &secondElement) {
    return firstElement.size() > secondElement.size();
}

// getDirStats() computes stats about directory a directory
//   dir_name = name of the directory to examine
//   n = how many top words/filet types/groups to report
//
// if successful, results.valid = true
// on failure, results.valid = false
//
Results
getDirStats(const std::string &dir_name, int n) {
    // Creates a new variable of the struct Results to store all the info of the specified directory recursively
    Results results;

    // Sets the default values for the results data struct
    results.valid = false;
    results.largest_file_path = "";
    results.largest_file_size = -1;
    results.n_files = 0;
    results.n_dirs = -1; // Starts at -1 to not count the root directory as an encountered directory
    results.all_files_size = 0;

    // If the passed in directory is not actually a valid directory, returns the results as is
    if (!is_dir(dir_name)) return results;

    // Creates a stack that will store a list of all the files/folders in the current directory and their contents recursively
    vector<string> itemsToParseStack;

    // Adds the current directory (root) to the top of the stack (bottom of the vector)
    itemsToParseStack.push_back(dir_name);

    // Creates an unordered map that will be used as a histogram for file types encountered
    unordered_map<string, int> fileTypeHistogram;

    // Creates an unordered map that will be used as a histogram for file digests encountered
    unordered_map<string, vector<string>> fileDigestHistogram;

    // Creates an unordered map that will be used as a histogram for words used in the files encountered
    unordered_map<string, int> fileWordsHistogram;

    // Loops through anything remaining in the stack and looks through them recursively if possible
    while (!itemsToParseStack.empty()) {
        // Stores the reference to the file path of the item at the top of the stack
        auto currentTopItem = itemsToParseStack.back();

        // Removes the item from the top of the stack (file path of the file/folder to examine)
        itemsToParseStack.pop_back();

        // Opens the file/folder at the path popped from the stack
        DIR *currentTopItemData = opendir(currentTopItem.c_str());

        // If the file path is a directory then loops through its contents as well
        if (currentTopItemData) {
            // Loops through all the contents of the current top directory being examined
            while (true) {
                // Pointer value to the next sub-item
                dirent *nextSubItem = readdir(currentTopItemData);

                // If the pointer is not accessible (indicates the end of the folder) then breaks the loop
                if (!nextSubItem) break;

                // Stores the name of the sub-item
                string currentSubItemName = nextSubItem->d_name;

                // Skips the loop iteration for 2 of the values we receive (data mentioning the current and parent directory)
                if (currentSubItemName == "." || currentSubItemName == "..") continue;

                // Creates a string of the complete path of the current sub-item
                string path = currentTopItem + "/" + currentSubItemName;

                // Pushes the current sub-item's file path to the top of the stack to be checked later for its own subdirectories
                itemsToParseStack.push_back(path);
            }

            // Closes the current directory
            closedir(currentTopItemData);

            // Increments the counter keeping track of the total number of directories encountered
            results.n_dirs++;
        }
            // Code run if the file path is a file rather than a directory
        else {
            // Creates a new file stream that will store the data from popen
            FILE *popenFileData;

            // Character array of the size 4096 that will store the path being used by popen
            char popenData[4096];

            // Sets the filestream to contain the data from the popen command
            popenFileData = popen(("file -b " + currentTopItem).c_str(), "r");

            // Gets the data obtained from popen
            fgets(popenData, 4096, popenFileData);

            // Cleans the popen result by grabbing all text up to the first instance of ,
            string popenResult = string(popenData).substr(0, string(popenData).find(','));

            // Further cleans the popen result by stripping all unnecessary trailing whitespaces
            popenResult = popenResult.erase(popenResult.find_last_not_of(" \t\n\r\f\v") + 1);

            // Closes popen and the file stream
            pclose(popenFileData);

            // Adds the current file's type to the histogram
            fileTypeHistogram[popenResult]++;

            // Adds the current file's digest to the histogram
            fileDigestHistogram[sha256_from_file(currentTopItem).c_str()].push_back(currentTopItem);

            // Creates a stat struct that will be used to get the current file's size
            struct stat buffer;

            // Populates the stat struct with the file's data
            stat(currentTopItem.c_str(), &buffer);

            // Checks to see if the current file is the largest file we have encountered so far and stores its path and size if it is
            if (buffer.st_size > results.largest_file_size) {
                results.largest_file_path = currentTopItem;
                results.largest_file_size = buffer.st_size;
            }

            // Adds the current file size to the existing total file size of the specified directory in the results struct
            results.all_files_size += buffer.st_size;

            // Creates a new file stream that will store the data from fopen
            FILE *fopenFileData = fopen(currentTopItem.c_str(), "r");

            // Loops through the entirety of the file's contents
            while (true) {
                // String that will store the current word being parsed
                string currentWord;

                // Loops through the file and parses each character individually to form a word (stops parsing the current word on non alphabetical characters)
                while (true) {
                    // Stores the current character's number
                    int currentChar = fgetc(fopenFileData);

                    // If the current character indicates an end of file (is -1) then stops parsing the word further
                    if (currentChar == EOF) break;

                    // Converts the current character to its lower case counterpart
                    currentChar = tolower(currentChar);

                    // Checks to see if the current character being parsed is not an alphabetical character
                    if (!isalpha(currentChar)) {
                        // Proceeds with parsing the word further if there were no alphabetical characters encountered so far (word is of size 0 so there is nothing to store)
                        if (currentWord.empty())
                            continue;
                        else
                            // If alphabetical characters were encountered while parsing the current word then stops parsing it further (tp store the current word)
                            break;
                    } else {
                        // Appends the current character to the word string if it was an alphabetical character
                        currentWord.push_back(currentChar);
                    }
                }

                // If an empty word was passed back then stops parsing the file entirely (indicates end of file)
                if (currentWord.empty()) break;
                else if (currentWord.size() >= 3)
                    // If the word is of length 3 or more then adds it to the histogram
                    fileWordsHistogram[currentWord]++;
            }

            // Closes fopen and the file stream
            fclose(fopenFileData);

            // Increments the counter keeping track of the total number of files encountered
            results.n_files++;
        }
    }

    // Loops through the file type histogram and populates the most common file types results vector
    for (auto &currentElement : fileTypeHistogram)
        results.most_common_types.emplace_back(currentElement.first, currentElement.second);

    // Performs a partial sort if there are more than N entries
    if (results.most_common_types.size() > size_t(n)) {
        // Performs a partial sort up to N entries using the custom comparator
        partial_sort(results.most_common_types.begin(), results.most_common_types.begin() + n,
                     results.most_common_types.end(),
                     &fileTypeOrWordsComparator);

        // Drops all the entries that occur after N entries
        results.most_common_types.resize(n);
    } else {
        // Performs a full sort as there are less than N entries using the custom comparator
        sort(results.most_common_types.begin(), results.most_common_types.end(), &fileTypeOrWordsComparator);
    }

    // Loops through the file digest histogram and populates the duplicate files results vector
    for (auto &currentElement : fileDigestHistogram) {
        // Only adds vectors that had more than 1 entry (as 1 file per digest means there was no duplicate files)
        if (currentElement.second.size() > 1)
            results.duplicate_files.push_back(currentElement.second);
    }

    // Performs a partial sort if there are more than N entries
    if (results.duplicate_files.size() > size_t(n)) {
        // Performs a partial sort up to N entries using the custom comparator
        partial_sort(results.duplicate_files.begin(), results.duplicate_files.begin() + n,
                     results.duplicate_files.end(), &fileDigestComparator);

        // Drops all the entries that occur after N entries
        results.duplicate_files.resize(n);
    } else {
        // Performs a full sort as there are less than N entries using the custom comparator
        sort(results.duplicate_files.begin(), results.duplicate_files.end(), &fileDigestComparator);
    }

    // Loops through the file words histogram and populates the most common words results vector
    for (auto &currentElement : fileWordsHistogram)
        results.most_common_words.emplace_back(currentElement.first, currentElement.second);

    // Performs a partial sort if there are more than N entries
    if (results.most_common_words.size() > size_t(n)) {
        // Performs a partial sort up to N entries using the custom comparator
        partial_sort(results.most_common_words.begin(), results.most_common_words.begin() + n,
                     results.most_common_words.end(),
                     &fileTypeOrWordsComparator);

        // Drops all the entries that occur after N entries
        results.most_common_words.resize(n);
    } else {
        // Performs a full sort as there are less than N entries using the custom comparator
        sort(results.most_common_words.begin(), results.most_common_words.end(), &fileTypeOrWordsComparator);
    }

    // Updates the boolean to reflect that the directory's info is valid (complete)
    results.valid = true;

    // Returns the complete directory info
    return results;
}
