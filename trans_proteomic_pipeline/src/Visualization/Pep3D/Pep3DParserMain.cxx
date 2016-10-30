#include "Pep3DParser.h"
#include "common/constants.h"

int main(int argc, char** argv) {

  Pep3DParser* parser = new Pep3DParser(argv[1], argv+2, argc-2);

  if(parser != NULL) {
    cout << "retrieved " << parser->getNumDataEntries() << " entries" << endl;
    Pep3D_dataStrct* data = parser->getPep3DDataStrct();

    for(int k = 0; k < parser->getNumDataEntries(); k++)
      cout << data[k].startScan << " " << data[k].endScan << " " << data[k].charge << " " << data[k].prob << " " << data[k].score << endl;


  }



  return 0;
}
