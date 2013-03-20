#include "GUI.h"


int main(int argc, char** argv)
{
  if (argc == 2){
    std::string opt = "console";
    if (opt.compare(argv[1]) == 0){
      CSOM* som = new CSOM(40, 40, 3);
      int loop = 1;
      
      std::vector<std::vector<double> > ts = getTrainingSet();


      som->setTrainingValues(/*Training Set*/ts, /*Number of Iterations*/500, /*Initial Learning Rate*/0.07);
      while (som->trainNextIteration()){
        loop++;
//         std::cout << loop << endl;
      }
      return 0;
    }
  }
  Gtk::Main main_instance (argc, argv);

  GUI gui;
  Gtk::Main::run(gui);

  return 0;
}