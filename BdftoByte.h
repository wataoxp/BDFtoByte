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

// 状態管理フラグ
enum class State{
    SEARCH_CODE,
    START_BITMAP,
    WAITING_ENDCHAR,
    FINISH_CONVERT,
};

enum class ErrorCode{
    Success,
    NotOpenBDF,
    NotFountCode,
};


typedef struct{
    bool result;
    ErrorCode errorCode;
    std::vector<unsigned char> data;
}FunctionResult;

class BDF{
private:
    // 文字列をバイト配列として出力
    void PushArray(const std::string& hex_data_line, std::vector<unsigned char>& output_array);
    uint8_t ChartoHex(const std::string& str);
public:
    BDF(/* args */);
    ~BDF();
   
    // BDFファイルの走査
    FunctionResult ConvertBDFtoArray(const std::string& filename,const std::string& mojicode,int Width);
    // バイト配列の作成
    std::string ExportByteArray(const std::vector<unsigned char>& data,const std::string& JisCode);
    std::string CodetoString(int code,int num);
};

inline std::string BDF::CodetoString(int code,int num)
{
    std::stringstream tmp;

    tmp << code + num;

    return tmp.str();
}
