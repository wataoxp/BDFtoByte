#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <typeinfo>

 // フォントのサイズ
typedef struct{
    int Height;
    int Width;
}FontFormats;

/* std::stringやvectorはヒープ領域に確保されるため、戻り値として扱える */
class REGEX{
private:
    FontFormats font;
    void ArraytoString(const std::vector<uint8_t>& pixel,std::stringstream& ss,uint32_t columnBase);
    std::string RegexReplace(const std::string& before,const std::regex& pattern,const std::string& replacement);
public:
    REGEX(/* args */);
    ~REGEX();
    void SetFormat(int height,int width);
    /*** 正規表現置換で一つの配列にする ***/
    std::string EditFileString(const std::string& before,int width);
    /*** ファイル内テキストの整数化 ***/
    template <typename T>
    std::vector<T> StringtoHex(const std::string& file);
    /*** SSD130x用フォントに編集 ***/    
    template <typename T>
    std::string FontReverse(const std::vector<T>& data, const std::string& name);
};
