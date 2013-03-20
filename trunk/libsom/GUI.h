#ifndef GUI_H
#define GUI_H

#define NODE_SIZE       15

#include <gtkmm/drawingarea.h>
#include <gdkmm/window.h>
#include <gtkmm/textview.h>
#include <gtkmm.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>


#include <gtkmm/button.h>
#include <gtkmm/box.h>

#include <gtkmm/window.h>
#include <gtkmm/main.h>


#include <boost/thread.hpp>  


#include <pangomm/fontdescription.h>
#include <unistd.h>

#include "CSOM.h"


// CDrawingArea Plan Position Indicator: radar screen
class CDrawingArea : public Gtk::DrawingArea
{
public:
  CDrawingArea();
  virtual ~CDrawingArea();
  void train();
  void reset();
  void setTrainingSetsFromFile(std::string);
  void clearTrainingSet();
  void updateSleep(double val);
  void exportState(std::string filename);
  void importState(std::string filename);





protected:
  //Overridden default signal handlers:
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose* event);

  Glib::RefPtr<Gdk::GC> gc_;

private:
  std::vector<std::vector<std::vector<double> > > m_trainingSetVector;
  int trainingIteration;
  int sleepTime;
  int loop;
  double alpha;
  CSOM* som;
};


class GUI : public Gtk::Window
{
public:
  GUI();
  virtual ~GUI();
  std::string showOpenDialog();


  
protected:
  //signal handlers:
  void on_button_clicked(Glib::ustring data);
  void on_adjustment_value_changed();


  // Child widgets
  Gtk::HBox m_box1;
  Gtk::VBox m_box2;

	Gtk::Button m_buttonReset, m_buttonClear, m_buttonTrain, m_buttonOpen, m_buttonExport, m_buttonImport, m_buttonQuit;
  Gtk::Adjustment m_adjustment_digits;
  Gtk::HScale m_Scale_Digits;
  
  CDrawingArea m_area;
};

void printInputSet(std::vector<double> *inputSet);
void printTrainingSet(std::vector<std::vector<double> > *trainingSet);
void printTrainingSetVector(std::vector<std::vector<std::vector<double> > > *trainingSetVector);
std::vector<std::vector<double> > getTrainingSet();
void addValueToInputSet(std::ostringstream * tmps, std::vector<double> *inputSet);
void addInputSetToTrainingSet(std::vector<double> *inputSet, std::vector<std::vector<double> > *trainingSet);

#endif
