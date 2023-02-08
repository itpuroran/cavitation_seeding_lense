import os
import math

# input parameters
h_start, h_stop, delta_h = 4, 4, 1 # interval for lens height
R_start, R_stop, delta_R = 7, 7, 1 # interval for lens radios
n_copy = 2  # the number of copy
n = 2 # then number of repulsive spheres at the one side
input_file = 'create_cavity.txt' 
encoding =  'utf-8' # 'cp1252'
# full list of encoding: https://docs.python.org/3/library/codecs.html#standard-encodings


def rewrite_input_file(h: float, R: float, n: int, copy: int, n_h: int, n_R: int, 
                        all_lines: list, dump_file: str) -> list:
    
    with open(dump_file, 'r', encoding=encoding) as f_in:
        f_in.readline() # ITEM: TIMESTEP
        f_in.readline() # timestep
        f_in.readline() # ITEM: NUMBER OF ATOMS
        n_atoms = int(f_in.readline().strip())
        f_in.readline() # ITEM: BOX BOUNDS pp pp pp
        lx0, lxl = [float(i) for i in f_in.readline().strip().split(' ')]
        ly0, lyl = [float(i) for i in f_in.readline().strip().split(' ')]
        lz0, lzl = [float(i) for i in f_in.readline().strip().split(' ')]

    x0, y0, z0 = (lx0+lxl)/2, (ly0+lyl)/2, (lz0+lzl)/2
    L = math.sqrt(R**2 - (R-h)**2)
    
    all_coords =[(x0+i*k*L/(n+1), y0, z0, i + 2) for i in range(1, n + 1) for k in [-1, 1]]
    all_coords.extend([(x0, y0+i*k*L/(n+1), z0, i + 2) for i in range(1, n + 1) for k in [-1, 1]])
    all_coords.insert(0, (x0, y0, z0, 2))

    radii = [0 for _ in range(n + 3)]
    coords = [(x0+i*L/(n+1), y0, z0, i + 2) for i in range(n + 1)]
    for coord in coords:
        radii[coord[3]] = R - math.sqrt((R - h)**2 + (coord[0] - x0)**2)

    proportional_coeff = [rad / h for rad in radii]

    new_lines = []
    for line in all_lines:
        if 'variable	R equal' in line:
            new_lines.append(f'variable	R equal {R}\n')
        elif 'variable	h equal' in line:
            new_lines.append(f'variable	h equal {h}\n')
        elif 'variable	n equal' in line:
            new_lines.append(f'variable	n equal {n}\n')
        elif 'create_box	2 box' in line:
            new_lines.append(f'create_box	{n + 2} box\n')
        elif 'variable	N equal' in line:
            new_lines.append(f'variable	N equal {n_atoms}\n')
        elif  'read_dump	dump_start.txt' in line:
            new_lines.append(f'read_dump	dumpL/dump_start_{copy}.txt 0 x y z vx vy vz box yes replace yes\n')
        elif 'variable    self_name' in line:
            new_lines.append(f'variable        self_name string "create_cavity_files/create_cavity_h{n_h}_r{n_R}_c{copy}"\n')
        
        elif '# set coordinates' in line:
            new_lines.append(line)
            par_count = 0
            for coord in all_coords:
                new_lines.append(f'variable 	center_x_{par_count}	equal 	{coord[0]}\n')
                new_lines.append(f'variable 	center_y_{par_count}	equal 	{coord[1]}\n')
                new_lines.append(f'variable 	center_z_{par_count}	equal 	{coord[2]}\n')
                par_count += 1
        
        elif '# set proportional coefficients' in line:
            new_lines.append(line)
            par_count = 2
            for coeff in proportional_coeff[2:]:
                new_lines.append(f'variable	prop_coeff_{par_count}	equal	{coeff}\n')
                par_count += 1 
        
        elif '# set regions and delete' in line:
            new_lines.append(line)
            for i in range(len(all_coords)):
                new_lines.append(f'region 		small_sphere_{i} sphere ${{center_x_{i}}} ${{center_y_{i}}} ${{center_z_{i}}} ${{r_0}} units box\n')
                new_lines.append(f'delete_atoms	region small_sphere_{i}\n')

        elif '# set adding repulsive atoms' in line:
            new_lines.append(line)
            for i in range(len(all_coords)):
                new_lines.append(f'create_atoms	{all_coords[i][3]} single ${{center_x_{i}}} ${{center_y_{i}}} ${{center_z_{i}}} units box\n')
                new_lines.append(f'mass		{all_coords[i][3]} 10000000000.0\n')
            new_lines.append(f'pair_coeff	* * 1.0 1.0 0.0001\n')
            new_lines.append(f'pair_coeff	1 1 1.0 1.0 6.576\n')

        elif 'dump	all_dump' in line:
            new_lines.append(f'dump	all_dump all custom 150 dumps_twophase/dump_cluster_h{n_h}_r{n_R}_c{copy} id type x y z vx vy vz\n')
        
        elif '# set sigma for all repulsive particles' in line:
            for i in range(2, n + 3):
                new_lines.append(f'     variable new_sigma_{coords[i - 2][3]} equal ${{new_sigma}}*${{prop_coeff_{i}}}\n')

        elif '# set all pair coefficients' in line:
            for i in range(2, n + 3):
                new_lines.append(f'     pair_coeff  1 {coords[i - 2][3]} ${{epsilon_2}} ${{new_sigma_{all_coords[i][3]}}}  6.576\n')

        else:
            new_lines.append(line)
        
    return new_lines


if __name__=="__main__":

    if not os.path.exists('create_cavity_files'):
        os.makedirs('create_cavity_files')

    if not os.path.exists('dumps_twophase'):
        os.makedirs('dumps_twophase')

    all_h = [h_start]
    h_count = h_start
    while h_count <= h_stop - delta_h:        
        h_count = h_count + delta_h
        all_h.append(h_count)

    all_R = [R_start]
    R_count = R_start
    while R_count <= R_stop - delta_R:        
        R_count = R_count + delta_R
        all_R.append(R_count)

    all_lines = []
    with open(input_file, 'r', encoding=encoding) as f_in:
        all_lines = f_in.readlines()

    for i in range(len(all_h)):
        for j in range(len(all_R)):
            for copy in range(n_copy):
                dump_file = f'dumpL/dump_start_{copy}.txt'
                rewite_lines = rewrite_input_file(all_h[i], all_R[j], n, copy, i, j, all_lines, dump_file)
                file_new = f'create_cavity_files/create_cavity_h{i}_r{j}_c{copy}'
                with open (file_new, 'w', encoding=encoding) as f_out:
                    f_out.writelines(rewite_lines)
