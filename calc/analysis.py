n_h = 2
n_rad = 3
n_copy = 4

all_results =[[[0] * n_copy for _ in range(n_rad)] for _ in range(n_h)]

file_all_results = open('Results_all.txt', 'w')
file_all_results.write('height radius copy timestep n_cells volume\n')
file_average = open('Results_average.txt', 'w')
file_average.write('height radius growth_prob\n')

for h in range(n_h):
    for r in range(n_rad):
        for c in range(n_copy):

            result_file = f"results/result_h{h}_r{r}_c{c}"
            with open(result_file, 'r') as f_in:

                line = f_in.readline()
                file_all_results.write(f'{h} {r} {c} {line}')
                data = line.split(' ')
                volume_start = float(data[1])

                line = f_in.readline()
                file_all_results.write(f'{h} {r} {c} {line}')
                data = line.split(' ')
                volume_stop = float(data[1])

            all_results[h][r][c] = int(volume_stop >= volume_start)

    prob = sum(all_results[h][r]) / n_copy
    file_average.write(f'{h} {r} {prob}')

file_average.close()
file_all_results.close()



