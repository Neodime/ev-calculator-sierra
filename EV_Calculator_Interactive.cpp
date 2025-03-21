#include "sierrachart.h"

SCDLLName("EV Calculator - Interactive")

// ID de menu contextuel
const int MENU_ID_INPUTS = 100001;

SCSFExport scsf_EVCalculator_Interactive(SCStudyInterfaceRef sc)
{
  SCInputRef inputTarget = sc.Input[0];
  SCInputRef inputRisque = sc.Input[1];
  SCInputRef inputProba = sc.Input[2];

  if (sc.SetDefaults)
  {
    sc.GraphName = "EV Calculator (Interactive)";
    sc.StudyDescription = "EV dynamique avec input utilisateur via menu contextuel et InputBox()";
    sc.AutoLoop = 0;

    inputTarget.Name = "Target (en R)";
    inputTarget.SetFloat(1.00);

    inputRisque.Name = "Risque (en R)";
    inputRisque.SetFloat(-1.00);

    inputProba.Name = "Probabilité de gain (en %)";
    inputProba.SetFloat(50.0);

    // Active le menu contextuel personnalisé
    sc.MenuEventID = MENU_ID_INPUTS;

    return;
  }

  // 🎯 Menu personnalisé cliqué ?
  if (sc.MenuEventID == MENU_ID_INPUTS)
  {
    SCString val;
    float f;

    // 🟢 1. Target
    if (sc.InputBox("Entrez la Target (en R)", "Target", val))
    {
      f = (float)atof(val.GetChars());
      inputTarget.SetFloat(f);
    }

    // 🔴 2. Risque
    if (sc.InputBox("Entrez le Risque (en R)\nEx: -1 ou +0.75", "Risque", val))
    {
      f = (float)atof(val.GetChars());
      inputRisque.SetFloat(f);
    }

    // 🔵 3. Proba
    if (sc.InputBox("Entrez la Probabilité de gain (0-100)", "Probabilité", val))
    {
      f = (float)atof(val.GetChars());
      if (f < 0.0f) f = 0.0f;
      if (f > 100.0f) f = 100.0f;
      inputProba.SetFloat(f);
    }

    // Reset pour éviter relance
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
  tool.Color = ev >= 0 ? RGB(0, 255, 0) : RGB(255, 0, 0); // vert ou rouge
  tool.FontSize = 12;
  tool.FontBold = true;
  tool.BeginIndex = index;
  tool.BeginValue = price;

  sc.UseTool(tool);
}
