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

using namespace std;

static bool
is_dir(const std::string &path) {
    struct stat buff;
    if (0 != stat(path.c_str(), &buff)) return false;
    return S_ISDIR(buff.st_mode);
}

// ======================================================================
// You need to re-implement this function !!!!
// ======================================================================
//
// getDirStats() computes stats about directory a directory
//   dir_name = name of the directory to examine
//   n = how many top words/filet types/groups to report
//
// if successful, results.valid = true
// on failure, results.valid = false
//
Results
getDirStats(const std::string &dir_name, int n) {
    // Creates a new variable of the struct Results to store the all the info of the specified directory recursively
    Results results;

    // Sets the results boolean to default to invalid (will be changed to valid once all the parsing successfully completes)
    results.valid = false;

    // Creates a stack that will store a list of all the files/folders in the current directory and their contents recursively
    vector<string> itemsToParseStack;

    // Adds the current directory to the top of the stack (bottom of the vector)
    itemsToParseStack.push_back(dir_name);

    // Loops through anything remaining in the stack and looks through them recursively
    while (!itemsToParseStack.empty()) {
        // Stores the reference to the file path of the item at the top of the stack
        auto currentTopItemName = itemsToParseStack.back();

        // Removes the item from the top of the stack (file path of the file/folder to examine)
        itemsToParseStack.pop_back();

        // Prints out the file path of the current file/folder being examined
//        printf("%s\n", currentTopItemName.c_str());

        // Opens the file/folder at the path popped from the stack
        DIR *currentTopItem = opendir(currentTopItemName.c_str());

        // If the file path is a directory then loops through its contents as well
        if (currentTopItem) {
            // Loops through all the contents of the current top directory being examined
            while (1) {
                // Pointer value to the next sub item
                dirent *nextSubItem = readdir(currentTopItem);

                // If the pointer is not accessible (indicates the end of the folder) then breaks the loop
                if (!nextSubItem) break;

                // Stores the name of the sub item
                string currentSubItemName = nextSubItem->d_name;

                // Skips this loop iteration for the first 2 values we receive (useless data)
                if (currentSubItemName == "." || currentSubItemName == "..") continue;

                // Adds the current sub-item's full path as the new parent directory for the next directory to be examined
                string path = currentTopItemName + "/" + currentSubItemName;

                // Pushes the new file path to the top of the stack
                itemsToParseStack.push_back(path);
            }
            // Closes the current file/folder
            closedir(currentTopItem);
        }

        // Creates a new file stream that will store the data from popen
        FILE *fileData;

        // Character array of the size 4096 that will store the path being used by popen
        char popenData[PATH_MAX];

        // Sets the filestream to contain the data from the popen command
        fileData = popen(("file " + currentTopItemName).c_str(), "r");

        // Loops through all the data obtained from popen and prints it to the console
        while (fgets(popenData, PATH_MAX, fileData) != NULL)
            printf("%s", popenData);

        // Closes popen and the file stream
        pclose(fileData);

        // Creates a stat struct to get the current file's size
        struct stat buffer;

        // Prints out the data obtained through stat
        printf("Last modification: %s", ctime(&buffer.st_mtime));
        printf("File size: %lld bytes\n", (long long) buffer.st_size);
        printf("\n");
    }
    // Returns the complete directory info
    return results;

    // The results below are all hard-coded, to show you all the fields
    // you need to calculate. You should delete all code below and
    // replace it with your own code.

    // Custom data structure consisting of the directory's information
    Results res;

    // Stores a boolean to keep track of whether or not the directory's information is update to date and complete (valid)
    res.valid = false;

    // If the passed in directory is not actually a valid directory, returns the results as is (mentioning directory is not valid in main.cpp)
    if (!is_dir(dir_name)) return res;

    // prepare a fake results
    res.largest_file_path = dir_name + "/some_dir/some_file.txt";
    res.largest_file_size = 123;
    res.n_files = 321;
    res.n_dirs = 333;
    res.all_files_size = 1000000;

    std::string type1 = "C source";
    int count1 = 5;
    res.most_common_types.push_back({type1, count1});
    res.most_common_types.push_back({"makefile script", 4});
    res.most_common_types.push_back({"C++ source", 2});
    res.most_common_types.push_back({"PNG image", 1});

    res.most_common_words.push_back({"hello", 3});
    res.most_common_words.push_back({"world", 1});

    std::vector<std::string> group1;
    group1.push_back(dir_name + "/file1.cpp");
    group1.push_back(dir_name + "/lib/sub/other.c");
    res.duplicate_files.push_back(group1);
    std::vector<std::string> group2;
    group2.push_back(dir_name + "/readme.md");
    group2.push_back(dir_name + "/docs/readme.txt");
    group2.push_back(dir_name + "/x.y");
    res.duplicate_files.push_back(group2);

    // Updates the boolean to reflect that the directory's info is valid (complete)
    res.valid = true;

    // Returns the valid directory info
    return res;
}
