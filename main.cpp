#include "BdftoByte.h"
#include "Regex.h"
#include "FileEdit.h"

struct SystemOutputNames{
    const std::string tmpByteFileName;
    const std::string tmpArrayByteName;
    const std::string OutputArrayName;
    const std::string OutputFileName;
    const std::string DirectoryPath;
};

static bool CheckBDF(std::string& filename,FILEEDIT& edit,FontFormats& fonts,int& code);
static bool ConvertBDF(UserInputFiles& Names,FontFormats& FontSize,BDF& bdf,FILEEDIT& fileEdit,std::stringstream& ss);
static std::string EditRegex(const SystemOutputNames& names,std::string& after,REGEX& regexs,FILEEDIT& fileEdit,FontFormats& fonts);

int main(int argc,char*argv[]) 
{
    // 各種オブジェクト
    BDF bdf;
    REGEX regexs;
    FILEEDIT fileEdit;

    FunctionResult Result;
    UserInputFiles Names;
    FontFormats FontSize;

    static const SystemOutputNames FileNames = {
        "tmpByte.h",
        "tmpArrayByte.h",
        "font",
        "font_lcd.h",
        "./header",
    };

    if (!CheckBDF(Names.BdfFileName,fileEdit,FontSize,Names.StartCode))
    {
        std::cerr << "Error BDF" << std::endl;
        return 1;
    }

    /********** 暫定値 **************/
    Names.EndCode = 3;

    // BDF to Byte
    std::stringstream ss;
    if (!ConvertBDF(Names,FontSize,bdf,fileEdit,ss))
    {
        std::cerr << "Error Convert" << std::endl;
        return 2;
    }

    if (fileEdit.OutputFileWrite(FileNames.tmpByteFileName,ss.str(),FileNames.DirectoryPath) == "Error")
    {
        std::cerr << "BDF->BYTE ファイル開封エラー:" << FileNames.tmpByteFileName << std::endl;
        return 3;
    }
    
    std::cout << "変換完了/出力ファイル:" << FileNames.tmpByteFileName << std::endl;
   
    // Edit Byte Array
    char enter;
    std::string After;

    std::cout << "LCD用フォントを作成しますか？(y/n)" << std::endl;
    std::cin >> enter;

    std::cout << EditRegex(FileNames,After,regexs,fileEdit,FontSize) << std::endl;

    return 0;
}

static bool CheckBDF(std::string& filename,FILEEDIT& edit,FontFormats& fonts,int& code)
{
    ErrorCode result;

    std::cout << "BDFファイルを指定してください:";
    std::cin >> filename;
    std::cout << "BDFファイル名:" << filename << std::endl;

    result = edit.ArgumentCheck(filename);

    if (result != ErrorCode::Success)
    {
        std::cerr << "BDFファイルが見つかりません" << filename << std::endl;
        return false;
    }
    std::cout << "BDFファイルが見つかりました" << std::endl;

    result = edit.CheckBdfFile(filename,fonts.Width,fonts.Height,code);

    if (result != ErrorCode::Success)
    {
        switch (result)
        {
        case ErrorCode::unknownFontSize:
            std::cerr << "フォントサイズが見つかりません" << std::endl;
            break;
        case ErrorCode::unknownFontCode:
            std::cerr << "文字コードが見つかりません" << std::endl;
            break;
        default:
            break;
        }
        return false;
    }

    std::cout << "Height:" << std::dec << fonts.Height << std::endl;
    std::cout << "Width:" << std::dec << fonts.Width << std::endl;

    std::cout << "先頭コード:" << std::dec << code << std::endl; 

    return true;
}

static bool ConvertBDF(UserInputFiles& Names,FontFormats& FontSize,BDF& bdf,FILEEDIT& fileEdit,std::stringstream& ss)
{
    std::string mojiCode;
    std::string ByteArray;
    
    FunctionResult Result;
    std::vector<unsigned char> Array;

    // Names.StartCode = 0x2421;
    // Names.EndCode = 10;

    for (int i = 0; i <= ((Names.StartCode+Names.EndCode) - Names.StartCode); i++)
    {             
        // 文字コードを変換
        mojiCode = std::to_string(Names.StartCode + i);
        // BDFファイルから所定の文字列をサーチ
        Result = bdf.ConvertBDFtoArray(Names.BdfFileName, mojiCode,FontSize.Width);     
        
        if(Result.errorCode == bdfbybyte::ResultCode::NotFountCode)
        {
            continue;
        }

        Array = Result.data;                                    // vectorを取り出す
        mojiCode = fileEdit.DecToHex(mojiCode,4);               // 10進数を16進数に(主にJISコード用)
        ByteArray = bdf.ExportByteArray(Array,mojiCode);        // バイト配列を作成
        ss << ByteArray;                                        // ストリームに文字列を追加
    }
    return true;
}

static std::string EditRegex(const SystemOutputNames& names,std::string& after,REGEX& regexs,FILEEDIT& fileEdit,FontFormats& fonts)
{
    std::string before;
    std::string result = "Success";

    // フォントサイズをREGEXインスタンスに伝達
    regexs.SetFormat(fonts.Height,fonts.Width);

    // バイト配列ファイルを読み込む
    before = fileEdit.InputFileRead(names.tmpByteFileName,names.DirectoryPath);
    if (before == "Error")
    {
        result = "入力ファイル開封エラー";
        return result;
    }

    // 正規表現で置換
    after = regexs.EditFileString(before,fonts.Width);
    if (fileEdit.OutputFileWrite(names.tmpArrayByteName,after,names.DirectoryPath) == "Error")
    {
        result = "出力ファイル開封エラー";
        return result;
    }
    
    std::cout << "二次元配列ファイル作成完了" << names.tmpArrayByteName << std::endl;

    // LCD用フォント配列ファイルを作成
    if (fonts.Width <= 8)
    {
        std::cout << "8bitフォントとして処理します。" << std::endl;
        std::vector<uint8_t> data = regexs.StringtoHex<uint8_t>(after);
        after = regexs.FontReverse<uint8_t>(data, names.OutputArrayName);
    }
    else if (fonts.Width <= 16)
    {
        std::cout << "16bitフォントとして処理します。" << std::endl;
        std::vector<uint16_t> data = regexs.StringtoHex<uint16_t>(after);
        after = regexs.FontReverse<uint16_t>(data, names.OutputArrayName);
    }

    if (fileEdit.OutputFileWrite(names.OutputFileName,after,names.DirectoryPath) == "Error")
    {
        result = "出力ファイル開封エラー";
        return result;
    }
    
    return result;
}


#if 0
    32ビットフォントはテストできないので保留
    else if (FontSize.Width <= 32)
    {
        std::cout << "32bitフォントとして処理します。" << std::endl;
        std::vector<uint32_t> data = regexs.StringtoHex<uint32_t>(After);
        After = regexs.FontReverse<uint32_t>(data, OutputArrayName);
    }
#endif