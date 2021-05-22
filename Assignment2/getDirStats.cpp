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
    // Creates a new variable of the struct Results to store all the info of the specified directory recursively
    Results results;

    // Sets the default values for the results data struct
    results.valid = false;
    results.largest_file_path = "";
    results.largest_file_size = -1;
    results.n_files = 0;
    results.n_dirs = 0;
    results.all_files_size = 0;

    // If the passed in directory is not actually a valid directory, returns the results as is (mentioning directory is not valid in main.cpp)
    if (!is_dir(dir_name)) return results;

    // TODO: Remove below fake results

    // prepare a fake results
//    results.largest_file_path = dir_name + "/some_dir/some_file.txt";
//    results.largest_file_size = 123;
//    results.n_files = 321;
//    results.n_dirs = 333;
//    results.all_files_size = 1000000;

    std::string type1 = "C source";
    int count1 = 5;
    results.most_common_types.push_back({type1, count1});
    results.most_common_types.push_back({"makefile script", 4});
    results.most_common_types.push_back({"C++ source", 2});
    results.most_common_types.push_back({"PNG image", 1});

    results.most_common_words.push_back({"hello", 3});
    results.most_common_words.push_back({"world", 1});

    std::vector<std::string> group1;
    group1.push_back(dir_name + "/file1.cpp");
    group1.push_back(dir_name + "/lib/sub/other.c");
    results.duplicate_files.push_back(group1);
    std::vector<std::string> group2;
    group2.push_back(dir_name + "/readme.md");
    group2.push_back(dir_name + "/docs/readme.txt");
    group2.push_back(dir_name + "/x.y");
    results.duplicate_files.push_back(group2);

    // Updates the boolean to reflect that the directory's info is valid (complete)
    results.valid = true;

    // TODO: Remove above fake results

    // Creates a stack that will store a list of all the files/folders in the current directory and their contents recursively
    vector<string> itemsToParseStack;

    // Adds the current directory (root) to the top of the stack (bottom of the vector)
    itemsToParseStack.push_back(dir_name);

    // Loops through anything remaining in the stack and looks through them recursively if possible
    while (!itemsToParseStack.empty()) {
        // Stores the reference to the file path of the item at the top of the stack
        auto currentTopItem = itemsToParseStack.back();

        // Removes the item from the top of the stack (file path of the file/folder to examine)
        itemsToParseStack.pop_back();

        // Prints out the file path of the current file/folder being examined
//        printf("%s\n", currentTopItem.c_str());

        // Opens the file/folder at the path popped from the stack
        DIR *currentTopItemData = opendir(currentTopItem.c_str());

        // If the file path is a directory then loops through its contents as well
        if (currentTopItemData) {
            // Loops through all the contents of the current top directory being examined
            while (1) {
                // Pointer value to the next sub item
                dirent *nextSubItem = readdir(currentTopItemData);

                // If the pointer is not accessible (indicates the end of the folder) then breaks the loop
                if (!nextSubItem) break;

                // Stores the name of the sub item
                string currentSubItemName = nextSubItem->d_name;

                // Skips this loop iteration for the first 2 values we receive (useless data)
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
        // Increments the counter keeping track of the total number of files encountered
        results.n_files++;

        // Creates a new file stream that will store the data from popen
        FILE *fileData;

        // Character array of the size 4096 that will store the path being used by popen
        char popenData[PATH_MAX];

        // Sets the filestream to contain the data from the popen command
        fileData = popen(("file -b " + currentTopItem).c_str(), "r");

        // Prints the data obtained from popen
        while (fgets(popenData, PATH_MAX, fileData) != NULL) {
            printf(popenData);
        }

        // Closes popen and the file stream
        pclose(fileData);

        // Creates a stat struct that will be used to get the current file's size
        struct stat buffer;

        // Prints out the data obtained through stat
        if (stat(currentTopItem.c_str(), &buffer) == 0) {
//            printf("Last modification: %s", ctime(&buffer.st_mtime));
//            printf("File size: %lld bytes\n", (long long) buffer.st_size);
//            printf("\n");
        }

        // Checks to see if the current file is the largest file we have encountered and stores its data if it is
        if (buffer.st_size > results.largest_file_size) {
            results.largest_file_size = buffer.st_size;
            results.largest_file_path = currentTopItem;
        }

        // Adds the current file size to the existing total file size of the specified directory in the results struct
        results.all_files_size += buffer.st_size;
    }

    // Returns the complete directory info
    return results;
}
