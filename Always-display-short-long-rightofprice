// Example ACSIL Study: Custom Box Display
#include "sierrachart.h"

SCDLLName("Custom Box Study");

SCSFExport scsf_CustomBoxStudy(SCStudyInterfaceRef sc)
{
    // Define inputs on first run (SetDefaults)
    SCInputRef TradeDirection = sc.Input[0];
    if (sc.SetDefaults)
    {
        sc.GraphName = "Custom Box Study";
        sc.StudyDescription = "Displays two small boxes on the right of the chart. The red box is placed above or below the current price and the green box is placed on the opposite side depending on the selected Trade Direction.";
        sc.AutoLoop = 0;  // We'll update our drawings manually
        sc.GraphRegion = 0;

        // Trade direction: Choose "Short" (red above, green below) or "Long" (red below, green above)
        TradeDirection.Name = "Trade Direction";
        TradeDirection.SetCustomInputStrings("Short;Long");
        TradeDirection.SetCustomInputIndex(0); // default = Short

        // Input for the red box height (in price terms)
        sc.Input[1].Name = "Red Box Height (Price Units)";
        sc.Input[1].SetFloat(1.0);

        // Input for the green box height (in price terms)
        sc.Input[2].Name = "Green Box Height (Price Units)";
        sc.Input[2].SetFloat(1.0);

        // Fixed box width in pixels (adjust as desired)
        sc.Input[3].Name = "Box Width (Pixels)";
        sc.Input[3].SetInt(20);

        return;
    }

    // Retrieve input values
    bool IsShort = (TradeDirection.GetIndex() == 0);  // 0 = Short, 1 = Long
    float RedHeight = sc.Input[1].GetFloat();
    float GreenHeight = sc.Input[2].GetFloat();
    int BoxWidth = sc.Input[3].GetInt();

    // Get chart dimensions (for x coordinate placement)
    int ChartWidth = sc.GetChartWindowWidth();
    // Place the boxes near the right edge with a small right margin (e.g. 2 pixels)
    int xPos = ChartWidth - BoxWidth - 2;

    // Use the most recent price (you can change this logic if needed)
    int lastIndex = sc.ArraySize - 1;
    float CurrentPrice = sc.Close[lastIndex];

    // Determine the top and bottom price for each box based on the Trade Direction.
    // For Short:
    //    red box: from CurrentPrice up by RedHeight, green box: from CurrentPrice down by GreenHeight.
    // For Long:
    //    red box: from CurrentPrice down by RedHeight, green box: from CurrentPrice up by GreenHeight.
    float redPriceTop, redPriceBottom, greenPriceTop, greenPriceBottom;
    if (IsShort)
    {
        redPriceBottom = CurrentPrice;
        redPriceTop = CurrentPrice + RedHeight;
        greenPriceTop = CurrentPrice;
        greenPriceBottom = CurrentPrice - GreenHeight;
    }
    else
    {
        redPriceTop = CurrentPrice;
        redPriceBottom = CurrentPrice - RedHeight;
        greenPriceBottom = CurrentPrice;
        greenPriceTop = CurrentPrice + GreenHeight;
    }

    // Convert these price levels to Y pixel coordinates using Sierra Chart’s scaling.
    int redTopPixel, redBottomPixel, greenTopPixel, greenBottomPixel;
    sc.ScalePriceToYPixel(redPriceTop, redTopPixel);
    sc.ScalePriceToYPixel(redPriceBottom, redBottomPixel);
    sc.ScalePriceToYPixel(greenPriceTop, greenTopPixel);
    sc.ScalePriceToYPixel(greenPriceBottom, greenBottomPixel);

    // Determine rectangle coordinates (top is the smaller pixel value since (0,0) is the top-left)
    int redRectTop    = min(redTopPixel, redBottomPixel);
    int redRectBottom = max(redTopPixel, redBottomPixel);
    int greenRectTop    = min(greenTopPixel, greenBottomPixel);
    int greenRectBottom = max(greenTopPixel, greenBottomPixel);

    // Prepare to draw the red and green boxes using chart relative (pixel) coordinates.
    s_UseTool Tool;
    
    // ----- Draw the Red Box -----
    Tool.Clear();
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_RECTANGLE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.DrawingIndex = 1;
    Tool.UseChartRelativeCoordinates = 1; // coordinates in pixels relative to chart area
    Tool.BeginPoint.x = xPos;
    Tool.BeginPoint.y = redRectTop;
    Tool.EndPoint.x   = xPos + BoxWidth;
    Tool.EndPoint.y   = redRectBottom;
    // Set a light red color
    Tool.Color = RGB(255, 150, 150);
    sc.UseTool(Tool);

    // ----- Draw the Green Box -----
    Tool.Clear();
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_RECTANGLE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.DrawingIndex = 2;
    Tool.UseChartRelativeCoordinates = 1;
    Tool.BeginPoint.x = xPos;
    Tool.BeginPoint.y = greenRectTop;
    Tool.EndPoint.x   = xPos + BoxWidth;
    Tool.EndPoint.y   = greenRectBottom;
    // Set a light green color
    Tool.Color = RGB(150, 255, 150);
    sc.UseTool(Tool);
}

