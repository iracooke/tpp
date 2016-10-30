all:	psm2pdf

clean:
	rm -f *.o psm2pdf

psm2pdf:	psm2pdf.o
	g++ -O3 -o psm2pdf psm2pdf.o 

psm2pdf.o:	psm2pdf.o
	g++ -O3 -o psm2pdf.o psm2pdf.cxx
