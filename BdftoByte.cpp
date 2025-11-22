#include "BdftoByte.h"

using namespace bdfbybyte;

BDF::BDF(/* args */)
{
}

BDF::~BDF()
{
}
// 16進数文字列 (例: "40") を1バイト (0x40) に変換するヘルパー関数
inline uint8_t BDF::ChartoHex(const std::string& str)
{
    uint8_t hex = static_cast<uint8_t>(std::stoi(str,nullptr,16));

    return hex;
}

void BDF::PushArray(const std::string& line, std::vector<unsigned char>& OutputArray)
{
    std::string toStr;

    if(line.length() % 2 != 0)
    {
        std::cerr << "バイト列が奇数です:" << line << std::endl;
        return;
    }

    // 1行の文字列 (例: "4000") を2文字ずつ処理
    for (size_t i = 0; i < line.length(); i += 2) 
    {
        toStr = line.substr(i, 2);     //i番目から2文字抽出する
        OutputArray.push_back(ChartoHex(toStr));  
    }
}

FunctionResult BDF::ConvertBDFtoArray(const std::string& filename,const std::string& mojicode,int Height)
{
    // 入力ファイル
    std::ifstream ifs(filename);
    // 入力ファイル内の文字列を格納
    std::string Line;
    // 抽出対象のENCODING文字列を準備 (例: "STARTCHAR 2122"を探す)
    std::string TargetString = "ENCODING " + mojicode;
    // 状態管理フラグ
    State CurrentState = State::SEARCH_CODE;
    // バイト列
    std::vector<unsigned char> ByteData;

    int BitmapLines = 0;
    bool checkFlag = false;

    if (!ifs.is_open()) 
    {
        std::cerr << "入力ファイル開封エラー: " << filename << std::endl;
        return FunctionResult{false,ResultCode::NotOpenBDF,{}};
    }

    // getline()は改行文字\nが出るまでifstreamの内容をstringにコピーする
    // 戻り値は引数のifs。読み込みに成功したときはifsを改行文字の直後を指す状態にする
    while (std::getline(ifs, Line)) 
    {        
        switch (CurrentState)
        {
        case State::SEARCH_CODE:
            if(Line.find(TargetString) == 0) CurrentState = State::START_BITMAP;
            break;
        case State::START_BITMAP:
            checkFlag = true;           // 文字コードが見つかった
            if(Line.find("BITMAP") == 0) CurrentState = State::WAITING_ENDCHAR;
            break;
        case State::WAITING_ENDCHAR:
            HandleWaitingEndChar(Line,CurrentState,ByteData,BitmapLines);
            break;
        default:
            break;
        }        
        if (CurrentState == State::FINISH_CONVERT) break;
    }

    if (!checkFlag) return FunctionResult{false,ResultCode::NotFountCode,{}};

    ByteData.insert(ByteData.end(),(Height - BitmapLines),0x00);            // 末尾から足りない行数分0を追加

    return FunctionResult{true,ResultCode::Success,ByteData};
}

inline void BDF::HandleWaitingEndChar(const std::string& Line, State& CurrentState, std::vector<uint8_t>& ByteData, int& BitmapLines)
{
    if(Line.find("ENDCHAR") == 0)            
    {
        CurrentState = State::FINISH_CONVERT;
    } 
    else if(!Line.empty()) 
    {
        PushArray(Line, ByteData);
        BitmapLines++;          // 有効行数をカウント
    }
}

std::string BDF::ExportByteArray(const std::vector<unsigned char>& data,const std::string& mojicode)
{
    std::stringstream ss;
    int count = 0;

    ss << "unsigned char " << mojicode << "[] = {" << "\n";

    for (size_t i = 0; i < data.size(); i++)
    {
        if(i % 2 == 0)
        {
            //ss << std::setw(4) << std::setfill(' ');     // \tがでかすぎるので4文字の半角スペース
            ss << '\t';
        }
        // 16進数形式で出力し、0埋め
        ss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)data[i];

        // 末尾でないならカンマを打つ
        if (i < data.size() - 1)
        {
            ss << ",";
        }
        // 2x回カンマを打った時が改行のタイミング
        count++;
        if(count % 2 == 0)
        {
            ss << "\n";
        }
    }
    ss << "};" << "\n" << "\n";

    return ss.str();
}


#if 0
// std::optional<T> T型の戻り値を基本とし、戻り値にstd::nulloptを使えるようにする
std::optional<std::vector<unsigned char>> BDF::ConvertBDFtoArray(const std::string& filename,const std::string& mojicode)
{
    // 入力ファイル
    std::ifstream ifs(filename);
    // 入力ファイル内の文字列を格納
    std::string Line;
    // 抽出対象のENCODING文字列を準備 (例: "STARTCHAR 2122"を探す)
//    std::string TargetString = "STARTCHAR " + mojicode;

    std::string TargetString = "ENCODING " + mojicode;
    // 状態管理フラグ
    State CurrentState = State::SEARCH_CODE;
    // バイト列
    std::vector<unsigned char> ByteData;

    if (!ifs.is_open()) 
    {
        std::cerr << "入力ファイル開封エラー: " << filename << std::endl;
        return std::nullopt;
    }

    // getline()は改行文字\nが出るまでifstreamの内容をstringにコピーする
    // 戻り値は引数のifs。読み込みに成功したときはifsを改行文字の直後を指す状態にする
    while (std::getline(ifs, Line)) 
    {        
        switch (CurrentState)
        {
        case State::SEARCH_CODE:
            if(CheckString(Line,TargetString)) CurrentState = State::START_BITMAP;
            break;
        case State::START_BITMAP:
            if(CheckString(Line,"BITMAP")) CurrentState = State::WAITING_ENDCHAR;
            break;
        case State::WAITING_ENDCHAR:
            if(CheckString(Line,"ENDCHAR")) CurrentState = State::FINISH_CONVERT;
            if(!Line.empty() && CurrentState != State::FINISH_CONVERT) PushArray(Line,ByteData);    // Lineの中身が空でなくENDCHARでないとき
            break;
        default:
            break;
        }
    }
    if(CurrentState == State::FINISH_CONVERT)
    {
        return ByteData;
    }
    else
    {
        std::cerr << "該当なし" << std::endl;
        return std::nullopt;
    }
}
#endif