#include "sierrachart.h"

SCDLLName("Custom VAH/VAL Lines with VP Study Short Name - Last Element for Developing VP (Optimized, No Extra Lines)")

// Helper Function: DeleteVAHVALLines
// Deletes the drawn VAH and VAL lines if they exist by configuring an s_UseTool structure
// with SZM_DELETE and calling sc.UseTool with the appropriate line number.
// Logs deletion events if DebugMode is enabled.
void DeleteVAHVALLines(SCStudyInterfaceRef sc)
{
    int pLineID_VAH = sc.GetPersistentInt(1);
    int pLineID_VAL = sc.GetPersistentInt(2);

    if (pLineID_VAH == 0 && pLineID_VAL == 0)
        return; // Nothing to delete.
    
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
        char buf[128];
        sprintf(buf, "Deleting stale VAH line (LineNumber %d)", pLineID_VAH);
        sc.AddMessageToLog(buf, 0);
        Tool.LineNumber = pLineID_VAH;
        sc.UseTool(Tool);
        sc.SetPersistentInt(1, 0);
    }
    if (pLineID_VAL != 0)
    {
        char buf[128];
        sprintf(buf, "Deleting stale VAL line (LineNumber %d)", pLineID_VAL);
        sc.AddMessageToLog(buf, 0);
        Tool.LineNumber = pLineID_VAL;
        sc.UseTool(Tool);
        sc.SetPersistentInt(2, 0);
    }
}

// Study: CustomVAHVALLines_RevisedWithVPShortName
// Retrieves VAH (subgraph index 3) and VAL (subgraph index 4) from a VP study specified by its short name,
// then draws exactly two horizontal dashed lines (one for VAH and one for VAL).
// If the VP study is developing, the study uses the last element of the VP arrays for drawing.
// If the VP study's array size changes (e.g. via manual adjustment), stale drawings are deleted before redrawing.
// Debug logging is output if DebugMode is enabled.
SCSFExport scsf_CustomVAHVALLines_RevisedWithVPShortName(SCStudyInterfaceRef sc)
{
    // Input definitions:
    // [0] Refresh Interval (Bars), default 10 bars (~5 minutes on a 30-sec chart)
    // [1] Display Lines When VP Completed (Yes/No)
    // [2] VP Study Short Name (default "Volume Profile")
    // [3] Debug Mode (Yes/No)
    SCInputRef RefreshInterval    = sc.Input[0];
    SCInputRef DisplayCompletedVP = sc.Input[1];
    SCInputRef VPStudyShortName   = sc.Input[2];
    SCInputRef DebugMode          = sc.Input[3];

    if (sc.SetDefaults)
    {
        sc.GraphName = "Custom VAH/VAL Lines with VP Study Short Name - Last Element for Developing VP (Optimized)";
        sc.StudyDescription = "Retrieves VAH (SG3) and VAL (SG4) from a VP study (by short name) and draws exactly two horizontal dashed lines. If the VP is developing, the last element of the VP arrays is used. Stale drawings are deleted on manual adjustments.";
        sc.AutoLoop = 1;
        sc.GraphRegion = 0;
        sc.CalculationPrecedence = LOW_PREC_LEVEL;  // Ensures VP arrays are updated.
        
        RefreshInterval.Name = "Refresh Interval (Bars)";
        RefreshInterval.SetInt(10);
        RefreshInterval.SetIntLimits(1, 1000);
        
        DisplayCompletedVP.Name = "Display Lines When VP Completed";
        DisplayCompletedVP.SetYesNo(0);
        
        VPStudyShortName.Name = "VP Study Short Name";
        VPStudyShortName.SetString("Volume Profile");
        
        DebugMode.Name = "Debug Mode";
        DebugMode.SetYesNo(0);
        
        // Persistent indices:
        // 1: VAH line ID, 2: VAL line ID, 3: Last bar index when VP missing warning was logged,
        // 4: Cached last drawn VAH value, 5: Cached last drawn VAL value, 6: Cached VP array size.
        sc.SetPersistentInt(1, 0);
        sc.SetPersistentInt(2, 0);
        sc.SetPersistentInt(3, 0);
        sc.SetPersistentFloat(4, 0.0);
        sc.SetPersistentFloat(5, 0.0);
        sc.SetPersistentInt(6, 0);
        
        return;
    }
    
    if (sc.Index < 0 || sc.Index >= sc.ArraySize)
        return;
    
    int refreshInterval = RefreshInterval.GetInt();
    if ((sc.Index % refreshInterval) != 0)
        return;
    
    // Retrieve VP study using short name lookup; use third parameter = 1.
    int vpStudyID = sc.GetStudyIDByName(sc.ChartNumber, VPStudyShortName.GetString(), 1);
    if (vpStudyID == 0)
    {
        int lastWarnBar = sc.GetPersistentInt(3);
        if (sc.Index >= (lastWarnBar + refreshInterval))
        {
            char buf[256];
            sprintf(buf, "Warning: VP study with short name '%s' not found on chart.", VPStudyShortName.GetString());
            sc.AddMessageToLog(buf, 1);
            sc.SetPersistentInt(3, sc.Index);
        }
        return;
    }
    
    // Retrieve VP study arrays.
    SCFloatArray vpVAHArray;
    SCFloatArray vpVALArray;
    int resVAH = sc.GetStudyArrayUsingID(vpStudyID, 3, vpVAHArray);
    int resVAL = sc.GetStudyArrayUsingID(vpStudyID, 4, vpVALArray);
    if (resVAH == 0 || vpVAHArray.GetArraySize() == 0)
    {
        if (DebugMode.GetYesNo())
            sc.AddMessageToLog("Error: VAH array is invalid or empty in the VP study.", 1);
        return;
    }
    if (resVAL == 0 || vpVALArray.GetArraySize() == 0)
    {
        if (DebugMode.GetYesNo())
            sc.AddMessageToLog("Error: VAL array is invalid or empty in the VP study.", 1);
        return;
    }
    
    int currentArraySize = vpVAHArray.GetArraySize();
    int cachedArraySize = sc.GetPersistentInt(6);
    if (currentArraySize != cachedArraySize)
    {
        if (DebugMode.GetYesNo())
        {
            char buf[256];
            sprintf(buf, "VP array size changed from %d to %d. Deleting stale drawings.", cachedArraySize, currentArraySize);
            sc.AddMessageToLog(buf, 0);
        }
        // Force deletion of any stale drawings.
        DeleteVAHVALLines(sc);
        sc.SetPersistentInt(6, currentArraySize);
    }
    
    // Determine the index to use for retrieving VP values.
    // If the VP study is developing (sc.Index equals the last bar in the chart),
    // then use the last element of the VP arrays. Otherwise, use sc.Index if within bounds,
    // otherwise default to the last element.
    bool vpDeveloping = (sc.Index == sc.ArraySize - 1);
    int indexForValue = sc.Index;
    if (vpDeveloping)
    {
        indexForValue = currentArraySize - 1;
        if (DebugMode.GetYesNo())
        {
            char buf[256];
            sprintf(buf, "VP is developing; using last index %d of VP arrays.", indexForValue);
            sc.AddMessageToLog(buf, 0);
        }
    }
    else if (sc.Index >= currentArraySize)
    {
        indexForValue = currentArraySize - 1;
        if (DebugMode.GetYesNo())
        {
            char buf[256];
            sprintf(buf, "sc.Index (%d) out of bounds; using last index %d.", sc.Index, indexForValue);
            sc.AddMessageToLog(buf, 0);
        }
    }
    
    float currentVAH = vpVAHArray[indexForValue];
    float currentVAL = vpVALArray[indexForValue];
    
    if (DebugMode.GetYesNo())
    {
        char buf[256];
        sprintf(buf, "Using VP array index %d: VAH = %.4f, VAL = %.4f (Array Size = %d)", indexForValue, currentVAH, currentVAL, currentArraySize);
        sc.AddMessageToLog(buf, 0);
    }
    
    // If VP is not developing and DisplayCompletedVP is false, delete drawings and exit.
    if (!vpDeveloping && DisplayCompletedVP.GetYesNo() == 0)
    {
        if (DebugMode.GetYesNo())
            sc.AddMessageToLog("Info: VP is completed; not displaying VAH/VAL lines.", 0);
        DeleteVAHVALLines(sc);
        return;
    }
    
    // Retrieve cached drawn values.
    float lastDrawnVAH = sc.GetPersistentFloat(4);
    float lastDrawnVAL = sc.GetPersistentFloat(5);
    const float threshold = 0.0001f;
    bool updateNeeded = (fabs(currentVAH - lastDrawnVAH) > threshold || fabs(currentVAL - lastDrawnVAL) > threshold);
    if (!updateNeeded)
    {
        if (DebugMode.GetYesNo())
        {
            char buf[128];
            sprintf(buf, "Skipping update; VAH and VAL unchanged (VAH=%.4f, VAL=%.4f).", currentVAH, currentVAL);
            sc.AddMessageToLog(buf, 0);
        }
        return;
    }
    
    // Update drawing tools using UTAM_ADD_OR_ADJUST to update existing lines.
    s_UseTool Tool;
    memset(&Tool, 0, sizeof(Tool));
    Tool.ChartNumber = sc.ChartNumber;
    Tool.DrawingType = DRAWING_HORIZONTALLINE;
    Tool.AddMethod = UTAM_ADD_OR_ADJUST;
    Tool.Region = sc.GraphRegion;
    Tool.BeginIndex = 0;
    Tool.EndIndex = sc.ArraySize - 1;
    Tool.Color = RGB(255, 255, 255);
    Tool.LineStyle = LINESTYLE_DASH;
    Tool.LineWidth = 2;
    
    int pLineID_VAH = sc.GetPersistentInt(1);
    Tool.BeginValue = currentVAH;
    Tool.EndValue = currentVAH;
    Tool.LineNumber = (pLineID_VAH != 0) ? pLineID_VAH : 0;
    sc.UseTool(Tool);
    sc.SetPersistentInt(1, Tool.LineNumber);
    
    int pLineID_VAL = sc.GetPersistentInt(2);
    Tool.BeginValue = currentVAL;
    Tool.EndValue = currentVAL;
    Tool.LineNumber = (pLineID_VAL != 0) ? pLineID_VAL : 0;
    sc.UseTool(Tool);
    sc.SetPersistentInt(2, Tool.LineNumber);
    
    sc.SetPersistentFloat(4, currentVAH);
    sc.SetPersistentFloat(5, currentVAL);
    
    if (DebugMode.GetYesNo())
    {
        char buf[256];
        sprintf(buf, "Updated drawing: VAH = %.4f, VAL = %.4f (Line IDs: VAH=%d, VAL=%d)", currentVAH, currentVAL, sc.GetPersistentInt(1), sc.GetPersistentInt(2));
        sc.AddMessageToLog(buf, 0);
    }
}
