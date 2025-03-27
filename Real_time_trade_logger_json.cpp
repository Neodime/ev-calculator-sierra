#define NOMINMAX
#include "sierrachart.h"
#include <fstream>

SCDLLName("RealTime Trade Logger")

const int LAST_PROCESSED_FILL_INDEX = 0;

SCSFExport scsf_RealTimeTradeLogger(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphName = "Real-Time Trade Logger";
        sc.StudyDescription = "Logs trades from Order Fill Data to a JSON file.";

        sc.AutoLoop = 0;
        sc.UpdateAlways = 1;
        sc.ReceivePointerEvents = 0;

        return;
    }

    const char* FilePath = "C:\\Users\\timog\\Documents\\Trading\\Sierra Jason Trades\\trade_log.json";

    int& LastFillIndexProcessed = sc.GetPersistentInt(LAST_PROCESSED_FILL_INDEX);

    int TotalFills = sc.GetOrderFillArraySize();

    bool FileExists;
    std::ifstream CheckFile(FilePath);
    FileExists = CheckFile.good();
    CheckFile.close();

    std::fstream File;

    if (!FileExists)
    {
        File.open(FilePath, std::ios::out);
        File << "[\n";
        File.close();
    }

    File.open(FilePath, std::ios::in | std::ios::out);

    if (!File.is_open())
    {
        sc.AddMessageToLog("Error: Cannot open trade log file.", 1);
        return;
    }

    File.seekp(0, std::ios::end);

    bool FirstEntry = (File.tellp() <= 2);

    if (!FirstEntry)
    {
        File.seekp(-2, std::ios::end);
        File << ",\n";
    }

    s_SCOrderFillData FillData;
    s_SCTradeOrder TradeOrder;

    for (int FillIndex = LastFillIndexProcessed; FillIndex < TotalFills; FillIndex++)
    {
        if (sc.GetOrderFillEntry(FillIndex, FillData))
        {
            sc.GetOrderByOrderID(FillData.InternalOrderID, TradeOrder);

            File << "  {"
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
                 << "\"OrderType\":\"" << TradeOrder.OrderType << "\"}\n]";
        }
    }

    File.close();

    LastFillIndexProcessed = TotalFills;
}
