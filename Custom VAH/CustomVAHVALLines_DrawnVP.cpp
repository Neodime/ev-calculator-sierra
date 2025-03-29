#include "sierrachart.h"

SCDLLName("Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name")

// Helper Function: DeleteVAHVALLines
// This function deletes the drawn VAH and VAL lines if they exist,
// resetting their persistent storage values to 0.
void DeleteVAHVALLines(SCStudyInterfaceRef sc)
{
    // Retrieve persistent line IDs as integers (not pointers)
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);

    // If the VAH line exists, delete it using sc.DeleteDrawingTool and reset its ID.
    if (pLineID_VAH != 0)
    {
        sc.DeleteDrawingTool(pLineID_VAH);
        sc.SetPersistentInt(1, 0);
    }
    // If the VAL line exists, delete it using sc.DeleteDrawingTool and reset its ID.
    if (pLineID_VAL != 0)
    {
        sc.DeleteDrawingTool(pLineID_VAL);
        sc.SetPersistentInt(2, 0);
    }
}

// Study: CustomVAHVALLines_RevisedWithVPShortName
//
// This custom study retrieves the Value Area High (VAH) and Value Area Low (VAL)
// values from a drawn Volume Profile (VP) study whose short name is specified by the user.
// VAH is assumed to be in subgraph index 3 and VAL in subgraph index 4.
// White, dashed horizontal lines are drawn at these levels and updated based on a user-defined refresh interval.
// The study uses sc.GetStudyIDByName and sc.GetStudyArrayUsingID with an extra parameter,
// persistent variables as integers, and deletes drawing tools using sc.DeleteDrawingTool.
SCSFExport scsf_CustomVAHVALLines_RevisedWithVPShortName(SCStudyInterfaceRef sc)
{
    // Input Definitions
    SCInputRef RefreshInterval    = sc.Input[0];
    SCInputRef DisplayCompletedVP = sc.Input[1]; // Option to display lines even if VP is completed
    SCInputRef VPStudyShortName   = sc.Input[2]; // User-specified short name of the VP study

    if (sc.SetDefaults)
    {
        // Set Study Defaults
        sc.GraphName = "Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name";
        sc.StudyDescription = "Retrieves VAH (SG3) and VAL (SG4) from a VP study (specified by short name) and draws white dashed horizontal lines.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        // Input: Refresh Interval (number of bars between refresh updates)
        RefreshInterval.Name = "Refresh Interval (Bars)";
        RefreshInterval.SetInt(1);
        RefreshInterval.SetIntLimits(1, 1000);

        // Input: Option to display lines even when VP is completed
        DisplayCompletedVP.Name = "Display Lines When VP Completed";
        DisplayCompletedVP.SetYesNo(0); // Default is No (do not display lines if VP is completed)

        // Input: VP Study Short Name (default "Volume Profile")
        VPStudyShortName.Name = "VP Study Short Name";
        VPStudyShortName.SetString("Volume Profile");

        // Initialize persistent integers for storing drawn line IDs.
        sc.SetPersistentInt(1, 0);  // For VAH line
        sc.SetPersistentInt(2, 0);  // For VAL line

        return;
    }

    // Defensive Check: Ensure sc.Index is within the valid range.
    if (sc.Index < 0 || sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of valid range.", 1);
        return;
    }

    // Refresh Control: Update only every X bars as defined by the refresh interval.
    int refreshInterval = RefreshInterval.GetInt();
    if ((sc.Index % refreshInterval) != 0)
        return;

    // Retrieve the VP study using the user-specified short name.
    // Use sc.GetStudyIDByName (instead of the non-existent sc.GetStudyByName).
    int vpStudyID = sc.GetStudyIDByName(sc.ChartNumber, VPStudyShortName.GetString(), 0);
    if (vpStudyID == 0)
    {
        // Build the log message using sprintf (C-style formatting) to avoid std::string usage.
        char logBuffer[256];
        sprintf(logBuffer, "Warning: VP study with short name '%s' not found on chart.", VPStudyShortName.GetString());
        sc.AddMessageToLog(logBuffer, 1);
        DeleteVAHVALLines(sc);
        return;
    }

    // Retrieve VAH and VAL arrays from the VP study.
    // Add third parameter (0) as required by sc.GetStudyArrayUsingID.
    float* vpVAHArray = sc.GetStudyArrayUsingID(vpStudyID, 3, 0);
    float* vpVALArray = sc.GetStudyArrayUsingID(vpStudyID, 4, 0);
    if (vpVAHArray == nullptr || vpVALArray == nullptr)
    {
        sc.AddMessageToLog("Error: VAH/VAL arrays not found in the VP study.", 1);
        return;
    }

    // Defensive Check: Ensure sc.Index is within the bounds of the VP arrays.
    if (sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of bounds for the VP arrays.", 1);
        return;
    }

    // Retrieve current VAH and VAL values.
    float currentVAH = vpVAHArray[sc.Index];
    float currentVAL = vpVALArray[sc.Index];

    // Determine if the VP is "Developing".
    // Logic: The VP is considered developing if the current bar is the last bar on the chart.
    // This is based on the assumption that real-time updates occur only on the latest bar.
    bool vpDeveloping = (sc.Index == sc.ArraySize - 1);
    if (!vpDeveloping && DisplayCompletedVP.GetYesNo() == 0)
    {
        sc.AddMessageToLog("Info: VP is completed; VAH/VAL lines will not be displayed.", 0);
        DeleteVAHVALLines(sc);
        return;
    }

    // Debug Logging: Log the current VAH and VAL values.
    char debugBuffer[256];
    sprintf(debugBuffer, "Updating VAH/VAL lines: VAH = %.2f, VAL = %.2f", currentVAH, currentVAL);
    sc.AddMessageToLog(debugBuffer, 0);

    // Retrieve persistent storage for line IDs (as integers).
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);

    // Prepare the drawing tool for horizontal lines.
    s_UseTool Tool;
    memset(&Tool, 0, sizeof(Tool));
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_HORIZONTALLINE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.Region = sc.GraphRegion;
    // Draw the line across the entire chart.
    Tool.BeginIndex = 0;
    Tool.EndIndex = sc.ArraySize - 1;
    Tool.Color = RGB(255, 255, 255);      // White color
    Tool.LineStyle = LINESTYLE_DASH;      // Dashed line style
    Tool.LineWidth = 2;

    // Draw or update the VAH horizontal line.
    Tool.BeginValue = currentVAH;
    Tool.EndValue = currentVAH;
    // Use the existing line number if available, else 0.
    Tool.LineNumber = (pLineID_VAH != 0) ? pLineID_VAH : 0;
    sc.UseTool(Tool);
    // Save the updated line ID.
    sc.SetPersistentInt(1, Tool.LineNumber);

    // Draw or update the VAL horizontal line.
    Tool.BeginValue = currentVAL;
    Tool.EndValue = currentVAL;
    Tool.LineNumber = (pLineID_VAL != 0) ? pLineID_VAL : 0;
    sc.UseTool(Tool);
    // Save the updated line ID.
    sc.SetPersistentInt(2, Tool.LineNumber);
}
