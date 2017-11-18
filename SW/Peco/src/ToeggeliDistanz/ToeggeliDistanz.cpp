#include "ToeggeliDistanz.h"

ToeggeliDistanz::ToeggeliDistanz(){
	
}

void ToeggeliDistanz::init(Print &print, int p_iEchoInputPIN, int p_iTrigOutputPIN){

	printer = &print; //Object for printing on Serial
	printer->println("ToeggeliDistanz::init Anfang");
	iEchoInputPIN = p_iEchoInputPIN;
	iTrigOutputPIN = p_iTrigOutputPIN;

}


int ToeggeliDistanz::getAktuelleDistanzCm(){
  

    return 0;
   
  }
