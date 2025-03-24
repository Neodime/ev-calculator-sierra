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

        sc.Input[1].Name = "Upper Rectangle Height (USD)";
        sc.Input[1].SetFloat(200.0f);

        sc.Input[2].Name = "Lower Rectangle Height (USD)";
        sc.Input[2].SetFloat(200.0f);

        return;
    }

    // Get current price
    float currentPrice = sc.Close[sc.Index];

    // Get user-selected settings
    int positionDirection = sc.Input[0].GetIndex();
    float upperHeight = sc.Input[1].GetFloat();
    float lowerHeight = sc.Input[2].GetFloat();

    // Define rectangle positions (to the right of current price)
    int rectStartBar = sc.Index;
    int rectEndBar = sc.Index + 10; // Rectangle length of 10 bars, adjustable if needed

    // Upper rectangle coordinates
    float upperRectTop = currentPrice + upperHeight;
    float upperRectBottom = currentPrice;

    // Lower rectangle coordinates
    float lowerRectTop = currentPrice;
    float lowerRectBottom = currentPrice - lowerHeight;

    // Colors based on Position Direction selection
    COLORREF upperRectColor;
    COLORREF lowerRectColor;

    if (positionDirection == LONG_POSITION)
    {
        upperRectColor = RGB(0, 255, 0); // Green
        lowerRectColor = RGB(255, 0, 0); // Red
    }
    else // SHORT_POSITION
    {
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
    upperRect.UniqueID = 10001;
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
    lowerRect.UniqueID = 10002;
    sc.UseTool(lowerRect);
}
