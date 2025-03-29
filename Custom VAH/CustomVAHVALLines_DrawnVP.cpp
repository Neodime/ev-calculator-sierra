#include "sierrachart.h"

SCDLLName("Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name")

//--------------------------------------------------------------
// Helper Function: DeleteVAHVALLines
// This function deletes the drawn VAH and VAL lines if they exist,
// resetting their persistent storage values to 0.
//--------------------------------------------------------------
void DeleteVAHVALLines(SCStudyInterfaceRef sc)
{
    int* pLineID_VAH = sc.GetPersistentInt(1);
    int* pLineID_VAL = sc.GetPersistentInt(2);
    if (pLineID_VAH && *pLineID_VAH != 0)
    {
        sc.DeleteLine(*pLineID_VAH);
        *pLineID_VAH = 0;
    }
    if (pLineID_VAL && *pLineID_VAL != 0)
    {
        sc.DeleteLine(*pLineID_VAL);
        *pLineID_VAL = 0;
    }
}

//--------------------------------------------------------------
// Study: CustomVAHVALLines_RevisedWithVPShortName
//
// This custom study retrieves the Value Area High (VAH) and 
// Value Area Low (VAL) values from a drawn Volume Profile (VP) study.
// The VP study's short name is user-specified via an input (default "Volume Profile").
// VAH is assumed to be stored in subgraph index 3 and VAL in subgraph index 4.
// White, dashed horizontal lines are drawn at these levels and updated
// based on a user-defined refresh interval. If the VP is not "developing"
// (i.e. the current bar is not the last bar on the chart) and the user opts
// not to display lines for a completed VP, then the lines are removed.
//--------------------------------------------------------------
SCSFExport scsf_CustomVAHVALLines_RevisedWithVPShortName(SCStudyInterfaceRef sc)
{
    //----- Input Definitions -----
    SCInputRef RefreshInterval    = sc.Input[0];
    SCInputRef DisplayCompletedVP = sc.Input[1]; // Option to display lines even if VP is completed
    SCInputRef VPStudyShortName   = sc.Input[2]; // User-specified short name of the VP study

    if (sc.SetDefaults)
    {
        //----- Set Study Defaults -----
        sc.GraphName = "Custom VAH/VAL Lines from Drawn VP Revised with VP Study Short Name";
        sc.StudyDescription = "Retrieves VAH (SG3) and VAL (SG4) from a drawn VP study (specified by short name) and draws white dashed horizontal lines.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;

        // Input: Refresh Interval (number of bars)
        RefreshInterval.Name = "Refresh Interval (Bars)";
        RefreshInterval.SetInt(1);
        RefreshInterval.SetIntLimits(1, 1000);

        // Input: Option to display lines even when VP is completed
        DisplayCompletedVP.Name = "Display Lines When VP Completed";
        DisplayCompletedVP.SetYesNo(0); // Default: No (lines will not display if VP is completed)

        // Input: VP Study Short Name, defaulting to "Volume Profile"
        VPStudyShortName.Name = "VP Study Short Name";
        VPStudyShortName.SetString("Volume Profile");

        // Initialize persistent integers for storing drawn line IDs.
        sc.SetPersistentInt(1, 0);  // For VAH line
        sc.SetPersistentInt(2, 0);  // For VAL line

        return;
    }

    //----- Defensive Check: Ensure sc.Index is within valid range -----
    if (sc.Index < 0 || sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of valid range.", 1);
        return;
    }

    //----- Refresh Control: Update only every X bars as defined by the refresh interval -----
    int refreshInterval = RefreshInterval.GetInt();
    if ((sc.Index % refreshInterval) != 0)
        return;

    //----- Retrieve the Volume Profile Study using the user-specified short name -----
    // Instead of a hard-coded name, we use the value from VPStudyShortName input.
    int vpStudyID = sc.GetStudyByName(sc.ChartNumber, VPStudyShortName.GetString(), 0);
    if (vpStudyID == 0)
    {
        // Build the log message using sprintf instead of std::string.
        char logBuffer[256];
        sprintf(logBuffer, "Warning: VP study with short name '%s' not found on chart.", VPStudyShortName.GetString());
        sc.AddMessageToLog(logBuffer, 1);
        DeleteVAHVALLines(sc);
        return;
    }

    //----- Retrieve VAH and VAL Arrays from the VP study -----
    // VAH is stored in subgraph index 3 and VAL in subgraph index 4.
    float* vpVAHArray = sc.GetStudyArrayUsingID(vpStudyID, 3);
    float* vpVALArray = sc.GetStudyArrayUsingID(vpStudyID, 4);
    if (vpVAHArray == nullptr || vpVALArray == nullptr)
    {
        sc.AddMessageToLog("Error: VAH/VAL arrays not found in the VP study.", 1);
        return;
    }

    //----- Defensive Check: Ensure sc.Index is within the bounds of the VP arrays -----
    if (sc.Index >= sc.ArraySize)
    {
        sc.AddMessageToLog("Error: sc.Index is out of bounds for the VP arrays.", 1);
        return;
    }

    //----- Retrieve Current VAH and VAL Values -----
    float currentVAH = vpVAHArray[sc.Index];
    float currentVAL = vpVALArray[sc.Index];

    //----- Determine if the Volume Profile is "Developing" -----
    // The current logic considers the VP to be "developing" if the current bar (sc.Index)
    // is the last bar on the chart (sc.ArraySize - 1). This is based on the assumption 
    // that real-time updates occur only on the latest bar. If you wish to display the 
    // lines even after the VP is completed, set the "Display Lines When VP Completed" input to Yes.
    bool vpDeveloping = (sc.Index == sc.ArraySize - 1);
    if (!vpDeveloping && DisplayCompletedVP.GetYesNo() == 0)
    {
        sc.AddMessageToLog("Info: VP is completed; VAH/VAL lines will not be displayed.", 0);
        DeleteVAHVALLines(sc);
        return;
    }

    //----- Debug Logging: Log current VAH and VAL values -----
    char debugBuffer[256];
    sprintf(debugBuffer, "Updating VAH/VAL lines: VAH = %.2f, VAL = %.2f", currentVAH, currentVAL);
    sc.AddMessageToLog(debugBuffer, 0);

    //----- Retrieve Persistent Storage for Line IDs -----
    int* pLineID_VAH = sc.GetPersistentInt(1);
    int* pLineID_VAL = sc.GetPersistentInt(2);

    //----- Prepare Drawing Tool for Horizontal Lines -----
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

    //----- Draw or Update VAH Horizontal Line -----
    Tool.BeginValue = currentVAH;
    Tool.EndValue = currentVAH;
    // If the line already exists, use its LineNumber; otherwise, let ACSIL assign one.
    Tool.LineNumber = (pLineID_VAH && *pLineID_VAH != 0) ? *pLineID_VAH : 0;
    sc.UseTool(Tool);
    *pLineID_VAH = Tool.LineNumber;

    //----- Draw or Update VAL Horizontal Line -----
    Tool.BeginValue = currentVAL;
    Tool.EndValue = currentVAL;
    Tool.LineNumber = (pLineID_VAL && *pLineID_VAL != 0) ? *pLineID_VAL : 0;
    sc.UseTool(Tool);
    *pLineID_VAL = Tool.LineNumber;
}
