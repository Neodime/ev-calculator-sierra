#define NOMINMAX
#include "sierrachart.h"
#include <fstream>
#include <sys/stat.h>

SCDLLName("RealTime Trade Logger")

const int LAST_PROCESSED_FILL_INDEX = 0;

bool FileExists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

SCSFExport scsf_RealTimeTradeLogger(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphName = "Real-Time Trade Logger";
        sc.StudyDescription = "Logs trades from Order Fill Data to a JSON file.";

        sc.AutoLoop = 0;
        sc.UpdateAlways = 1;
        return;
    }

    const char* FilePath = "C:\\Users\\timog\\Documents\\Trading\\Sierra Jason Trades\\trade_log.json";
    int& LastFillIndexProcessed = sc.GetPersistentInt(LAST_PROCESSED_FILL_INDEX);
    int TotalFills = sc.GetOrderFillArraySize();

    s_SCOrderFillData FillData;
    s_SCTradeOrder TradeOrder;

    bool fileExists = FileExists(FilePath);
    bool isEmpty = true;

    if (fileExists)
    {
        std::ifstream infile(FilePath, std::ios::ate | std::ios::binary);
        isEmpty = (infile.tellg() == 0);
        infile.close();
    }

    std::fstream file;

    if (!fileExists || isEmpty)
    {
        file.open(FilePath, std::ios::out);
        file << "[\n";
    }
    else
    {
        file.open(FilePath, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(-1, std::ios::end);

        char ch;
        do {
            file.seekp(-1, std::ios::cur);
            file.get(ch);
            file.seekp(-1, std::ios::cur);
        } while (ch != ']' && file.tellp() > 0);

        if (ch == ']') {
            file.seekp(-1, std::ios::cur);
            file << ",\n";
        } else {
            sc.AddMessageToLog("Error: JSON closing bracket not found.", 1);
            file.close();
            return;
        }
    }

    for (int FillIndex = LastFillIndexProcessed; FillIndex < TotalFills; FillIndex++)
    {
        if (sc.GetOrderFillEntry(FillIndex, FillData))
        {
            sc.GetOrderByOrderID(FillData.InternalOrderID, TradeOrder);

            file << "  {"
                 << "\"FillDateTime\":\"" << sc.DateTimeToString(FillData.FillDateTime, FLAG_DT_COMPLETE_DATETIME_MS) << "\"," 
                 << "\"Symbol\":\"" << FillData.Symbol << "\"," 
                 << "\"TradeAccount\":\"" << FillData.TradeAccount << "\"," 
                 << "\"InternalOrderID\":" << FillData.InternalOrderID << ","
                 << "\"Quantity\":" << FillData.Quantity << "," 
                 << "\"FillPrice\":" << FillData.FillPrice << ","
                 << "\"BuySell\":\"" << ((FillData.BuySell == BSE_BUY) ? "BUY" : "SELL") << "\"," 
                 << "\"FillExecutionServiceID\":\"" << FillData.FillExecutionServiceID << "\"," 
                 << "\"TradePositionQuantity\":" << FillData.TradePositionQuantity << ","
                 << "\"IsSimulated\":" << FillData.IsSimulated << ","
                 << "\"OrderActionSource\":\"" << FillData.OrderActionSource << "\"," 
                 << "\"Note\":\"" << FillData.Note << "\"," 
                 << "\"OrderType\":\"" << TradeOrder.OrderType << "\"}\n";

            if (FillIndex < TotalFills - 1)
                file << ",\n";
        }
    }

    file << "]";

    file.close();

    LastFillIndexProcessed = TotalFills;
}
