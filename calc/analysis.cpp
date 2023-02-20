// analysis of growing probability
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <string>
#include <sstream>
#include <queue>
#include <ctime>

using namespace std;

void read_results (std::string fout_dump, std::vector<double> &start, std::vector<double> &finish)
{
	 int line_count = 0;
	 std::ifstream file;
	 std::string line;
	 file.open(fout_dump);
    
	 while (!std::getline(file,line).eof())
	 {
		 std::string buf; // Have a buffer string
		 std::stringstream ss(line); // Insert the string into a stream
		 std::vector<std::string> tokens; // Create vector to hold our words
		 while (ss >> buf)
		     tokens.push_back(buf);
         if ((tokens.size() == 3)&&(line_count == 0))	
		 {
			 start[0] = std::stoi(tokens[0]);
             start[1] = std::stoi(tokens[1]);
             start[2] = std::stod(tokens[2]);
		 } 
		 else if ((tokens.size() == 3)&&(line_count > 0))
		 {
			 finish[0] = std::stoi(tokens[0]);
             finish[1] = std::stoi(tokens[1]);
             finish[2] = std::stod(tokens[2]); 
		 }
         line_count++;
     }
         file.close();

}

     int main(int argc, char* argv[]) 
{ 
     int num_h = std::stoi(argv[1]);
     int num_rad = std::stoi(argv[2]);
	 int num_copy = std::stoi(argv[3]);

	 std::vector<std::vector<std::vector<double>>>  cluster_volume(num_h+1, std::vector<double>(num_rad+1, std::vector<double>(num_copy+1)));
	 std::vector<std::vector<std::vector<double>>>  cluster_volume0(num_h+1, std::vector<double>(num_rad+1, std::vector<double>(num_copy+1)));

	 std::string file_result_av="Result_av.txt";
         std::ofstream f_result_av(file_result_av,std::ios_base::out | std::ios_base::app);
         std::string file_result_gen="Result_gen.txt";
         std::ofstream f_result_gen(file_result_gen,std::ios_base::out | std::ios_base::app);
		 int is_growth;
 for (int h = 0; h < num_rad; ++h)
 {
    for (int r = 0; r < num_rad; ++r) 
 {
	 double cluster_volume0_av = 0;
	 double grow_prob = 0;
     std::vector<double> start(3);
     std::vector<double> finish(3);

	 for (int c = 0; c < num_copy; ++c) 
	 {
		 int time_start = time(NULL);	 
		 std::string file_result="results/result_h" + h + "_r" +std::to_string(r)+ "_c" + std::to_string(c) +".txt";
         read_results(file_result,start, finish);
		 std::cout << h << " " << r << " " << c <<  std::endl;

		 cluster_volume0[h][r][c] = start[2];
		 cluster_volume0_av = cluster_volume0_av + cluster_volume0[r][c];
		 cluster_volume[h][r][c] = finish[2];
		 if (cluster_volume[h][r][c] > cluster_volume0[h][r][c])
		 	 {
			 ++grow_prob;
		 	 is_growth = 1;
			 }
		 else
		 {
			 is_growth = 0;
		 }		 
		 f_result_gen << " " << start[2] << " " << finish[2] << " " << is_growth << std::endl;
	 }
	 cluster_volume0_av = cluster_volume0_av/num_copy;
	 grow_prob = grow_prob/num_copy;
	 f_result_av << h << " " << std::to_string(r) << " " << std::to_string(cluster_volume0_av) << " " << std::to_string(grow_prob) << std::endl;
 }
 } 
 
 f_result_av.close();
 f_result_gen.close();
}

