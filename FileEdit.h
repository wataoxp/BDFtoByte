#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <filesystem>

typedef struct {
    std::string BdfFileName;
    std::string FontFile;
    int StartCode;
    int EndCode;
}UserInputFiles;

enum class ErrorCode{
    Success,
    NotFoundBDF,

    NotOpenBDF,
    unknownFontSize,
    unknownFontCode,
};

class FILEEDIT
{
private:
    /* data */
public:
    FILEEDIT(/* args */);
    ~FILEEDIT();
    std::string DecToHex(const std::string& decimal,int width);
    ErrorCode ArgumentCheck(std::string& Filename);
    ErrorCode CheckBdfFile(const std::string& Filename,int& x,int& y,int& code);

    std::string InputFileRead(const std::string& filename);
    std::string OutputFileWrite(const std::string& filename,const std::string& After,const std::string& path);
};

inline std::string FILEEDIT::DecToHex(const std::string& decimal,int width)
{
    std::stringstream ss;
    int value;

    value = std::stoi(decimal);
    ss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << value;

    return ss.str();
}

