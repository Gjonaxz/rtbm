#include "GUI.h"


CDrawingArea::CDrawingArea()
{
	add_events(Gdk::EXPOSURE_MASK);
	reset();
}

void CDrawingArea::reset(){
	som = new CSOM(15, 15, 300);
	trainingIteration = 1;
	loop = 1;
	sleepTime = 0;
	queue_draw();
}

void CDrawingArea::exportState(std::string filename){
//   std::cout << "CDrawingArea_exportState" << std::endl;
	som->exportState(filename.c_str());
}

void CDrawingArea::importState(std::string filename){
	//   std::cout << "CDrawingArea_importState" << std::endl;
	som->importState((const char *)filename.data());
	queue_draw();
}

CDrawingArea::~CDrawingArea()
{
}


void CDrawingArea::on_realize()
{
	// We need to call the base on_realize()
	Gtk::DrawingArea::on_realize();

	// Now we can allocate any additional resources we need
	Glib::RefPtr<Gdk::Window> window = get_window();

	gc_ = Gdk::GC::create(window);

//   window->set_background(black_);
	window->clear();

//   gc_->set_foreground(green_);
}

void CDrawingArea::updateSleep(double val)
{
	sleepTime = int(val);
}

//-------------------------- on_expose_event -----------------------------
//
//  Render of the Gtk::DrawingArea.
//
//------------------------------------------------------------------------
bool CDrawingArea::on_expose_event(GdkEventExpose* event)
{
	// we need a ref to the gdkmm window
	Glib::RefPtr<Gdk::Window> window = get_window();
	
	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
	int x, y;

// 	std::vector<double> weights;//TODO: Improve this loop, consider that this code is called on each refresh! keep in memory the values that don't change
// 	for (y=0; y<som->getHeight(); y++){
// 		for (x=0; x<som->getWidth(); x++){
// 			weights = *(*som->getNode(x,y)).getWeights();
// 			cr->rectangle(x*NODE_SIZE, y*NODE_SIZE, NODE_SIZE, NODE_SIZE);
// 			cr->set_source_rgb(weights[0],weights[1],weights[2]);
// 			cr->fill();
// 	//      cr->set_source_rgb(0, 0, 0);
// 	//      cr->stroke();   // outline it
// 		}
// 	}

//	PRINT BMUs
	int ly = 0; //(som->getHeight()*NODE_SIZE)+15;
	int lx = 0;
	cr->rectangle(lx, ly, som->getWidth()*NODE_SIZE, som->getHeight()*NODE_SIZE);
	cr->set_source_rgb(0,0,0);
	cr->fill();

//	std::cout << "size" << bmus->size() << std::endl;
	std::vector<CNode*> * bmus = som->getBMUlist();
	for (int i=0; i<bmus->size(); i++){
		x = (*bmus)[i]->X();
		y = (*bmus)[i]->Y();
//		std::cout << (*bmus)[i]->X() << ", " << (*bmus)[i]->Y() << std::endl;
		cr->rectangle(lx+(x*NODE_SIZE), ly+(y*NODE_SIZE), NODE_SIZE, NODE_SIZE);
		cr->set_source_rgb((double)i/bmus->size(),1,1);
		cr->fill();
		//std::cout << "X,Y:" << x << "," << y << std::endl;

	}	
	


//PRINT BORDERS
//	int ly = (som->getHeight()*NODE_SIZE)+15;
//  for (y=0; y<som->getHeight(); y++){
//    for (x=0; x<som->getWidth(); x++){
//      weights = *(*som->getNode(x,y)).getWeights();
//      cr->rectangle(x*NODE_SIZE, ly+(y*NODE_SIZE), NODE_SIZE, NODE_SIZE);
//      cr->set_source_rgb(0,0,0);
//      cr->fill();
//    }
//  }
	
	std::ostringstream ost;
	ost << "training set: " << (trainingIteration+1);
	Glib::RefPtr<Pango::Layout> alayout = create_pango_layout(ost.str());
	window->draw_layout(gc_, event->area.width/2-10, event->area.height - 45, alayout);

	ost.str("");
	ost << "iteration: " << loop;
	alayout = create_pango_layout(ost.str());
	window->draw_layout(gc_, event->area.width/2-10, event->area.height - 25, alayout);
	
	return true;
}

void CDrawingArea::train()
{  
	std::cout << "number of training sets:" << m_trainingSetVector.size() << std::endl; 
	for (trainingIteration=0 ; trainingIteration<m_trainingSetVector.size() ; trainingIteration++){
//     printTrainingSet(&(m_trainingSetVector[trainingIteration]));

		som->setTrainingValues(/*Training Set*/m_trainingSetVector[trainingIteration], /*Number of Iterations*/500, /*Initial Learning Rate*/0.07);
		while (som->trainNextIteration()){
			usleep(sleepTime*1000);
			loop++;
			queue_draw();
		};
		loop=1;
	}
}



GUI::GUI()
	: m_box1(/*homogeneous*/false, /*spacing*/5), m_box2(false, 5), 
		m_adjustment_digits(0, 0, 1000, 10, 50, 0),
		m_Scale_Digits(m_adjustment_digits),
		m_buttonReset("Reset SOM"), m_buttonClear("Clear Training sets"), m_buttonTrain("Train"), m_buttonExport("Export"), m_buttonImport("Import"), m_buttonOpen("Append Training set from file"), m_buttonQuit("Quit"),
		m_area()
{

	// box2
	m_buttonReset.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "reset"));
	m_buttonClear.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "clear"));
	m_buttonTrain.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "train"));
	m_buttonExport.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "export"));
	m_buttonImport.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "import"));	
	m_buttonOpen.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "open"));
	m_buttonQuit.signal_clicked().connect(sigc::bind<-1, Glib::ustring>(sigc::mem_fun(*this, &GUI::on_button_clicked), "quit"));
	
	m_box2.pack_start(m_buttonReset, /*Gtk::PackOptions*/Gtk::PACK_SHRINK, /*padding*/5);
	m_box2.pack_start(m_buttonClear, Gtk::PACK_SHRINK, 5);
	m_box2.pack_start(m_buttonTrain, Gtk::PACK_SHRINK, 5);
	m_box2.pack_start(m_buttonExport, Gtk::PACK_SHRINK, 5);
	m_box2.pack_start(m_buttonImport, Gtk::PACK_SHRINK, 5);
	m_box2.pack_start(m_buttonOpen, Gtk::PACK_SHRINK, 5);
	m_box2.pack_start(m_buttonQuit, Gtk::PACK_SHRINK, 5);
	
	m_Scale_Digits.set_digits(0);
	m_adjustment_digits.signal_value_changed().connect(sigc::mem_fun(*this, &GUI::on_adjustment_value_changed));
	
	m_box2.pack_start(*Gtk::manage(new Gtk::Label("Sleep between iterations:", 0)),Gtk::PACK_SHRINK);
	m_box2.pack_start(m_Scale_Digits,Gtk::PACK_SHRINK);

	// box1
	m_area.set_size_request(280, 100);
	m_box1.pack_start(m_area, Gtk::PACK_EXPAND_WIDGET, 5);
	m_box1.pack_start(m_box2, Gtk::PACK_SHRINK, 5);
	
	set_border_width(10);
	add(m_box1);
	show_all();

}

void GUI::on_adjustment_value_changed()
{
	const double val = m_adjustment_digits.get_value();
	m_area.updateSleep(val);
}

GUI::~GUI()
{
}

void GUI::on_button_clicked(Glib::ustring data)
{
	if (data.compare("quit") == 0){
		hide();
	}
	if (data.compare("clear") == 0){
		m_area.clearTrainingSet();
	}
	if (data.compare("train") == 0){
		m_area.setTrainingSetsFromFile("sample_input_rtbm.txt"); //TODO: remove when not testing
// 		m_area.setTrainingSetsFromFile("sample_input_colors.txt"); //TODO: remove when not testing
		boost::thread workerThread(&CDrawingArea::train, &m_area);
	}
//  if (data.compare("open") == 0){
//    std::cout << "open" << std::endl;
//    std::string filename = showOpenDialog();
//    if (filename.size()>0)
//      m_area.setTrainingSetsFromFile(filename);
//  }
	if (data.compare("reset") == 0){
		m_area.reset();
	}
	if (data.compare("export") == 0){
		m_area.exportState("SOM_export.dat");
	}
	if (data.compare("import") == 0){
		m_area.importState("SOM_import.dat");
	}
	
}

std::string GUI::showOpenDialog(){
	Gtk::FileChooserDialog dialog("Please choose a file",
					Gtk::FILE_CHOOSER_ACTION_OPEN);
	dialog.set_transient_for(*this);

	//Add response buttons the the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);


	//Show the dialog and wait for a user response:
	int result = dialog.run();

	//Handle the response:
	switch(result)
	{
		case(Gtk::RESPONSE_OK):
		{
			std::cout << "Open clicked." << std::endl;

			//Notice that this is a std::string, not a Glib::ustring.
			std::string filename = dialog.get_filename();
			std::cout << "File selected: " <<  filename << std::endl;
			return filename;
			break;
		}
		case(Gtk::RESPONSE_CANCEL):
		{
			std::cout << "Cancel clicked." << std::endl;
			break;
		}
		default:
		{
			std::cout << "Unexpected button clicked." << std::endl;
			break;
		}
	}
	return "";
}
void CDrawingArea::clearTrainingSet(){
	m_trainingSetVector.clear();
}

void CDrawingArea::setTrainingSetsFromFile(std::string filename){
	std::cout << "setting training set vector from file" << std::endl;
	std::ifstream inFile(filename.data());
	bool ignore = false;
	char ch;
	double tmpd;
	std::ostringstream tmps;
	std::ostringstream lg;
	std::vector<std::vector<double> > trainingSet;
	std::vector<double> inputSet;
	int i,j,k;

	if (! inFile)
	{
		std::cerr << "unable to open input file: "
				<< filename << " --bailing out! \n";
		return;
	}

	while (inFile.get(ch)){
		if (ignore){
//       std::cout << ch;
			if ((int(ch) == 13) || (int(ch) == 10)) {//end of training set
				ignore = false;
			}
		} else {
			if (int(ch) == 32){ //space
				addValueToInputSet(&tmps, &inputSet);

			} else if ((int(ch) == 13) || (int(ch) == 10)) {//end of line
				if (inputSet.size()>0){
					addValueToInputSet(&tmps, &inputSet);
					addInputSetToTrainingSet(&inputSet, &trainingSet);
				}
			} else if (int(ch) == 61) {//end of training set (=)
				if (trainingSet.size()>0){
					m_trainingSetVector.push_back(trainingSet);
					trainingSet.clear();
				} 
			} else if (int(ch) == 35){ //space
				ignore = true;
//         std::cout << "ignoring: ";

			} else {
				tmps << ch;
			}
		}
//     std::cout << int(ch) << std::end; 
	}
	if (trainingSet.size()>0){
		m_trainingSetVector.push_back(trainingSet);
	} 

//	printTrainingSetVector(&m_trainingSetVector);
	inFile.close();
}

void addInputSetToTrainingSet(std::vector<double> *inputSet, std::vector<std::vector<double> > *trainingSet){
	trainingSet->push_back(*inputSet);
	inputSet->clear();
}

void addValueToInputSet(std::ostringstream *tmps, std::vector<double> *inputSet){
	if (tmps->str().size() == 0) return;
	float tmpd = atof ( tmps->str().data() );
	tmps->str("");
	tmps->flush();
	inputSet->push_back(tmpd);
}

void printInputSet(std::vector<double> *inputSet){
	int i;
	std::cout << "inputSet[" << inputSet->size() << "]:[";
	for (i=0 ; i<inputSet->size() ; i++){
		std::cout << (*inputSet)[i] << ",";
	}
	std::cout << "]" << std::endl;
}

void printTrainingSet(std::vector<std::vector<double> > *trainingSet){
	int i;
	std::cout << "trainingSet[" << trainingSet->size() << "]:[\n";
	for (i=0 ; i<trainingSet->size() ; i++){
		printInputSet(&((*trainingSet)[i]));
	}
	std::cout << "]" << std::endl;
}

void printTrainingSetVector(std::vector<std::vector<std::vector<double> > > *trainingSetVector){
	int i;
	std::cout << "trainingSetVector[" << trainingSetVector->size() << "]:[\n";
	for (i=0 ; i<trainingSetVector->size() ; i++){
		printTrainingSet(&((*trainingSetVector)[i]));
	}
	std::cout << "]" << std::endl;
}


std::vector<std::vector<double> > getTrainingSet(){
	std::vector<std::vector<double> > m_TrainingSet;

	std::vector<double> color;

	for (int i=0; i<7; i++){
		color.push_back(rand()/(RAND_MAX+1.0));
		color.push_back(rand()/(RAND_MAX+1.0));
		color.push_back(rand()/(RAND_MAX+1.0));
		m_TrainingSet.push_back(color);
		color.clear();
	}
	return m_TrainingSet;
}

