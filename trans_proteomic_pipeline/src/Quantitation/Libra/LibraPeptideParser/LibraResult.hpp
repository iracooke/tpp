#ifndef LIBRARESULT_HPP
#define LIBRARESULT_HPP

#include "Intensity.hpp"
#include "Quantitation.hpp"
#include "typedefs.hpp"

#include "Parsers/Parser/Tag.h"
#include "common/Array.h"

#include <vector>
#include <utility>

using std::vector;
using std::pair;
using std::make_pair;


class LibraResult
{
private:

    LibraConditionHandler* pLibraConditionHandler;

    Intensity* pIntensity;

    char* mzXMLFile;

    int scanNum;


public:

    LibraResult(LibraConditionHandler*, char*, int scanNum );

    ~LibraResult();

    vector<double> sumIntensities();

    vector<double> getTargetMasses();

    vector<double> getAbsoluteIntensities();

    vector<double> getNormalizedIntensities();

    int getNumberOfChannels();
  
    Array<Tag*>* getPepXMLTags();
  
};

#endif
