#include <iostream>
#include <string>

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <sys/stat.h>

// Attempts to determine whether the argument is a path that needs to be
// converted to a Windows path
bool is_path(char* arg) {
  // Check if the file exists
  struct stat buf;
  return stat(arg, &buf) == 0;
}

int main(int argc, char* argv[]) {
  std::string new_exec = "tigcc ";
  char path[2048];
  
  for(int i = 1; i < argc; ++i) {
    char* arg = argv[i];
    
    if(arg[0] == '-' && arg[1] == 'I') {
      new_exec += "-I";
      arg += 2;
    }
    
    if(is_path(arg)) {
    
      // Convert the path to a Windows path
      FILE* file = popen((std::string("cygpath --windows ") + std::string(arg)).c_str(), "r");
      
      if(!file) {
        std::cout << "Error: failed to execute cygpath!" << std::endl;
        std::cout << "Argumet: " << arg << std::endl;
        return -1;
      }
      
      if(!fgets(path, sizeof(path) - 1, file)) {
        std::cout << "Error: could not read Windows path from cygpath!" << std::endl;
        return -1;
      }
      
      // Remove the trailing newline
      int length = strlen(path);
      
      if(path[length - 1] == '\n')
        path[length - 1] = '\0';
      
      new_exec += "\"" + std::string(path) + "\" ";
    }
    else {
      new_exec += std::string(arg) + " ";
    }
  }
  
  return system(new_exec.c_str());
}