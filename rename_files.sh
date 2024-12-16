#!/bin/bash

# Define an array of directory names (new_prefixes)
directories=("14_output_Nm2_FixC_LFixCP_updated_xrange" "17_output_Nm2_FreeC_HFixCP_5000_updated_xrange" "305_output_Nm1_Fre\
eC_01FixCP_xP075_updated_xrange" "702_output_Nm1_FreeC_LFixCP_Bs2_xstretch_updated_xrange" "8108_piplus")

for new_prefix in "${directories[@]}"; do
    # Check if the directory exists
    if [ -d "$new_prefix" ]; then
        # Change to the directory
        cd "$new_prefix/$new_prefix"

        # Check if there are files to rename
        if [ -f "xFitterPI_*.dat" ] || [ -f "xFitterPI.info" ]; then
            # Rename .dat files
            for file in xFitterPI_*.dat; do
                new_name="${file/xFitterPI/$new_prefix}"
                mv "$file" "$new_name"
            done

            # Rename .info file
            mv xFitterPI.info "${new_prefix}.info"
        fi

        # Change back to the parent directory
        cd ../..
    fi
done
