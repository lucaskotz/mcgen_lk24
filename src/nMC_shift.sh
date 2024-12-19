#!/usr/bin/env bash
# Check if at least one argument is provided
if [ $# -eq 0 ]; then
    echo "No arguments provided."
    echo "Usage: ./nMC_shift.sh [Hessian PDF folder name] [nMC replica folder name] [number of Hessian PDF error sets] [number of nMC replicas (excluding central)]"
    exit 1
fi

# Assign arguments to variables for clarity
hessianPDF="$1"
nMCname="$2"
NHessian="$3"
NnMC="$4"

nMCcentral=$nMCname/${nMCname}_0000.dat;

for inMC in $(seq -f "%04g" 1 $NnMC); do

    outputname=${nMCname}_shifted_hessian_${inMC}
    mkdir ${outputname}
    cp ${hessianPDF}/${hessianPDF}.info ${outputname}/${hessianPDF}_${inMC}_shifted.info

    inMCPDF=${nMCname}/${nMCname}_${inMC}.dat
    
    ./mcgen.x add nMC_shift.dat ${nMCcentral} ${inMCPDF} -1 1
    
    for iHessian in $(seq -f "%04g" 0 $NHessian); do
	iShiftPDF=${hessianPDF}_${inMC}_shifted_${iHessian}.dat
	iHessianPDF=${hessianPDF}/${hessianPDF}_${iHessian}.dat
	./mcgen.x add ${iShiftPDF} ${iHessianPDF} nMC_shift.dat 1 1
	mv ${iShiftPDF} ${outputname}
    done
    
    rm -f nMC_shift.dat

done
