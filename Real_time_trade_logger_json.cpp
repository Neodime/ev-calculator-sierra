#include "sierrachart.h"
#include <fstream>

SCDLLName("RealTime Trade Logger")

// Persistent variable index
const int LAST_PROCESSED_FILL_INDEX = 0;

SCSFExport scsf_RealTimeTradeLogger(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphName = "Real-Time Trade Logger";
        sc.StudyDescription = "Logs trades from Order Fill Data to a JSON file.";

        sc.AutoLoop = 0;  // Manual looping
        sc.UpdateAlways = 1;
        sc.ReceivePointerEvents = 0;

        return;
    }

    // File path
    const char* FilePath = "C:\\Users\\timog\\Documents\\Trading\\Sierra Jason Trades\\trade_log.json";

    int& LastFillIndexProcessed = sc.GetPersistentInt(LAST_PROCESSED_FILL_INDEX);

    int TotalFills = sc.GetOrderFillArraySize();

    bool FirstEntry = (LastFillIndexProcessed == 0);

    std::ofstream File;
    File.open(FilePath, std::ios::app);

    if (!File.is_open())
    {
        sc.AddMessageToLog("Error: Cannot open trade log file.", 1);
        return;
    }

    s_SCOrderFillData FillData;

    // Fetch and process new fills
    for (int FillIndex = LastFillIndexProcessed; FillIndex < TotalFills; FillIndex++)
    {
        if (sc.GetOrderFillEntry(FillIndex, FillData))
        {
            if (!FirstEntry)
                File << ",\n";
            else
                FirstEntry = false;

            File << "{"
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
                << "\"ServiceOrderID\":\"" << FillData.ServiceOrderID << "\"}"
                ;
        }
    }

    File.close();

    LastFillIndexProcessed = TotalFills;
}
