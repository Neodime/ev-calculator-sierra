#include "sierrachart.h"

SCDLLName("EV Calculator - Menu Inputs")

// ID unique pour notre menu custom
const int MENU_ID_EV_INPUTS = 100001;

SCSFExport scsf_EVCalculator_Menu(SCStudyInterfaceRef sc)
{
  SCInputRef inputTarget = sc.Input[0];
  SCInputRef inputRisque = sc.Input[1];
  SCInputRef inputProba = sc.Input[2];

  if (sc.SetDefaults)
  {
    sc.GraphName = "EV Calculator (Menu Inputs)";
    sc.StudyDescription = "Affiche l'EV avec modification des inputs via menu contextuel.";
    sc.AutoLoop = 0;

    inputTarget.Name = "Target (en R)";
    inputTarget.SetFloat(1.00);

    inputRisque.Name = "Risque (en R)";
    inputRisque.SetFloat(-1.00);

    inputProba.Name = "Probabilité de gain (en %)";
    inputProba.SetFloat(50.0);

    // Enregistre l'entrée de menu personnalisée
    sc.MenuEventID = MENU_ID_EV_INPUTS;

    return;
  }

  // 🎯 Gérer le menu contextuel personnalisé
  if (sc.MenuEventID == MENU_ID_EV_INPUTS)
  {
    SCString prompt;
    SCString result;

    prompt = "Entrez les valeurs séparées par des virgules :\nTarget,Risque,Proba (%)\nEx: 1.5,-0.75,60";
    if (sc.InputBox(prompt, "Modifier les paramètres EV", result))
    {
      SCStringArray parts;
      result.Tokenize(",", parts);

      if (parts.GetArraySize() == 3)
      {
        float t = atof(parts[0].GetChars());
        float r = atof(parts[1].GetChars());
        float p = atof(parts[2].GetChars());

        inputTarget.SetFloat(t);
        inputRisque.SetFloat(r);
        inputProba.SetFloat(p);
      }
    }
    sc.MenuEventID = 0; // Reset
  }

  // 📊 Calcul EV
  float target = inputTarget.GetFloat();
  float risque = inputRisque.GetFloat();
  float proba = inputProba.GetFloat();

  if (proba < 0.0f) proba = 0.0f;
  if (proba > 100.0f) proba = 100.0f;

  float p = proba / 100.0f;
  float ev = p * target + (1.0f - p) * risque;

  SCString text;
  text.Format("EV: %.2f R | Target: %.2f | Risque: %.2f | P(Gain): %.1f%%", ev, target, risque, proba);

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
  tool.Color = RGB(0, 255, 0);
  tool.FontSize = 12;
  tool.FontBold = true;
  tool.BeginIndex = index;
  tool.BeginValue = price;

  sc.UseTool(tool);
}
