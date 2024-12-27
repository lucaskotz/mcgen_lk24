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
seedfile="$5"

echo "$hessianPDF $nMCname $NHessian $NnMC $seedfile"

# All zero-padded to 4 digits for consistency
mapfile -t random_order < <(seq -f "%04g" 1 ${NnMC} | shuf --random-source=${seedfile})

echo "random_order length: ${#random_order[@]}"

# Generate the list of files from 1 to 100, zero-padded to 4 digits
files=$(seq -f "${nMCname}/${nMCname}_%04g.dat" 1 $NnMC)

outputname=${hessianPDF}_MC_shifted-Hessian+unshifted-nuclear
mkdir ${outputname}

# Run the command with the generated file list
./mcgen.x average "${nMCname}_avg.dat" $files

cp "${hessianPDF}/${hessianPDF}.info" "${outputname}/${hessianPDF}_MC_shifted-Hessian+unshifted-nuclear.info"
cp "${hessianPDF}/${hessianPDF}_0000.dat" "${outputname}/${hessianPDF}_MC_shifted-Hessian+unshifted-nuclear_0000.dat"

echo "Adding nMC shift to Hessian sets."

for k in $(seq -f "%04g" 1 $NHessian); do

    kp="${random_order[$((10#$k - 1))]}"

    knMCPDF="${nMCname}/${nMCname}_${kp}.dat"
    
    Deltakfile=${hessianPDF}_nuclear_shift.dat
    nHfile=${hessianPDF}_MC_shifted-Hessian+unshifted-nuclear_$k.dat
    kHessian=${hessianPDF}/${hessianPDF}_$k.dat
    
    ./mcgen.x add "${Deltakfile}" ${nMCname}_avg.dat ${knMCPDF} -1 1

    ./mcgen.x add "${nHfile}" ${Deltakfile} ${kHessian} 1 1

    mv ${nHfile} ${outputname}
    
    rm -f ${Deltakfile}
done

echo "Finished nMC_shift for $hessianPDF"
