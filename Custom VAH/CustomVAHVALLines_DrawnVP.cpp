#include "sierrachart.h"

SCDLLName("Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name")

// Helper Function: DeleteVAHVALLines
// Deletes the drawn VAH and VAL lines (if they exist) by configuring an s_UseTool
// structure with SZM_DELETE and calling sc.UseTool with the appropriate line number.
void DeleteVAHVALLines(SCStudyInterfaceRef sc)
{
    // Retrieve persistent line IDs as integers.
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);
    
    s_UseTool Tool;
    memset(&Tool, 0, sizeof(Tool));
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_HORIZONTALLINE;
    Tool.AddMethod = SZM_DELETE;  // Use SZM_DELETE to delete the drawing tool.
    Tool.Region = sc.GraphRegion;
    Tool.BeginIndex = 0;
    Tool.EndIndex = sc.ArraySize - 1;

    // Delete the VAH line if it exists.
    if (pLineID_VAH != 0)
    {
        Tool.LineNumber = pLineID_VAH;
        sc.UseTool(Tool);
        sc.SetPersistentInt(1, 0);
    }

    // Delete the VAL line if it exists.
    if (pLineID_VAL != 0)
    {
        Tool.LineNumber = pLineID_VAL;
        sc.UseTool(Tool);
        sc.SetPersistentInt(2, 0);
    }
}

// Study: CustomVAHVALLines_RevisedWithVPShortName
// Retrieves VAH (subgraph index 3) and VAL (subgraph index 4) from a user-specified VP study,
// then draws white, dashed horizontal lines at these levels. The lines are updated at a user-defined refresh interval.
SCSFExport scsf_CustomVAHVALLines_RevisedWithVPShortName(SCStudyInterfaceRef sc)
{
    // Input Definitions
    SCInputRef RefreshInterval    = sc.Input[0];
    SCInputRef DisplayCompletedVP = sc.Input[1]; // Option to display lines even if VP is completed.
    SCInputRef VPStudyShortName   = sc.Input[2]; // User-specified short name of the VP study.

    if (sc.SetDefaults)
    {
        sc.GraphName = "Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name";
        sc.StudyDescription = "Retrieves VAH (SG3) and VAL (SG4) from a VP study (specified by short name) and draws white dashed horizontal lines.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        // Input: Refresh Interval (number of bars between updates)
        RefreshInterval.Name = "Refresh Interval (Bars)";
        RefreshInterval.SetInt(1);
        RefreshInterval.SetIntLimits(1, 1000);

        // Input: Option to display lines even when VP is completed
        DisplayCompletedVP.Name = "Display Lines When VP Completed";
        DisplayCompletedVP.SetYesNo(0); // Default is No.

        // Input: VP Study Short Name (default "Volume Profile")
        VPStudyShortName.Name = "VP Study Short Name";
        VPStudyShortName.SetString("Volume Profile");

        // Initialize persistent integers for storing drawn line IDs.
        sc.SetPersistentInt(1, 0);  // VAH line ID.
        sc.SetPersistentInt(2, 0);  // VAL line ID.

        return;
    }

    // Ensure sc.Index is within valid range.
    if (sc.Index < 0 || sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of valid range.", 1);
        return;
    }

    // Refresh Control: Update only every X bars.
    int refreshInterval = RefreshInterval.GetInt();
    if ((sc.Index % refreshInterval) != 0)
        return;

    // Retrieve the VP study using the user-specified short name.
    int vpStudyID = sc.GetStudyIDByName(sc.ChartNumber, VPStudyShortName.GetString(), 0);
    if (vpStudyID == 0)
    {
        char logBuffer[256];
        sprintf(logBuffer, "Warning: VP study with short name '%s' not found on chart.", VPStudyShortName.GetString());
        sc.AddMessageToLog(logBuffer, 1);
        DeleteVAHVALLines(sc);
        return;
    }

    // Retrieve VAH and VAL arrays from the VP study (subgraph indices 3 and 4).
    SCFloatArrayRef vpVAHArray = sc.GetStudyArrayUsingID(vpStudyID, 3);
    SCFloatArrayRef vpVALArray = sc.GetStudyArrayUsingID(vpStudyID, 4);
    if (vpVAHArray.GetArray() == nullptr || vpVALArray.GetArray() == nullptr)
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

    // Determine if the VP is "Developing" (current bar is the last bar on the chart).
    bool vpDeveloping = (sc.Index == sc.ArraySize - 1);
    if (!vpDeveloping && DisplayCompletedVP.GetYesNo() == 0)
    {
        sc.AddMessageToLog("Info: VP is completed; VAH/VAL lines will not be displayed.", 0);
        DeleteVAHVALLines(sc);
        return;
    }

    // Log current VAH and VAL values.
    char debugBuffer[256];
    sprintf(debugBuffer, "Updating VAH/VAL lines: VAH = %.2f, VAL = %.2f", currentVAH, currentVAL);
    sc.AddMessageToLog(debugBuffer, 0);

    // Retrieve persistent storage for line IDs.
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);

    // Prepare the drawing tool for horizontal lines.
    s_UseTool Tool;
    memset(&Tool, 0, sizeof(Tool));
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_HORIZONTALLINE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.Region = sc.GraphRegion;
    Tool.BeginIndex = 0;
    Tool.EndIndex = sc.ArraySize - 1;
    Tool.Color = RGB(255, 255, 255);      // White.
    Tool.LineStyle = LINESTYLE_DASH;      // Dashed.
    Tool.LineWidth = 2;

    // Draw or update the VAH horizontal line.
    Tool.BeginValue = currentVAH;
    Tool.EndValue = currentVAH;
    Tool.LineNumber = (pLineID_VAH != 0) ? pLineID_VAH : 0;
    sc.UseTool(Tool);
    sc.SetPersistentInt(1, Tool.LineNumber);

    // Draw or update the VAL horizontal line.
    Tool.BeginValue = currentVAL;
    Tool.EndValue = currentVAL;
    Tool.LineNumber = (pLineID_VAL != 0) ? pLineID_VAL : 0;
    sc.UseTool(Tool);
    sc.SetPersistentInt(2, Tool.LineNumber);
}
