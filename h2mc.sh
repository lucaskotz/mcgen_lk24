#!/bin/bash
# A script to generate Monte-Carlo replicas from Hessian replicas
# Version: June 2016
# Authors: Jun Gao, Pavel Nadolsky
# Run ./h2mc.sh to print out a help message

# User-configurable parameters ========================================

# If needed, specify special paths to the LHAPDF and BOOST libraries 
# and headers here. [The script lhapdf-config included into the LHAPDF 
# installation returns paths to the library and header directories 
# of the LHAPDF distribution. The headers for BOOST functions can be often 
# installed as a part of the development packages for BOOST, such as 
# libboost-all-dev on Ubuntu or boost-devel on Scientific Linux 6.
LHALIB=$(lhapdf-config --libdir)
LHAINC=$(lhapdf-config --incdir)
BOOSTINC=/usr/include/boost/

#========================================================================
# Main part of the script; do not change unless you know what you are doing

echo "h2mc.sh: a script to generate Monte-Carlo replicas from Hessian PDFs"
echo "Author: Jun Gao, Pavel Nadolsky, 2016"
echo "For help message, type: h2mc.sh help"

 if [[ ( $# -lt 2) ||  ("$1" == "help") ]]; then
    echo "Example of usage"
    echo " h2mc.sh generate mcgen.card"
    echo "   Generate LHAPDF6 grids for random MC replicas from an input"
    echo "   LHAPDF ensemble, reading the input parameters from mcgen.card"
    echo
    echo "Please read README with usage guidelines"
    exit 0
fi #help -> =============================================================

#compiling the c++ executables if needed 
LIBS6='-L '$LHALIB' -lLHAPDF'
CXX='g++'
CXXFLAGS6='-I '$LHAINC' -I '$BOOSTINC
echo Updating C++ executables
make -C ./src LHALIB=$LHALIB LHAINC=$LHAINC BOOSTINC=$BOOSTINC

# Generation must be able to find a subdirectory with an LHAPDF ensemble 
# within the work directory
export LHAPDF_DATA_PATH="./:"$LHAPDF_DATA_PATH

# Generate MC replicas ==================================================
if [[ "$1" == "generate" ]]; then
# Generate LHAPDF files for random MC replicas 

    gencard="$2"
    [ -e $gencard ] || 
    {
        echo Error: $gencard does not exist;
        exit 1;
    }

    #Extract the input and output PDF name, other parameters from gencard
    inpdfname=$(grep "input PDF ensemble from LHAPDF6" $gencard |awk '{print $1;'});
    outpdfname=$(grep "output PDF ensemble" $gencard |awk '{print $1;'});
    err_type=$(grep " input error type," $gencard |awk '{print $1;'});
    nmc=$(grep "number of MC replicas to generate" $gencard |awk '{print $1;'});
    alphasorder=$(grep "order of alpha_s" $gencard |awk '{print $1;'});
    alphasmz=$(grep "alpha_s(MZ)" $gencard |awk '{print $1;'});

    echo "Generating "$nmc" Monte-Carlo replicas  for "$outpdfname" according to "$gencard
    
    #Create or overwrite the output directory
    [ -e $outpdfname ] && rm -fr $outpdfname;  
    mkdir $outpdfname; 
 
    #Generate LHAPDF .dat files for random MC replicas =======================
    #Go to the output directory and generate LHAPDF grids for random MC replicas
    cp $gencard $outpdfname
    cd $outpdfname
    ln -sf ../src/mcgen.x .
    ./mcgen.x generate $gencard
   
    [ $? -ne 0 ] && { 
        echo Problem with generation of MC replicas for $outpdfname; 
        exit; }

    #Write an output LHAPDF info file  
    headerinfo="../inc/Header_"$alphasorder"_"$alphasmz".info"
    if [ -e $headerinfo ]; then 
        cp  $headerinfo $outpdfname".info"
        vi -e -s -c "%s/nzzz/$outpdfname/g" -c "%s/nxxx/$[$nmc+1]/g" -c "%s/nyyy/$nmc/g" -c "wq" "$outpdfname".info
    else
        echo "Error: cannot create an LHAPDF info file "
        echo "for alpha_s(MZ) = "$alphasmz" at "$alphasorder
        echo "Problem with input alpha_s(MZ)?"
        echo "Only alpha_s(MZ)=0.116, 0.117, 0.118, 0.119, 0.120 are implemented"
    fi    

    #Clean the output directory
    rm mcgen.x
    cd ..

    echo Generation of LHAPDF files for MC replicas in $outpdfname"/" 
    echo is completed
    echo Now edit the new LHAPDF meta-data in 
    echo $outpdfname"/"$outpdfname."info" by hand if necessary
    exit 0
fi #generate -> ============================================================
