#include "sierrachart.h"

SCDLLName("EV Calculator - Menu Lite")

// ID du menu contextuel personnalisé
const int MENU_ID_HINT = 100001;

SCSFExport scsf_EVCalculator_Interactive_Lite(SCStudyInterfaceRef sc)
{
  SCInputRef inputTarget = sc.Input[0];
  SCInputRef inputRisque = sc.Input[1];
  SCInputRef inputProba = sc.Input[2];

  if (sc.SetDefaults)
  {
    sc.GraphName = "EV Calculator (Menu Lite)";
    sc.StudyDescription = "EV avec menu contextuel. Modifier les inputs dans le panneau d'étude.";
    sc.AutoLoop = 0;

    inputTarget.Name = "Target (en R)";
    inputTarget.SetFloat(1.00);

    inputRisque.Name = "Risque (en R)";
    inputRisque.SetFloat(-1.00);

    inputProba.Name = "Probabilité de gain (en %)";
    inputProba.SetFloat(50.0);

    sc.MenuEventID = MENU_ID_HINT;

    return;
  }

  // 🧠 Menu contextuel activé
  if (sc.MenuEventID == MENU_ID_HINT)
  {
    sc.AddMessageToLog("➡️ Pour modifier les paramètres EV (Target, Risque, Proba), ouvrez le panneau d'étude et ajustez les inputs.", 0);
    sc.MenuEventID = 0;
  }

  // 📊 Calcul de l'EV
  float target = inputTarget.GetFloat();
  float risque = inputRisque.GetFloat();
  float proba = inputProba.GetFloat();
  float p = proba / 100.0f;
  float ev = p * target + (1.0f - p) * risque;

  // 📋 Affichage texte au-dessus de la dernière bougie
  SCString text;
  text.Format("EV: %.2f R | 🎯 %.2f | 📉 %.2f | 📈 %.1f%%", ev, target, risque, proba);

  int index = sc.ArraySize - 1;
  float price = sc.High[index] + sc.TickSize * 10;

  s_UseTool tool;
  tool.Clear();
  tool.ChartNumber = sc.ChartNumber;
  tool.DrawingType = DRAWING_TEXT;
  tool.Region = 0;
  tool.AddAsUserDrawnDrawing = false;
  tool.LineNumber = 1;
  tool.Text = text;
  tool.Color = RGB(0, 255, 0); // couleur fixe
  tool.FontSize = 12;
  tool.FontBold = true;
  tool.BeginIndex = index;
  tool.BeginValue = price;

  sc.UseTool(tool);
}
