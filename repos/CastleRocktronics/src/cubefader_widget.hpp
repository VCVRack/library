#include "cubefader.hpp"
#include "rack.hpp"

using namespace rack;

extern Plugin *plugin;

struct CubefaderWidget : ModuleWidget {
  CubefaderWidget();

 private:
  int moduleWidth;

  void placeGuiElements();
  void placeAudioInputs(int x, int y, int verticalSpacing);
  void placeCvInputs(int x, int y, int horizontalSpacing);
  void placeOutput(int x, int y);
  void placeSlider(int x, int y);
  void placeTrimpots(int x, int y, int horizontalSpacing);
  void placeScrews();
};