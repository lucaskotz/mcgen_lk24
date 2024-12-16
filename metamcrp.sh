#!/bin/bash
# A script to generate Monte-Carlo replicas and input files to
# generate META PDFs
# Version: August 11, 2015
# Authors: Jun Gao, Pavel Nadolsky
# Run ./metamcrp.sh to print out a help message

# User-configurable parameters ========================================

# If needed, specify special paths to the LHAPDF and BOOST libraries and headers here.
# [The script lhapdf-config included into the LHAPDF installation returns paths
#  to the library and header directories of the LHAPDF distribution. The headers of 
# for BOOST functions can be often installed as a part of the development packages
# for BOOST, such as libboost-all-dev on Ubuntu or boost-devel on Scientific Linux 6.
LHALIB=$(lhapdf-config --libdir)
LHAINC=$(lhapdf-config --incdir)
BOOSTINC=/usr/include/boost/

#Set MP4LHC=yes to output additional files needed for mp4lhc
MP4LHC=no

#========================================================================
# Main part of the script; do not change unless you know what you are doing

echo "metamcrp.sh: a script to generate Monte-Carlo replicas for the META combination"
echo "Author: Jun Gao, Pavel Nadolsky, 2015"
echo "For help message, type: metamcrp.sh help"

 if [[ ( $# -lt 2) ||  ("$1" == "help") ]]; then
    echo "Examples of usage"
    echo "1. metamcrp.sh generate mcgen.card"
    echo "   Generate LHAPDF6 grids for random MC replicas from an input"
    echo "   LHAPDF ensemble, reading the input parameters from mcgen.card"
    echo "2. metamcrp.h combine mcadd.card"
    echo "   Combine grids of LHAPDF ensembles 1, 2, 3...,"
    echo "   reading the input parameters from mcadd.card"
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
# Generate random MC replicas according to the following procedure:
# 1. Generate LHAPDF files for random MC replica
# 2. Convert each output LHAPDF .dat file into a .plt file for META combination
# 3. Compute standard deviations for the META combination, stored in .er files

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
 
    #1. Generate LHAPDF .dat files for random MC replicas =======================
    #Go to the output directory and generate LHAPDF grids for random MC replicas
    cp $gencard $outpdfname
    cd $outpdfname
    ln -sf ../src/mcgen.x .
    ./mcgen.x generate $gencard
   
    [ $? -ne 0 ] && { 
        echo Problem with generation of MC replicas for $outpdfname; 
        exit; }

    #Create a subdirectory for storing output LHAPDF files
    mkdir $outpdfname'/';
    
    #Write an output LHAPDF info file  
    headerinfo="../inc/Header_"$alphasorder"_"$alphasmz".info"
    if [ -e $headerinfo ]; then 
        cp  $headerinfo $outpdfname".info"
        vi -e -s -c "%s/nzzz/$outpdfname/g" -c "%s/nxxx/$[$nmc+1]/g" -c "%s/nyyy/$nmc/g" -c "wq" "$outpdfname".info
        mv *.dat *.info $outpdfname'/';
    else
        echo "Error: cannot create an LHAPDF info file "
        echo "for alpha_s(MZ) = "$alphasmz" at "$alphasorder
        echo "Problem with input alpha_s(MZ)?"
        echo "Only alpha_s(MZ)=0.116, 0.117, 0.118, 0.119, 0.120 are implemented"
    fi    

    echo Wrote LHAPDF replicas 

    #2. Convert LHAPDF .dat files into .plt files ==========================
    if [ "$MP4LHC" == "yes" ]; then
     
      ./mcgen.x convert $outpdfname

      [ $? -ne 0 ] && { 
          echo Problem with conversion of LHAPDF replicas into .plt files;
          exit; }

      echo Converted LHAPDF replicas into .plt files

    #3. Compute .er files with standard deviations for input and output files 
    #=========================
      ./mcgen.x std_devs $inpdfname $err_type
      ./mcgen.x std_devs $outpdfname "mc"

      [ $? -ne 0 ] && { 
          echo Problem with computation of standard deviations; 
          exit; }

      echo Computed standard deviations
 
    fi # if $MP4LHC=="yes"

    #4. Clean the output directory
    rm mcgen.x
    cd ..

    echo Generation of $outpdfname"/" for META combination is completed
    echo The LHAPDF replicas are in $outpdfname"/"$outpdfname
    echo ATTENTION: temporary LHAPDF metadata are written 
    echo into $outpdfname"/"$outpdfname"/"$outpdfname."info" 
    echo Most likely, you need to edit this .info file by copying information 
    echo and alpha_s values from the .info file for the input PDFs
    exit 0
fi #generate -> ============================================================

# Combine several LHAPDF ensembles =========================================
if [[ "$1" == "combine" ]]; then
# Combine ensembles of MC replicas IN the current directory into a new ensemble

    addcard="$2"
    [ -e $addcard ] || 
    {
        echo Error: $addcard does not exist;
        exit 1;
    }

    #Extract the input and output PDF name, other parameters from addcard
    outpdfname=$(grep "output combined PDF ensemble" $addcard |awk '{print $1;'});
    alphasorder=$(grep "order of alpha_s" $addcard |awk '{print $1;'});
    alphasmz=$(grep "alpha_s(MZ)" $addcard |awk '{print $1;'});
    inpdfnames=$(head -n6 $addcard |tail -n1);

    echo "Generating a Monte-Carlo ensemble "$outpdfname" according to "$addcard
    
    #Create or overwrite the output directory
    [ -e $outpdfname ] && {
        echo Overwriting $outpdfname"/";
        rm -fr $outpdfname;  
    }
    mkdir $outpdfname; mkdir $outpdfname"/"$outpdfname
 
    #1. Copy LHAPDF replicas from the input ensembles into the final ensemble
    imc=0; # counter of final replicas
    for inpdf in $inpdfnames; do
        echo Copying replicas from $inpdf
        cd $inpdf/$inpdf;

        FoundCentral=0;
        for i in *.dat; do 
            if [ $(ls $i |grep _0000.dat)"=" != '=' ]; then 
                #Central replica: copied into $outpdfname for averaging
                cp $i ../../$outpdfname"/"
                FoundCentral=1
            else 
                #Error replica: copied into $outpdfname/$outpdfname
                imc=$((imc+1));
                if [ $imc -lt 10 ]; then  #The ID of the final replica
                    smc="_000"$imc;
                elif [ $imc -lt 100 ]; then
                    smc="_00"$imc;
                elif [ $imc -lt 1000 ]; then
                    smc="_0"$imc;
                else 
                    smc="_"$imc;
                fi

                cp $i ../../$outpdfname"/"$outpdfname"/"$outpdfname$smc".dat"
            fi; #[ $(ls $i |grep _0000.dat)"=" != '=' ]
        done

        if [ $FoundCentral -eq 0 ]; then
            echo Error in $inpdf: cannot find the central set $inpdf"_0000.dat"
            exit
        fi

        cd ../..; 
    done #inpdf
    
    echo Copied $imc error replicas into the output directory
    
    #2. Average LHAPDF replicas of the input ensembles
    cp $addcard $outpdfname
    cd $outpdfname
    ln -sf ../src/mcgen.x .

    CentralPDFs=$(ls *_0000.dat)
    ./mcgen.x average $outpdfname"_0000.dat" $CentralPDFs

    [ $? -ne 0 ] && { 
        echo Problem with averaging $CentralPDFs; 
        exit; }

    mv $outpdfname"_0000.dat" $outpdfname"/"
    echo Computed the average of central replicas

    #Write an output LHAPDF info file  
    headerinfo="../inc/Header_"$alphasorder"_"$alphasmz".info"
    if [ -e $headerinfo ]; then 
        cp  $headerinfo $outpdfname".info"
        vi -e -s -c "%s/nzzz/$outpdfname/g" -c "%s/nxxx/$[$imc+1]/g" -c "%s/nyyy/$imc/g" -c "wq" "$outpdfname".info
        mv *.dat *.info $outpdfname'/';
    else
        echo "Error: cannot create an LHAPDF info file "
        echo "for alpha_s(MZ) = "$alphasmz" at "$alphasorder
        echo "Problem with input alpha_s(MZ)?"
        echo "Only alpha_s(MZ)=0.116, 0.117, 0.118, 0.119, 0.120 are implemented"
    fi    

    #3. Convert LHAPDF .dat files into .plt files ==========================
    ./mcgen.x convert $outpdfname
    [ $? -ne 0 ] && { 
        echo Problem with conversion of LHAPDF replicas for combination into .plt files; 
        exit; }

    echo Converted LHAPDF replicas into .plt files

    #4. Compute .er files with standard deviations for the output ensemble
    #=========================
    ./mcgen.x std_devs $outpdfname "mc"
    [ $? -ne 0 ] && { 
        echo Problem with computation of standard deviations; 
        exit; }

    echo Computed standard deviations

    #5. Clean the output directory
    rm mcgen.x 
    cd ..

    echo Combination is completed. The output is in $outpdfname"/".
    echo The LHAPDF replicas are in $outpdfname"/"$outpdfname"." 
    echo Now edit the new LHAPDF meta-data in $outpdfname"/"$outpdfname"/"$outpdfname."info" by hand.
    exit 0

fi #combine -> =============================================================
