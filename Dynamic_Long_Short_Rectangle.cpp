#include "sierrachart.h"

SCDLLName("Dynamic Rectangles Long Short")

// Enumeration for easier reading of Position Direction
enum PositionDirectionEnum
{
    LONG_POSITION = 0,
    SHORT_POSITION = 1
};

SCSFExport scsf_DynamicRectangles(SCStudyInterfaceRef sc)
{
    // Define inputs and persistent data once
    if (sc.SetDefaults)
    {
        sc.GraphName = "Dynamic Long/Short Rectangles";

        sc.AutoLoop = 1;
        sc.GraphRegion = 0;

        // User settings
        sc.Input[0].Name = "Position Direction";
        sc.Input[0].SetCustomInputStrings("Long;Short");
        sc.Input[0].SetCustomInputIndex(0);

        sc.Input[1].Name = "Long Rectangle Height (USD)";
        sc.Input[1].SetFloat(200.0f);

        sc.Input[2].Name = "Short Rectangle Height (USD)";
        sc.Input[2].SetFloat(200.0f);

        return;
    }

    // Get current price
    float currentPrice = sc.Close[sc.Index];

    // Get user-selected settings
    int positionDirection = sc.Input[0].GetIndex();
    float longHeight = sc.Input[1].GetFloat();
    float shortHeight = sc.Input[2].GetFloat();

    // Define rectangle positions (to the right of current price)
    int rectStartBar = sc.Index;
    int rectEndBar = sc.Index + 10; // Rectangle length of 10 bars, adjustable if needed

    float upperRectTop, upperRectBottom, lowerRectTop, lowerRectBottom;
    COLORREF upperRectColor, lowerRectColor;

    if (positionDirection == LONG_POSITION)
    {
        upperRectTop = currentPrice + longHeight;
        upperRectBottom = currentPrice;
        lowerRectTop = currentPrice;
        lowerRectBottom = currentPrice - shortHeight;

        upperRectColor = RGB(0, 255, 0); // Green
        lowerRectColor = RGB(255, 0, 0); // Red
    }
    else // SHORT_POSITION
    {
        upperRectTop = currentPrice + shortHeight;
        upperRectBottom = currentPrice;
        lowerRectTop = currentPrice;
        lowerRectBottom = currentPrice - longHeight;

        upperRectColor = RGB(255, 0, 0); // Red
        lowerRectColor = RGB(0, 255, 0); // Green
    }

    // Draw upper rectangle
    s_UseTool upperRect;
    upperRect.Clear();
    upperRect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
    upperRect.BeginIndex = rectStartBar;
    upperRect.EndIndex = rectEndBar;
    upperRect.BeginValue = upperRectTop;
    upperRect.EndValue = upperRectBottom;
    upperRect.Color = upperRectColor;
    upperRect.TransparencyLevel = 70;
    upperRect.LineWidth = 1;
    upperRect.AddMethod = UTAM_ADD_OR_ADJUST;
    upperRect.LineNumber = 10001;
    sc.UseTool(upperRect);

    // Draw lower rectangle
    s_UseTool lowerRect;
    lowerRect.Clear();
    lowerRect.DrawingType = DRAWING_RECTANGLE_EXT_HIGHLIGHT;
    lowerRect.BeginIndex = rectStartBar;
    lowerRect.EndIndex = rectEndBar;
    lowerRect.BeginValue = lowerRectTop;
    lowerRect.EndValue = lowerRectBottom;
    lowerRect.Color = lowerRectColor;
    lowerRect.TransparencyLevel = 70;
    lowerRect.LineWidth = 1;
    lowerRect.AddMethod = UTAM_ADD_OR_ADJUST;
    lowerRect.LineNumber = 10002;
    sc.UseTool(lowerRect);
}

