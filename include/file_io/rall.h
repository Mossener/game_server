#ifndef RALL_H
#define RALL_H
#include <string>
#include <fstream>
#include <typeindex>
#include <stdexcept>
#include <ios>
class FILE_IO{
private:
    std::ofstream file;
public:
    FILE_IO(const std::string &path,const std::ios::openmode mode = std::ios::app): file(path,mode){
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + path);
        }
    }
    ~FILE_IO(){
        if (file.is_open()) {
            file.close();
        }
    }
    std::ofstream& getFile(){
        return file;
    }
};
#endif // RALL_H