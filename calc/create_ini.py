#! /usr/bin/env python

# This file is used to take a template input json file for forward flux
# and create a input file for ssages that uses multiple walkers.

import json 
import numpy as np
import os
from random import randint

class LessPrecise(float):
    def __repr__(self):
        return str(self)

if not os.path.exists('ini_files'):
        os.makedirs('ini_files');

if not os.path.exists('profiles'):
        os.makedirs('profiles');

if not os.path.exists('logs'):
        os.makedirs('logs');

if not os.path.exists('ptu'):
        os.makedirs('ptu');

if not os.path.exists('dumps_end'):
        os.makedirs('dumps_end');



def lammps_random_number(lammps_filename, nWalkers):
    """Takes a lammps input file, finds the seed number of langevin, replaces it with a random number, and
	generates new input files
	"""
    randon_numbers = set()
    while (len(randon_numbers) < nWalkers):
        randon_numbers.add(randint(0, 100000))
    randon_numbers = list(randon_numbers)

    in_lammps_file = open(lammps_filename, 'r')

    for r in range(0, nrad):
        for h in range(0, nh): 
            for c in range(0, ncopy):
                out_lammps_file = open('ini_files/'+lammps_filename + '_h' + str(h)+'_r'+str(r) + '_c' + str(c), 'w')
                for line in in_lammps_file:
                    if line.find('seed_date equal') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 3:
                                line = line.replace(columns[3], str(randon_numbers[r]))
                                line = line + '\n'
                    if line.find('log') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 1:
                                line = line.replace(columns[1], 'logs/log_h'+str(h)+'_r'+str(r)+'_c'+str(c))
                                line = line + '\n'
                    if line.find('all ave/time') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 10:
                                line = line.replace(columns[10], 'ptu/pt_h'+str(h)+'_r'+str(r)+'_c'+str(c))
                                line = line + '\n'
                    if line.find('read_dump') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 1:
                                line = line.replace(columns[1], 'dumps_twophase/dump_cluster_h'+str(h)+'_r'+str(r)+'_c'+str(c))  
                                line = line + '\n'
                    if line.find('all_dump') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 3:
                                line = line.replace(columns[5], 'dumps_end/dump_end_h'+str(h)+'_r'+str(r)+'_c'+str(c))
                                line = line + '\n'
                    if line.find('fix aveRUN') != -1:
                        line = line.strip()
                        if not line.startswith('#'):
                            columns = line.split()
                            if len(columns) >= 3:
                                line = line.replace(columns[12], 'profiles/Profiles_run_h'+str(h)+'_r'+str(r)+'_c'+str(c))
                                line = line + '\n'
                    out_lammps_file.write(line)
                in_lammps_file.seek(0)
                out_lammps_file.close()

    in_lammps_file.close()


# User must set these variables
nh = _
nrad = _
ncopy = _

nWalkers=nrad*ncopy
input_filename = "cav_npt"

# Use if you have many equally-spaced interfaces

#Generate the new lammps input files
lammps_random_number(input_filename, nWalkers)
