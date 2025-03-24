#include "sierrachart.h"

SCDLLName("Dynamic SL/TP Lines")

// Enumeration for easier reading of Position Direction
enum PositionDirectionEnum
{
    LONG_POSITION = 0,
    SHORT_POSITION = 1
};

SCSFExport scsf_DynamicLines(SCStudyInterfaceRef sc)
{
    if (sc.SetDefaults)
    {
        sc.GraphName = "Dynamic SL/TP Lines";

        sc.AutoLoop = 1;
        sc.GraphRegion = 0;

        sc.Input[0].Name = "Position Direction";
        sc.Input[0].SetCustomInputStrings("Long;Short");
        sc.Input[0].SetCustomInputIndex(0);

        sc.Input[1].Name = "SL Distance (USD)";
        sc.Input[1].SetFloat(200.0f);

        sc.Input[2].Name = "TP Distance (USD)";
        sc.Input[2].SetFloat(200.0f);

        return;
    }

    float currentPrice = sc.Close[sc.Index];

    int positionDirection = sc.Input[0].GetIndex();
    float slDistance = sc.Input[1].GetFloat();
    float tpDistance = sc.Input[2].GetFloat();

    int lineStartBar = sc.Index + 1;
    int lineEndBar = sc.Index + 10;

    float slPrice, tpPrice;
    COLORREF slColor = RGB(255, 182, 193); // Light Red
    COLORREF tpColor = RGB(144, 238, 144); // Light Green

    if (positionDirection == LONG_POSITION)
    {
        tpPrice = currentPrice + tpDistance;
        slPrice = currentPrice - slDistance;
    }
    else // SHORT_POSITION
    {
        slPrice = currentPrice + slDistance;
        tpPrice = currentPrice - tpDistance;
    }

    s_UseTool slLine;
    slLine.Clear();
    slLine.DrawingType = DRAWING_LINE;
    slLine.BeginIndex = lineStartBar;
    slLine.EndIndex = lineEndBar;
    slLine.BeginValue = slPrice;
    slLine.EndValue = slPrice;
    slLine.Color = slColor;
    slLine.LineWidth = 2;
    slLine.AddMethod = UTAM_ADD_OR_ADJUST;
    slLine.LineNumber = 10001;
    sc.UseTool(slLine);

    s_UseTool tpLine;
    tpLine.Clear();
    tpLine.DrawingType = DRAWING_LINE;
    tpLine.BeginIndex = lineStartBar;
    tpLine.EndIndex = lineEndBar;
    tpLine.BeginValue = tpPrice;
    tpLine.EndValue = tpPrice;
    tpLine.Color = tpColor;
    tpLine.LineWidth = 2;
    tpLine.AddMethod = UTAM_ADD_OR_ADJUST;
    tpLine.LineNumber = 10002;
    sc.UseTool(tpLine);
}
