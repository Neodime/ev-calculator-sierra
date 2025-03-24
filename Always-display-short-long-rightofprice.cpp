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
        sc.AutoLoop = 1;      // Process each bar automatically.
        sc.GraphRegion = 0;   // Overlay on the main price graph.

        // Trade Direction: "Short" means red box above current price, "Long" means red box below.
        TradeDirection.Name = "Trade Direction";
        TradeDirection.SetCustomInputStrings("Short;Long");
        TradeDirection.SetCustomInputIndex(0);  // Default: Short.

        // Red box height in price units.
        sc.Input[1].Name = "Red Box Height (Price Units)";
        sc.Input[1].SetFloat(1.0f);

        // Green box height in price units.
        sc.Input[2].Name = "Green Box Height (Price Units)";
        sc.Input[2].SetFloat(1.0f);

        // Box width in bars.
        sc.Input[3].Name = "Box Width (Bars)";
        sc.Input[3].SetInt(2);  // Default: 2 bars wide.

        return;
    }

    // Draw the boxes only on the rightmost bar so they always appear to the right of the price.
    if (sc.Index != sc.ArraySize - 1)
        return;

    bool IsShort = (TradeDirection.GetIndex() == 0);  // 0 = Short, 1 = Long.
    float CurrentPrice = sc.Close[sc.Index];

    float RedBoxHeight = sc.Input[1].GetFloat();
    float GreenBoxHeight = sc.Input[2].GetFloat();
    int BoxWidthBars = sc.Input[3].GetInt();

    float redPriceTop, redPriceBottom, greenPriceTop, greenPriceBottom;
    if (IsShort)
    {
        // For Short: red box is drawn above the current price, green below.
        redPriceBottom = CurrentPrice;
        redPriceTop = CurrentPrice + RedBoxHeight;
        greenPriceTop = CurrentPrice;
        greenPriceBottom = CurrentPrice - GreenBoxHeight;
    }
    else
    {
        // For Long: red box is drawn below the current price, green above.
        redPriceTop = CurrentPrice;
        redPriceBottom = CurrentPrice - RedBoxHeight;
        greenPriceBottom = CurrentPrice;
        greenPriceTop = CurrentPrice + GreenBoxHeight;
    }

    // Draw the red box.
    // sc.DrawRectangle parameters: (UniqueID, BeginIndex, BeginValue, EndIndex, EndValue, Color, Transparency, LineWidth)
    // BeginValue should be the higher price (top) and EndValue the lower (bottom).
    sc.DrawRectangle(1, sc.ArraySize - 1, redPriceTop, sc.ArraySize - 1 + BoxWidthBars, redPriceBottom, RGB(255,150,150), 0, 1);

    // Draw the green box.
    sc.DrawRectangle(2, sc.ArraySize - 1, greenPriceTop, sc.ArraySize - 1 + BoxWidthBars, greenPriceBottom, RGB(150,255,150), 0, 1);
}

