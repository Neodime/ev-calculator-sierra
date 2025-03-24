#include "sierrachart.h"

SCDLLName("Always Display Short/Long Right of Price");

SCSFExport scsf_CustomBoxStudy(SCStudyInterfaceRef sc)
{
    // Define study inputs on first run.
    SCInputRef TradeDirection = sc.Input[0];
    if (sc.SetDefaults)
    {
        sc.GraphName = "Always Display Short/Long Right of Price";
        sc.StudyDescription = "Displays a filled red and green rectangle on the right side of the price. Their placement depends on the Trade Direction input (Short: red above, green below; Long: red below, green above).";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0; // Overlay on the main price graph

        // Trade Direction: "Short" means red box will be drawn above current price, "Long" means red box below.
        TradeDirection.Name = "Trade Direction";
        TradeDirection.SetCustomInputStrings("Short;Long");
        TradeDirection.SetCustomInputIndex(0); // Default: Short

        // Red box height in price units.
        sc.Input[1].Name = "Red Box Height (Price Units)";
        sc.Input[1].SetFloat(1.0f);

        // Green box height in price units.
        sc.Input[2].Name = "Green Box Height (Price Units)";
        sc.Input[2].SetFloat(1.0f);

        // Box width in pixels.
        sc.Input[3].Name = "Box Width (Pixels)";
        sc.Input[3].SetInt(20);

        return;
    }

    // Only draw on the last bar so the rectangles appear on the right of the price.
    if (sc.Index != sc.ArraySize - 1)
        return;

    bool IsShort = (TradeDirection.GetIndex() == 0);  // 0 = Short, 1 = Long.
    float CurrentPrice = sc.Close[sc.Index];

    float RedBoxHeight = sc.Input[1].GetFloat();
    float GreenBoxHeight = sc.Input[2].GetFloat();
    int BoxWidth = sc.Input[3].GetInt();

    // Determine the top and bottom price for each box based on Trade Direction.
    float redPriceTop, redPriceBottom, greenPriceTop, greenPriceBottom;
    if (IsShort)
    {
        // Short: red box above current price, green box below.
        redPriceBottom = CurrentPrice;
        redPriceTop = CurrentPrice + RedBoxHeight;
        greenPriceTop = CurrentPrice;
        greenPriceBottom = CurrentPrice - GreenBoxHeight;
    }
    else
    {
        // Long: red box below current price, green box above.
        redPriceTop = CurrentPrice;
        redPriceBottom = CurrentPrice - RedBoxHeight;
        greenPriceBottom = CurrentPrice;
        greenPriceTop = CurrentPrice + GreenBoxHeight;
    }

    // Convert the x-coordinate of the last bar to a pixel coordinate.
    int leftRect = sc.GetXFromBarIndex(sc.ArraySize - 1);
    int rightRect = leftRect + BoxWidth;

    // Convert price levels to pixel y-coordinates.
    int redTopPixel = sc.GetYFromPrice(redPriceTop);
    int redBottomPixel = sc.GetYFromPrice(redPriceBottom);
    int redTop = (redTopPixel < redBottomPixel ? redTopPixel : redBottomPixel);
    int redBottom = (redTopPixel < redBottomPixel ? redBottomPixel : redTopPixel);

    int greenTopPixel = sc.GetYFromPrice(greenPriceTop);
    int greenBottomPixel = sc.GetYFromPrice(greenPriceBottom);
    int greenTop = (greenTopPixel < greenBottomPixel ? greenTopPixel : greenBottomPixel);
    int greenBottom = (greenTopPixel < greenBottomPixel ? greenBottomPixel : greenTopPixel);

    // Draw filled rectangles using documented ACSIL functions.
    n_ACSIL::s_GraphicsRectangle rect;
    n_ACSIL::s_GraphicsBrush brush;

    // Draw the red rectangle.
    rect.Left = leftRect;
    rect.Top = redTop;
    rect.Right = rightRect;
    rect.Bottom = redBottom;
    brush.Color = RGB(255, 150, 150);  // Light red.
    sc.FillRectangle(rect, brush);

    // Draw the green rectangle.
    rect.Left = leftRect;
    rect.Top = greenTop;
    rect.Right = rightRect;
    rect.Bottom = greenBottom;
    brush.Color = RGB(150, 255, 150);  // Light green.
    sc.FillRectangle(rect, brush);
}
