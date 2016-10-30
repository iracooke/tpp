Amazon Web Services TPP Simulator (AMZSIM)

Contained within this directory is a simple web based application for simulating
the cost of analyzing an set of MS/MS spectrum files on Amazon Web Services 
using the AMZTPP set of tools.

INSTALLATION

1) Place the contents of this directory under an web server.  The application
is pure html/javascript based and contains no CGI or other types of programs
besides the ExtJS Javascript library.

2) Install ExtJS in the directory extjs/.

LIMITATIONS

* EC2 instances are not "managed".  Neither are EC2 nodes added as needed nor
  are they shutdown as they become unnecessary.  The EC2 costs are based on
  the total runtime and the # of instances inputed.

* No consideration is taken for files that may already been uploaded.  When
  performing multiple searches on the same input the file only has to be
  uploaded once.

* S3 storage does not take into consideration the growth of usage -- meaning
  it simply used the total storage amount over the total runtime.

* S3 storage tiers are not considered, all storage costs are based on the
  first tier usage of $0.095/GB month.
 
