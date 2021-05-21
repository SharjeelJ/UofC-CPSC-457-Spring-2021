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
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

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
    // Creates a stack that will store a list of all the files/folders in the current directory and their contents recursively
    std::vector<std::string> stack;

    // Adds the current directory to the top of the stack (bottom of the vector)
    stack.push_back(".");

    // Loops through anything remaining in the stack and looks through them recursively
    while (!stack.empty()) {
        // Stores the reference to the file path of the item at the top of the stack
        auto dirname = stack.back();

        // Removes the item from the top of the stack (file path of next file/folder to examine)
        stack.pop_back();

        // Prints out the file path of the current file/folder being examined
        printf("%s\n", dirname.c_str());

        // Opens the file/folder at the path popped from the stack
        DIR *dir = opendir(dirname.c_str());

        // If the file path is a directory them loops through its contents as well
        if (dir) {
            // Loops through all the contents of the current subdirectory
            while (1) {
                // Pointer value to the next directory entry
                dirent *de = readdir(dir);

                // If the pointer is not a directory then breaks the loop (stops parsing the current subdirectory further)
                if (!de) break;

                // Stores the name of the current subdirectory
                std::string name = de->d_name;
                if (name == "." || name == "..") continue;
                std::string path = dirname + "/" + de->d_name;
                stack.push_back(path);
            }
            closedir(dir);
        }
    }
    Results temp;
    temp.valid = false;
    return temp;

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
