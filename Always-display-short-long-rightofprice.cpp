#include "sierrachart.h"

SCDLLName("Always Display Short/Long Right of Price");

SCSFExport scsf_CustomBoxStudy(SCStudyInterfaceRef sc)
{
    // Define study inputs on first run.
    SCInputRef TradeDirection = sc.Input[0];
    if (sc.SetDefaults)
    {
        sc.GraphName = "Always Display Short/Long Right of Price";
        sc.StudyDescription = "Displays a red and a green box on the right side of the price. Their placement depends on Trade Direction (Short: red above, green below; Long: red below, green above).";
        sc.AutoLoop = 1;         // Process each bar automatically.
        sc.GraphRegion = 0;      // Overlay on the main price graph.

        // Trade Direction: "Short" means red box will be above current price; "Long" means red box will be below.
        TradeDirection.Name = "Trade Direction";
        TradeDirection.SetCustomInputStrings("Short;Long");
        TradeDirection.SetCustomInputIndex(0);  // Default: Short.

        // Red box height (in price units).
        sc.Input[1].Name = "Red Box Height (Price Units)";
        sc.Input[1].SetFloat(1.0f);

        // Green box height (in price units).
        sc.Input[2].Name = "Green Box Height (Price Units)";
        sc.Input[2].SetFloat(1.0f);

        // Box width in bars (since pixel based drawing is not available here).
        sc.Input[3].Name = "Box Width (Bars)";
        sc.Input[3].SetInt(2);  // Default: 2 bars wide.

        return;
    }

    // Draw the boxes only on the last bar so they appear on the right side of the price.
    if (sc.Index != sc.ArraySize - 1)
        return;

    bool IsShort = (TradeDirection.GetIndex() == 0);  // 0 = Short, 1 = Long.
    float CurrentPrice = sc.Close[sc.Index];

    float RedBoxHeight = sc.Input[1].GetFloat();
    float GreenBoxHeight = sc.Input[2].GetFloat();
    int BoxWidthBars = sc.Input[3].GetInt();

    // Determine the price boundaries for each box.
    float redPriceTop, redPriceBottom, greenPriceTop, greenPriceBottom;
    if (IsShort)
    {
        // For Short: red box is drawn above the price; green box is drawn below.
        redPriceBottom = CurrentPrice;
        redPriceTop = CurrentPrice + RedBoxHeight;
        greenPriceTop = CurrentPrice;
        greenPriceBottom = CurrentPrice - GreenBoxHeight;
    }
    else
    {
        // For Long: red box is drawn below the price; green box is drawn above.
        redPriceTop = CurrentPrice;
        redPriceBottom = CurrentPrice - RedBoxHeight;
        greenPriceBottom = CurrentPrice;
        greenPriceTop = CurrentPrice + GreenBoxHeight;
    }

    // Draw the red box using a rectangle drawing tool.
    s_UseTool Tool;
    Tool.Clear();
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_RECTANGLE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    // Set horizontal coordinates using bar indices.
    Tool.BeginIndex = sc.ArraySize - 1;                // Left edge on the last bar.
    Tool.EndIndex = sc.ArraySize - 1 + BoxWidthBars;     // Extend the rectangle to the right.
    // For rectangles, BeginValue should be the higher price (top) and EndValue the lower (bottom).
    Tool.BeginValue = (redPriceTop > redPriceBottom ? redPriceTop : redPriceBottom);
    Tool.EndValue   = (redPriceTop > redPriceBottom ? redPriceBottom : redPriceTop);
    Tool.Color = RGB(255, 150, 150); // Light red.
    sc.UseTool(Tool);

    // Draw the green box.
    Tool.Clear();
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_RECTANGLE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.BeginIndex = sc.ArraySize - 1;
    Tool.EndIndex = sc.ArraySize - 1 + BoxWidthBars;
    Tool.BeginValue = (greenPriceTop > greenPriceBottom ? greenPriceTop : greenPriceBottom);
    Tool.EndValue   = (greenPriceTop > greenPriceBottom ? greenPriceBottom : greenPriceTop);
    Tool.Color = RGB(150, 255, 150); // Light green.
    sc.UseTool(Tool);
}
