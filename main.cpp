#include "BdftoByte.h"
#include "Regex.h"
#include "FileEdit.h"

const std::string tmpByteFileName = "tmpByte.h";
const std::string tmpArrayByteName = "tmpArrayByte.h";
const std::string OutputArrayName = "font";
const std::string OutputFileName = "font_lcd.h";
const std::string DirectoryPath = "./header";

const int FONT_END_CODE = 0x2123;

int main(int argc,char*argv[]) 
{
    BDF bdf;
    REGEX regexs;
    FILEEDIT fileEdit;

    std::string JisCode;
    std::string ByteArray;
    
    std::optional<std::vector<unsigned char>> result;
    std::vector<unsigned char> Array;

    std::stringstream ss;

    UserInputFiles Names;
    FontFormats FontSize;

    if (fileEdit.ArgumentCheck(Names.BdfFileName) != true)
    {
        return 1;
    }

    fileEdit.CheckBdfFile(Names.BdfFileName,FontSize.Height,FontSize.Width,Names.StartCode);
    regexs.SetFormat(FontSize.Height,FontSize.Width);
    
    /********** 暫定値 **************/
    Names.EndCode = FONT_END_CODE;

    // BDF->Byte
    for (int i = 0; i <= (Names.EndCode - Names.StartCode); i++)    // 出力数=END-START
    {             
        JisCode = bdf.HextoString(Names.StartCode,i);                 // START+出力数=現在のJISコード
        result = bdf.ConvertBDFtoArray(Names.BdfFileName,JisCode);
        
        if(!result)
        {
            std::cerr << "指定されたJISコードが見つかりません:0x" << std::hex << JisCode << std::endl;
            return 2;
        }

        Array = *result;                                        // vectorの中身を取り出す
        ByteArray = bdf.ExportByteArray(Array,JisCode);
        ss << ByteArray;                                        // ストリームに文字列を追加
    }

    if (fileEdit.OutputFileWrite(tmpByteFileName,ss.str(),DirectoryPath) == "Error")
    {
        std::cerr << "BDF->BYTE ファイル開封エラー:" << tmpByteFileName << std::endl;
        return 3;
    }
    
    std::cout << "変換完了/出力ファイル:" << tmpByteFileName << std::endl;
   
    // もう使わない
    bdf.~BDF();

    // Edit Byte Array
    char enter;
    std::string Before,After;

    std::cout << "LCD用フォントを作成しますか？(y/n)" << std::endl;
    std::cin >> enter;

    if (enter != 'y')
    {
        std::cout << "処理を終了します" << std::endl;
        return 0;
    }

    Before = fileEdit.InputFileRead(tmpByteFileName);
    if (Before == "Error")
    {
        std::cerr << "入力ファイル開封エラー:" << tmpByteFileName << std::endl;
        return 4;
    }   
    After = regexs.EditFileString(Before,FontSize.Width);      // 配列群を1つの配列に

    if (fileEdit.OutputFileWrite(tmpArrayByteName,After,DirectoryPath) == "Error") 
    {
        std::cerr << "char->short 出力ファイル開封エラー:" << tmpArrayByteName << std::endl;
        return 5;
    }

    std::cout << "2バイト配列ファイル作成完了" << tmpArrayByteName << std::endl;

    // FontSize.Widthに応じて適切なデータ型でFontReverseを呼び出す
    if (FontSize.Width <= 8)
    {
        std::cout << "8bitフォントとして処理します。" << std::endl;
        std::vector<uint8_t> data = regexs.StringtoHex<uint8_t>(After);
        After = regexs.FontReverse<uint8_t>(data, OutputArrayName);
    }
    else if (FontSize.Width <= 16)
    {
        std::cout << "16bitフォントとして処理します。" << std::endl;
        std::vector<uint16_t> data = regexs.StringtoHex<uint16_t>(After);
        After = regexs.FontReverse<uint16_t>(data, OutputArrayName);
    }
    else if (FontSize.Width <= 32)
    {
        std::cout << "32bitフォントとして処理します。" << std::endl;
        std::vector<uint32_t> data = regexs.StringtoHex<uint32_t>(After);
        After = regexs.FontReverse<uint32_t>(data, OutputArrayName);
    }
    
    if (fileEdit.OutputFileWrite(OutputFileName,After,DirectoryPath) == "Error")
    {
        std::cerr << "出力ファイル開封エラー:" << OutputFileName << std::endl;
        return 6;
    }

    std::cout << "すべての処理が完了しました" << OutputFileName << std::endl;

    return 0;
}