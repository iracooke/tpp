#ifndef LIBRAWRAPPER_HPP
#define LIBRAWRAPPER_HPP

#include "LibraConditionHandler.hpp"
#include "LibraResult.hpp"
#include "LibraSummary.hpp"
#include "Quantitation.hpp"

#include<vector>
#include<utility>
#include "typedefs.hpp"

class LibraWrapper
{
private:

    char* mzXMLFile;

    LibraConditionHandler* pLibraConditionHandler;

public:

    LibraWrapper( LibraConditionHandler* , char* );

    LibraWrapper( char* , char* );

    ~LibraWrapper();

    LibraSummary* getLibraSummary();

    LibraResult* getLibraResult(int scanNum);

};

#endif
