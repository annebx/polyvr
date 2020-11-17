#ifndef VRSYSTEM_H_INCLUDED
#define VRSYSTEM_H_INCLUDED

#include <string>
#include <vector>

using namespace std;

void printBacktrace();

bool exists(string path);
bool isFile(string path);
bool isFolder(string path);
bool makedir(string path);
bool removeFile(string path);
string canonical(string path);
string absolute(string path);
string getFileName(string path, bool withExtension = true);
string getFileExtension(string path);
string getFolderName(string path);

vector<string> openFolder(string folder);

int systemCall(string cmd);
bool compileCodeblocksProject(string path);

void fileReplaceStrings(string filePath, string oldString, string newString);

void initTime();
long long getTime();


#endif // VRSYSTEM_H_INCLUDED
