#include "visuals.h"
#include "Overlay.h"
#include <iostream>

using namespace std;
PRENDER* PRENDER::Instance()
{
	static PRENDER instance;
	return &instance;
}


void PRENDER::Render()
{

	//cout << "render!" << endl;
}
