#include "LibraResult.hpp"

/**
* Constructor for LibraResult object creates a quantitation
* object and intensity object, turns off quantitation's
* print to outfile and print to std out.
* @param pointer to condition object
* @param name of mzXML file
* @param start scan number
* @param end scan number
*/
LibraResult::LibraResult(LibraConditionHandler* pCond, char* mzXMLFileName,
int startScanNum )
{

    if (dbug) cout << "LibraResult::LibraResult(LibraConditionHandler*,char*,int)"<<endl;

    scanNum = startScanNum;

    mzXMLFile = mzXMLFileName;

    pLibraConditionHandler = pCond;

    bool b = false;

    pIntensity = new Intensity(pLibraConditionHandler, mzXMLFileName, 
    scanNum );

}



LibraResult::~LibraResult()
{

    delete pIntensity;

}



vector<double> LibraResult::getTargetMasses()
{

    return pIntensity->getTargetMass();
    
}


vector<double> LibraResult::getAbsoluteIntensities()
{

    return pIntensity->getAbsoluteIntensities();
    
}


vector<double> LibraResult::getNormalizedIntensities()
{

    return pIntensity->getNormalizedIntensities();
    
}



int LibraResult::getNumberOfChannels()
{

    return pIntensity->getNumberOfChannels();

}



  Array<Tag*>* LibraResult::getPepXMLTags() {
    char text[200];
    Array<Tag*>* output = new Array<Tag*>;
    Tag* next = new Tag("libra_result", True, False);
    output->insertAtEnd(next);
    vector<double> masses = getTargetMasses();
    vector<double> abs = getAbsoluteIntensities();
    vector<double> norm = getNormalizedIntensities();

    for(int k = 0; k < getNumberOfChannels(); k++) {
      next = new Tag("intensity", True, True);  
      sprintf(text, "%d", k+1);
      next->setAttributeValue("channel", text);

      sprintf(text, "%0.3f", masses[k]);
      next->setAttributeValue("target_mass", text);
      sprintf(text, "%0.3f", abs[k]);
      next->setAttributeValue("absolute", text);
      sprintf(text, "%0.3f", norm[k]);
      next->setAttributeValue("normalized", text);
      if(norm[k] > 0)
	sprintf(text, "%0.3f", abs[k]/norm[k]);
      else
	sprintf(text, "%0.0f", -999.0);
      // Hi Andy, we should remove the normalization_factor tag...I'll explain ...
      //next->setAttributeValue("normalization_factor", text);


      output->insertAtEnd(next);
    } // next channel
    next = new Tag("libra_result", False, True);
    output->insertAtEnd(next);
    return output;

  }
