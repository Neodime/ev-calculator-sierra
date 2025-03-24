#include "sierrachart.h"

SCDLLName("Always Display Short/Long Right of Price");

SCSFExport scsf_CustomBoxStudy(SCStudyInterfaceRef sc)
{
    // Define study inputs on first run.
    SCInputRef TradeDirection = sc.Input[0];
    if (sc.SetDefaults)
    {
        sc.GraphName = "Always Display Short/Long Right of Price";
        sc.StudyDescription = "Displays two boxes (red and green) to the right of the current price. For Trade Direction Short, the red box is above and the green below the price; for Long, the red box is below and the green above.";
        sc.AutoLoop = 1;      // Process each bar automatically.
        sc.GraphRegion = 0;   // Overlay on the main price chart.

        // Trade Direction: "Short" means red box above current price; "Long" means red box below.
        TradeDirection.Name = "Trade Direction";
        TradeDirection.SetCustomInputStrings("Short;Long");
        TradeDirection.SetCustomInputIndex(0);  // Default: Short.

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

    // Draw the boxes only on the last bar so they always appear at the right of the chart.
    if (sc.Index != sc.ArraySize - 1)
        return;

    bool bIsShort = (TradeDirection.GetIndex() == 0);  // 0 = Short, 1 = Long.
    float CurrentPrice = sc.Close[sc.Index];

    float RedBoxHeight = sc.Input[1].GetFloat();
    float GreenBoxHeight = sc.Input[2].GetFloat();
    int BoxWidth = sc.Input[3].GetInt();

    // Determine the price boundaries for each box.
    float redPriceTop, redPriceBottom, greenPriceTop, greenPriceBottom;
    if (bIsShort)
    {
        // For Short: red box above the price; green box below.
        redPriceBottom = CurrentPrice;
        redPriceTop = CurrentPrice + RedBoxHeight;
        greenPriceTop = CurrentPrice;
        greenPriceBottom = CurrentPrice - GreenBoxHeight;
    }
    else
    {
        // For Long: red box below the price; green box above.
        redPriceTop = CurrentPrice;
        redPriceBottom = CurrentPrice - RedBoxHeight;
        greenPriceBottom = CurrentPrice;
        greenPriceTop = CurrentPrice + GreenBoxHeight;
    }

    // Convert the x-coordinate of the last bar to a pixel coordinate.
    int leftX = sc.GetXFromBarIndex(sc.ArraySize - 1);
    int rightX = leftX + BoxWidth;

    // Convert price levels to pixel y-coordinates.
    int redTopPixel = sc.GetYFromPrice(redPriceTop);
    int redBottomPixel = sc.GetYFromPrice(redPriceBottom);
    // Determine the top (smaller pixel value) and bottom.
    int redTop = (redTopPixel < redBottomPixel ? redTopPixel : redBottomPixel);
    int redBottom = (redTopPixel < redBottomPixel ? redBottomPixel : redTopPixel);

    int greenTopPixel = sc.GetYFromPrice(greenPriceTop);
    int greenBottomPixel = sc.GetYFromPrice(greenPriceBottom);
    int greenTop = (greenTopPixel < greenBottomPixel ? greenTopPixel : greenBottomPixel);
    int greenBottom = (greenTopPixel < greenBottomPixel ? greenBottomPixel : greenTopPixel);

    // Create a graphics rectangle and brush for the red box.
    n_ACSIL::s_GraphicsRectangle redRect;
    redRect.Left = leftX;
    redRect.Top = redTop;
    redRect.Right = rightX;
    redRect.Bottom = redBottom;

    n_ACSIL::s_GraphicsBrush redBrush;
    redBrush.Color = RGB(255, 150, 150);  // Light red.

    // Fill the red rectangle.
    sc.FillRectangle(redRect, redBrush);

    // Create a graphics rectangle and brush for the green box.
    n_ACSIL::s_GraphicsRectangle greenRect;
    greenRect.Left = leftX;
    greenRect.Top = greenTop;
    greenRect.Right = rightX;
    greenRect.Bottom = greenBottom;

    n_ACSIL::s_GraphicsBrush greenBrush;
    greenBrush.Color = RGB(150, 255, 150);  // Light green.

    // Fill the green rectangle.
    sc.FillRectangle(greenRect, greenBrush);
}
