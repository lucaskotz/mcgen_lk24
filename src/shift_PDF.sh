#!/usr/bin/env bash

# Read files from file_list.txt into an array
Hessian_files=($(cat file_Hessian_list.txt))
nMC_files=($(cat file_nMC_list.txt))
seed_files=($(cat file_seeds_list.txt))

# Define a specific value for each file.
# For example, if file_list.txt contains 4 files, define an array of 4 corresponding values.
values=(100 100 100 100)

# Check that we have the same number of values as files.
if [ ${#Hessian_files[@]} -ne ${#values[@]} ]; then
    echo "Error: The number of files and values do not match."
    exit 1
fi

# Loop through files by index
for idx in "${!Hessian_files[@]}"; do
    i="${Hessian_files[idx]}"
    j="${nMC_files[idx]}"
    iseed="${seed_files[idx]}"
    val="${values[idx]}"

    echo "$i $j $iseed $val"
    
    echo "Starting nMC_shift.sh for $i."
    
    # Perform your operations
    echo "Copying Hessian fits from Hessian_fits/${i}"
    cp -r "Hessian_fits/${i}" .

    echo "Copying nMC fits from packaged_nMC_fits/${j}"
    cp -r "packaged_nMC_fits/${j}" .

    echo "Copying seed file from seeds/${iseed}"
    cp "seeds/${iseed}" .
    
    # Run your script and pass the value as an argument if needed
    ./nMC_shift.sh ${i} ${j} $val 100 $iseed

    
    rm -r "${j}"
    rm -r "${i}"
    rm -f "${iseed}"
    
done
