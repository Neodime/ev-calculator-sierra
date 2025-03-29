#include "sierrachart.h"

SCDLLName("Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name - Throttled Warnings")

// Helper Function: DeleteVAHVALLines
// Deletes the drawn VAH and VAL lines (if they exist) by setting up an s_UseTool structure with SZM_DELETE
// and calling sc.UseTool with the appropriate line number. Logs the deletion process.
void DeleteVAHVALLines(SCStudyInterfaceRef sc)
{
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);

    // Log deletion if persistent IDs are valid.
    if (pLineID_VAH != 0 || pLineID_VAL != 0)
    {
        sc.AddMessageToLog("Deleting VAH/VAL drawing tools.", 0);
    }

    s_UseTool Tool;
    memset(&Tool, 0, sizeof(Tool));
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_HORIZONTALLINE;
    Tool.AddMethod = SZM_DELETE;  // Use SZM_DELETE to delete the drawing tool.
    Tool.Region = sc.GraphRegion;
    Tool.BeginIndex = 0;
    Tool.EndIndex = sc.ArraySize - 1;

    if (pLineID_VAH != 0)
    {
        Tool.LineNumber = pLineID_VAH;
        sc.UseTool(Tool);
        sc.SetPersistentInt(1, 0);
    }
    if (pLineID_VAL != 0)
    {
        Tool.LineNumber = pLineID_VAL;
        sc.UseTool(Tool);
        sc.SetPersistentInt(2, 0);
    }
}

// Study: CustomVAHVALLines_RevisedWithVPShortName
// Retrieves VAH (subgraph index 3) and VAL (subgraph index 4) from a VP study specified by its short name,
// then draws white, dashed horizontal lines at these levels. This version includes additional safety checks,
// throttled warning logging, and a longer default refresh interval.
SCSFExport scsf_CustomVAHVALLines_RevisedWithVPShortName(SCStudyInterfaceRef sc)
{
    // Input Definitions
    SCInputRef RefreshInterval    = sc.Input[0];
    SCInputRef DisplayCompletedVP = sc.Input[1]; // Option: display lines even if VP is completed.
    SCInputRef VPStudyShortName   = sc.Input[2]; // VP study short name.

    if (sc.SetDefaults)
    {
        sc.GraphName = "Custom VAH/VAL Lines with VP Study Short Name - Throttled Warnings";
        sc.StudyDescription = "Retrieves VAH (SG3) and VAL (SG4) from a VP study (specified by short name) and draws white dashed horizontal lines with safety checks and throttled warning logging.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;  // Ensure arrays are up-to-date.

        // Set default refresh interval to 10 bars (approximately 5 minutes on a 30-second chart).
        RefreshInterval.Name = "Refresh Interval (Bars)";
        RefreshInterval.SetInt(10);
        RefreshInterval.SetIntLimits(1, 1000);

        DisplayCompletedVP.Name = "Display Lines When VP Completed";
        DisplayCompletedVP.SetYesNo(0);  // Default: do not display if VP is completed.

        VPStudyShortName.Name = "VP Study Short Name";
        VPStudyShortName.SetString("Volume Profile");

        // Persistent indices:
        // Index 1: VAH line ID.
        // Index 2: VAL line ID.
        // Index 3: Last bar index when VP missing warning was logged.
        sc.SetPersistentInt(1, 0);
        sc.SetPersistentInt(2, 0);
        sc.SetPersistentInt(3, 0);

        return;
    }

    // Check that sc.Index is within valid range.
    if (sc.Index < 0 || sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of valid range.", 1);
        return;
    }

    int refreshInterval = RefreshInterval.GetInt();
    if ((sc.Index % refreshInterval) != 0)
        return;

    // Retrieve the VP study using the user-specified short name.
    int vpStudyID = sc.GetStudyIDByName(sc.ChartNumber, VPStudyShortName.GetString(), 0);
    if (vpStudyID == 0)
    {
        // Throttle the warning logging using persistent index 3.
        int lastWarningBar = sc.GetPersistentInt(3);
        if (sc.Index >= (lastWarningBar + refreshInterval))
        {
            char logBuffer[256];
            sprintf(logBuffer, "Warning: VP study with short name '%s' not found on chart.", VPStudyShortName.GetString());
            sc.AddMessageToLog(logBuffer, 1);
            sc.SetPersistentInt(3, sc.Index);
        }
        return;  // Do not call DeleteVAHVALLines when VP study is not found.
    }

    // Retrieve the VP study's subgraph arrays.
    SCFloatArray vpVAHArray;
    SCFloatArray vpVALArray;
    int resultVAH = sc.GetStudyArrayUsingID(vpStudyID, 3, vpVAHArray);
    int resultVAL = sc.GetStudyArrayUsingID(vpStudyID, 4, vpVALArray);

    if (resultVAH == 0 || vpVAHArray.GetArraySize() == 0)
    {
        sc.AddMessageToLog("Error: VAH array is invalid or empty in the VP study.", 1);
        return;
    }
    if (resultVAL == 0 || vpVALArray.GetArraySize() == 0)
    {
        sc.AddMessageToLog("Error: VAL array is invalid or empty in the VP study.", 1);
        return;
    }

    // Verify that sc.Index is within the bounds of the VP arrays.
    if (sc.Index >= vpVAHArray.GetArraySize() || sc.Index >= vpVALArray.GetArraySize())
    {
        char logBuffer[256];
        sprintf(logBuffer, "Error: sc.Index (%d) out of bounds for VP arrays (VAH size: %d, VAL size: %d).", sc.Index, vpVAHArray.GetArraySize(), vpVALArray.GetArraySize());
        sc.AddMessageToLog(logBuffer, 1);
        return;
    }

    // Retrieve current VAH and VAL values.
    float currentVAH = vpVAHArray[sc.Index];
    float currentVAL = vpVALArray[sc.Index];

    char debugBuffer[256];
    sprintf(debugBuffer, "Index = %d, VP Array Size = %d, VAH = %.2f, VAL = %.2f", sc.Index, vpVAHArray.GetArraySize(), currentVAH, currentVAL);
    sc.AddMessageToLog(debugBuffer, 0);

    // Determine if the VP is "Developing" (i.e., current bar is the last bar on the chart).
    bool vpDeveloping = (sc.Index == sc.ArraySize - 1);
    if (!vpDeveloping && DisplayCompletedVP.GetYesNo() == 0)
    {
        sc.AddMessageToLog("Info: VP is completed; VAH/VAL lines will not be displayed.", 0);
        DeleteVAHVALLines(sc);
        return;
    }

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
