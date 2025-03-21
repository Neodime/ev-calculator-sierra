#include "sierrachart.h"

SCDLLName("EV Calculator - Overlay Mode")

SCSFExport scsf_EVCalculator_Overlay(SCStudyInterfaceRef sc)
{
  SCInputRef inputTarget = sc.Input[0];
  SCInputRef inputRisque = sc.Input[1];
  SCInputRef inputProba = sc.Input[2];

  if (sc.SetDefaults)
  {
    sc.GraphName = "EV Calculator (Overlay)";
    sc.StudyDescription = "Affiche l'espérance de valeur (EV) en haut à gauche du graphique.";
    sc.AutoLoop = 0;

    inputTarget.Name = "Target (en R)";
    inputTarget.SetFloat(1.00);

    inputRisque.Name = "Risque (en R)";
    inputRisque.SetFloat(-1.00);

    inputProba.Name = "Probabilité de gain (en %)";
    inputProba.SetFloat(50.0);

    return;
  }

  float target = inputTarget.GetFloat();
  float risque = inputRisque.GetFloat();
  float proba = inputProba.GetFloat();

  // Clamp probabilité entre 0 et 100
  if (proba < 0.0f) proba = 0.0f;
  if (proba > 100.0f) proba = 100.0f;

  // Calcul de l'espérance de valeur
  float p = proba / 100.0f;
  float ev = p * target + (1.0f - p) * risque;

  // Formatage du texte à afficher
  SCString text;
  text.Format("📊 EV: %.2f R\n🎯 Target: %.2f R\n📉 Risque: %.2f R\n📈 P(Gain): %.1f%%", ev, target, risque, proba);

  // Affichage graphique : texte flottant en haut à gauche
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
  tool.UseRelativeVerticalPosition = true;
  tool.UseRelativeHorizontalPosition = true;
  tool.RelativeVerticalPosition = 0.02f;
  tool.RelativeHorizontalPosition = 0.01f;

  sc.UseTool(tool);
}
