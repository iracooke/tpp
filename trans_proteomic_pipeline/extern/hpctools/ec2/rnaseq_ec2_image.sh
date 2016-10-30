#!/bin/bash 
#
# Script for preparing a TPP Amazon EC2 image from a stock ubutunu based EC2 
# image.  Much of this script was taken from the original work done by
# Insilicos to run the TPP on the cloud.
#

TPP_VERSION=${TPP_VERSION:-4.6.2}
TPP_DIR=/opt/tpp
TPP_DATA=/mnt/tppdata
TPP_USERS=$TPP_DIR/users

set -e		# Exit on error

ROOTDIR=`pwd`

cd /tmp

echo "### Installing Bowtie..."
#wget http://sourceforge.net/projects/bowtie-bio/files/bowtie2/2.1.0/bowtie2-2.1.0-linux-x86_64.zip
wget http://sourceforge.net/projects/bowtie-bio/files/bowtie/1.0.0/bowtie-1.0.0-linux-x86_64.zip
unzip bowtie-1.0.0-linux-x86_64.zip
mv bowtie-1.0.0 /opt
find /opt/bowtie-1.0.0 -type d -exec chmod a+rx {} \;
chmod a+rwx /opt/bowtie-1.0.0/bowtie-build
chmod a+x /opt/bowtie-1.0.0/bowtie
chmod -R g-w /opt/bowtie-1.0.0
echo 'export PATH=$PATH:/opt/bowtie-1.0.0' | tee -a /etc/profile.d/tpp.sh
echo 'export PATH=$PATH:/opt/bowtie-1.0.0/scripts' | tee -a /etc/profile.d/tpp.sh
echo "### Done with Bowtie"

echo "### Installing cufflinks..."
wget http://cufflinks.cbcb.umd.edu/downloads/cufflinks-2.1.1.Linux_x86_64.tar.gz
tar xvzf cufflinks-2.1.1.Linux_x86_64.tar.gz
mv cufflinks-2.1.1.Linux_x86_64 /opt/cufflinks-2.1.1
chown -R root:root /opt/cufflinks*
chmod -R g-w /opt/cufflinks-2.1.1/
echo 'export PATH=$PATH:/opt/cufflinks-2.1.1' | tee -a /etc/profile.d/tpp.sh
echo "### Done with cufflinks"

echo "### Installing samtools..."
apt-get -y install libncurses-dev
wget http://sourceforge.net/projects/samtools/files/samtools/0.1.19/samtools-0.1.19.tar.bz2
tar xvf samtools-0.1.19.tar.bz2
cd samtools-0.1.19
make
mkdir /opt/samtools-0.1.19
MISC=$(find misc -type f -perm -u=x)
cp samtools bcftools/bcftools $MISC /opt/samtools-0.1.19
echo 'export PATH=$PATH:/opt/samtools-0.1.19' | tee -a /etc/profile.d/tpp.sh
cd /tmp
echo "### Done with samtools"

echo "### Installing SnpEff..."
wget http://sourceforge.net/projects/snpeff/files/snpEff_latest_core.zip
unzip snpEff_latest_core.zip
mv snpEff /opt
chown -R root:root /opt/snpEff
echo 'export PATH=$PATH:/opt/snpEff/scripts' | tee -a /etc/profile.d/tpp.sh
echo "### Done with SnpEff"

echo "### Installing tophat..."
wget http://tophat.cbcb.umd.edu/downloads/tophat-2.0.8b.Linux_x86_64.tar.gz
tar xvzf tophat-2.0.8b.Linux_x86_64.tar.gz
mv tophat-2.0.8b.Linux_x86_64 /opt/tophat-2.0.8b
chown -R root:root /opt/tophat*
chmod -R g-w /opt/tophat-2.0.8b
echo 'export PATH=$PATH:/opt/tophat-2.0.8b' | tee -a /etc/profile.d/tpp.sh
echo "### Done with tophat"

echo "### Installing SRA Toolkit..."
wget http://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/2.3.2-5/sratoolkit.2.3.2-5-ubuntu64.tar.gz
tar xvzf sratoolkit.2.3.2-5-ubuntu64.tar.gz
mv sratoolkit.2.3.2-5-ubuntu64 /opt/sratoolkit.2.3.2-5
chown -R root:root /opt/sratoolkit*
chmod -R g-w /opt/sratoolkit.2.3.2-5
echo 'export PATH=$PATH:/opt/sratoolkit.2.3.2-5/bin' | tee -a /etc/profile.d/tpp.sh
echo "### Done with SRA Toolkit"

echo "### Installing FastQC..."
wget http://www.bioinformatics.babraham.ac.uk/projects/fastqc/fastqc_v0.10.1.zip
unzip fastqc_v0.10.1.zip
mv FastQC /opt
chmod a+x /opt/FastQC/fastqc
chown -R root:root /opt/FastQC
echo 'export PATH=$PATH:/opt/FastQC' | tee -a /etc/profile.d/tpp.sh
echo "### Done with FastQC"

echo "### Installing TPP rnaseq..."
svn export https://svn.code.sf.net/p/sashimi/code/trunk/rnaseq rnaseq
mv rnaseq /opt/tpp
chmod a+rx /opt/tpp/rnaseq
chmod a+rx /opt/tpp/rnaseq/bin
chmod a+rx /opt/tpp/rnaseq/lib
export PERL=$(which perl)
sed -i "1 s|^#\!.*perl|#!/usr/bin/perl|" /opt/tpp/rnaseq/bin/*
echo "export PATH=\$PATH:/opt/tpp/rnaseq/bin" | tee -a /etc/profile.d/tpp.sh
echo "### Done with TPP rnaseq"
