#!/bin/bash
#SBATCH --job-name main
#SBATCH -q secondary
#SBATCH -N 1
#SBATCH --ntasks-per-node=32
#SBATCH --mem=12G
#SBATCH --constraint=amd
#SBATCH -o output.out
#SBATCH -e errors.err
#SBATCH -t 7-0:0:0


module load openmpi3/3.1.0
mpirun -np 32 --mca btl ^openib ./main

