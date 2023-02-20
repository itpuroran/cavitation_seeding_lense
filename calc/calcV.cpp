// new metod for searching cluster
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

   bool space(char c)    // для разделения строки
 {
   return isspace(c);
 }

 bool not_space(char c)  // для разделения строки
 {
   return !isspace(c);
 }

  std::vector<std::string> split(const std::string& str)  // разделение строки
{
   typedef std::string::const_iterator iter;
   std::vector<std::string> ret;

   iter i = str.begin();
   while (i != str.end()) {

      // ignore leading blanks
      i = find_if(i, str.end(), not_space);

      // find end of next word
      iter j = find_if(i, str.end(), space);

      // copy the characters in [i,j)
      if (i != str.end()) {
         ret.push_back(std::string(i,j));
      }
      i = j;
   }
   return ret;
}

  std::map<std::string, std::string> readparams(const std::string fname)  // чтение входных параметров
{
   std::map<std::string, std::string> params;
   std::ifstream infile;
   infile.open(fname.c_str());
   std::string sline;
   std::vector<std::string> spline;
     
   // warning: there is no real error checking here   
   while (infile) { 

      std::getline(infile,sline);

      // comments # must be first char on line
      if (sline[0] != '#') {
         spline = split(sline);
         if (spline.size() == 2) {
            params[spline[0]] = spline[1];
         }
      }
   }
   infile.close();
   return params;
}

void read_dump (int n_atoms, int num_reading, std::vector<std::vector<double>> &positions, std::vector<int> &neighbors, std::string fout_dump, \
				double &lboxx0, double &lboxxl, double &lboxy0, double &lboxyl, double &lboxz0, double &lboxzl)
{
	 int line_count = 0;
	 std::ifstream file;
	 std::string line;
	 file.open(fout_dump);
    
	 int line_start = num_reading * (n_atoms + 9);
	 int line_stop = line_start + (n_atoms + 9);
	 std::cout << line_start <<" " << line_stop << std::endl;
	 while (line_count < line_start)
	 {
		 if (!std::getline(file,line).eof())
		 {
			 line_count++;
		 }
		 else
		 {
			 std::cout << " the end of file " << std::endl;
			 return void();
		 }		 		
	 }
	 while(line_count < line_stop)
	 {
         std::getline(file,line);
		 std::string buf; // Have a buffer string
		 std::stringstream ss(line); // Insert the string into a stream
		 std::vector<std::string> tokens; // Create vector to hold our words
		 while (ss >> buf)
		     tokens.push_back(buf);

         if ((line_count == line_start + 1) && (tokens.size() == 1))	
		 {
			 int timestep = std::stoi(tokens[0]);
		 }          
		 else if ((line_count == line_start +  5) && (tokens.size() == 2))
		 {
			 lboxx0 = std::stod(tokens[0]); 
			 lboxxl = std::stod(tokens[1]);
		 }
		 else if ((line_count == line_start + 6) && (tokens.size() == 2))
		 {
			 lboxy0 = std::stod(tokens[0]);
			 lboxyl = std::stod(tokens[1]);
		 }
		 else if ((line_count == line_start + 7) && (tokens.size() == 2))
		 {
			 lboxz0 = std::stod(tokens[0]);
			 lboxzl = std::stod(tokens[1]);
		 }
		 else if ((line_count >= line_start + 9) && (line_count <= line_start + 9 + n_atoms))
		 {
			 int atom_id = std::stoi(tokens[0]); 
			 positions[atom_id][0] = std::stod(tokens[1]);
			 positions[atom_id][1] = std::stod(tokens[2]);
			 positions[atom_id][2] = std::stod(tokens[3]);
			 neighbors[atom_id] = std::stoi(tokens[4]); 
		 }
         line_count++;
     }
         file.close();

}

int number(int j, int l, int k, int num_cells)
{
    return (j%num_cells)*num_cells*num_cells+(l%num_cells)*num_cells+(k%num_cells);
}

void number_to_jlk (int numb, int &j, int &l, int &k, int num_cells)
{
     k = numb%num_cells;   
	 int div = numb/num_cells;  
	 l = div%num_cells;  
	 div = div/num_cells;      
	 j = div%num_cells;
}

     int main(int argc, char* argv[]) 
{ 
	 std::cout << argc << " " << argv[1] << " " << argv[2] << " " << argv[3] << std::endl;
     std::string fin_param = "params.txt";
     std::map<std::string, std::string> params = readparams(fin_param);  // чтение входных параметров
     double still_sep = atof(params["still_sep"].c_str());
     int num_neighbors = atoi(params["num_neighbors"].c_str());
     int num_cells = atoi(params["num_cells"].c_str());

	 std::system("mkdir -p results");


	 std::string file_result_gen="Result_gen.txt";
     std::ofstream f_result_gen(file_result_gen,std::ios_base::out | std::ios_base::app);
     
     int h = std::stoi(argv[1]);
     int r = std::stoi(argv[2]);
     int c = std::stoi(argv[3]);
	 double cluster_volume0_av = 0;
	 double grow_prob = 0;
	 
		 std::string file_result="results/result_h"+ h + "_r" + r + "_c" + c;
         std::ofstream f_result(file_result,std::ios_base::out | std::ios_base::app);
		 std::cout << h << " " << r << " " << c <<  std::endl;

		 int n_atoms; 	
		 double lboxx0;
		 double lboxxl;
		 double lboxy0;
		 double lboxyl;
		 double lboxz0;
		 double lboxzl;
	
		 std::string fout_dump = "dumps_end/dump_end_h" + h + "_r" +  r + "_c" + c;
		 std::ifstream file;
		 std::string line;

		 file.open(fout_dump);
		 if(!file) 
		 {
			 std::cerr << "Error! Unable to read " << fout_dump << "\n"; 
		 }

		 int line_count = 0;
		 while (line_count <= 4)
		 {
			 std::getline(file,line);
			 std::string buf; 
			 std::stringstream ss(line); 
			 std::vector<std::string> tokens; 
			 while (ss >> buf)
				 tokens.push_back(buf);
			 if ((line_count == 3) && (tokens.size() == 1))
				 n_atoms = std::stoi(tokens[0]);
			 line_count++;
		 }	
	 	 file.close();

		 std::vector<std::vector<double>>  positions(n_atoms+1, std::vector<double>(3));
		 std::vector<int> neighbors(n_atoms+1);
         
		 int num_reading = 0;
		 std::cout << "check1" << std::endl;
		 while (num_reading <=1)
		 {
			 read_dump(n_atoms, num_reading, positions, neighbors, fout_dump, lboxx0, lboxxl, lboxy0, lboxyl, lboxz0, lboxzl);
			 std::cout << "check2" << std::endl;
			
			 enum PCLASS {LIQ, VAP};    // LIQ == 0, VAP == 1 
			 std::vector<PCLASS> parclass(n_atoms, LIQ);
   
   			 int n_VAP=0;
   
   			 for (int i=0; i < n_atoms; ++i) 
			 {
	   			 if (neighbors[i] < num_neighbors) 
	  			 {
		   			 parclass[i] = VAP;
		   			 ++n_VAP;
	   			 }
   			 }
			
  			 int cellclass[num_cells+1][num_cells+1][num_cells+1];
  			 int cellclass_numb[num_cells*num_cells*num_cells];
  			 int cellclust[num_cells+1][num_cells+1][num_cells+1];
  			 int cellclust_numb[num_cells*num_cells*num_cells];
  
  			 int ncell = 0;
  
 			 double cellposx[num_cells+1][num_cells+1][num_cells+1];
  			 double cellposy[num_cells+1][num_cells+1][num_cells+1];
 			 double cellposz[num_cells+1][num_cells+1][num_cells+1];

			 double lcellx = (lboxxl-lboxx0)/num_cells;
  			 double lcelly = (lboxyl-lboxy0)/num_cells;
  			 double lcellz = (lboxzl-lboxz0)/num_cells;

			 double lboxx = (lboxxl-lboxx0);
			 double lboxy = (lboxyl-lboxy0);
			 double lboxz = (lboxzl-lboxz0);
			
			 int numb=0;

  			 for (int j=0; j<= num_cells; ++j)
  			 {
	  			 for (int l=0; l<= num_cells; ++l)
	  			 {
		  			 for (int k=0; k<= num_cells; ++k)
		  			 {
						 cellposx[j][l][k] = lboxx0+(j%num_cells)*lcellx;  
						 cellposy[j][l][k] = lboxy0+(l%num_cells)*lcelly; 
						 cellposz[j][l][k] = lboxz0+(k%num_cells)*lcellz; 
						 cellclass[j][l][k] = 1;
						 cellclust[j][l][k] = 0;
						 numb = (j%num_cells)*num_cells*num_cells+(l%num_cells)*num_cells+(k%num_cells);
						 cellclust_numb[numb]=0;
						 cellclass_numb[numb]=1;
		  			 }
	  			 }
  			 };

			 for (int i=0; i < n_atoms; ++i)
    		 {
	   			 if (parclass[i] == LIQ) 
				 {
		   			 for (int j=0; j<=num_cells; ++j)
             		 {
				 		 for (int l=0; l< num_cells; ++l)
				 		 {
					 		 for (int k=0; k< num_cells; ++k)
		             		 {
								 double sepx,sepy,sepz;
								 sepx = positions[i][0] - cellposx[j][l][k];
								 sepy = positions[i][1] - cellposy[j][l][k];
								 sepz = positions[i][2] - cellposz[j][l][k];
     
                        		 if (sepx > 0.5 * lboxx) 
								 {
									 sepx = sepx - lboxx;
								 }
								 else if (sepx < -0.5 * lboxx) 
								 {
									 sepx = sepx + lboxx;
					    		 }
					    
					    		 if (sepy > 0.5 * lboxy) 
								 {
									 sepy = sepy - lboxy;
								 }
								 else if (sepy < -0.5 * lboxy) 
								 {
									 sepy = sepy + lboxy;
								 }
						
								 if (sepz > 0.5 * lboxz) 
								 {
									 sepz = sepz - lboxz;
								 }
								 else if (sepz < -0.5 * lboxz) 
								 {
									 sepz = sepz + lboxz;
								 }							

								 double rsq = sepx*sepx+sepy*sepy+sepz*sepz;
								 if (rsq < still_sep*still_sep) 
								 {
									 cellclass[j][l][k] = 0;  // 0=liq
									 numb = (j%num_cells)*num_cells*num_cells+(l%num_cells)*num_cells+(k%num_cells);
									 cellclass_numb[numb]=0;
								 }
							  }
						 }
					 }
				 }
			 }

			 for (int j=0; j<=num_cells; ++j)  
             {
				 for (int l=0; l< num_cells; ++l)
				 {
					 cellclass[j][l][num_cells] = cellclass[j][l][0];
					 cellclass[j][num_cells][l] = cellclass[j][0][l];
				 }
				 cellclass[j][num_cells][num_cells] = cellclass[j][0][0];
			 };
			 int clust_tmp = 0;
             std::queue<int> NeighborsIDQueue;
			 while (!NeighborsIDQueue.empty())
			 {
				 NeighborsIDQueue.pop();
			 }
			 int current_number =0;
			 std::vector<int> queue_checking((num_cells+1)*(num_cells+1)*(num_cells+1));

					 for (int cell_number = 0; cell_number < num_cells*num_cells*num_cells; ++cell_number)
		             {
						 int j;
						 int l;
						 int k;
						 number_to_jlk(cell_number,j,l,k,num_cells);
						 if ((cellclass[j][l][k] == 1) && (cellclust[j][l][k] == 0))
                         {
							 clust_tmp++;
                             NeighborsIDQueue.push(number(j,l,k,num_cells));
							 queue_checking[number(j,l,k,num_cells)] = 1;
                             while (!NeighborsIDQueue.empty())
                             {
                                 int j2;
                                 int l2;
                                 int k2;
								 current_number = NeighborsIDQueue.front();
                                 number_to_jlk(current_number,j2,l2,k2,num_cells);
                                 cellclust[j2][l2][k2] = clust_tmp;
                                 for (int j3=j2-1; j3 <= j2+1; ++j3)
                                 {
                                     for (int l3=l2-1; l3 <= l2+1; ++l3)
                                     {
                                         for (int k3=k2-1; k3 <= k2+1; ++k3)
                                         {
                                             if ((cellclass[j3][l3][k3] == 1)&&(cellclust[j3][l3][k3] == 0)&&(j3 >= 0)&&(l3 >= 0)&&(k3 >= 0))
                                             {
												 if (queue_checking[number(j3,l3,k3,num_cells)] != 1)
												 {
                                                 	 NeighborsIDQueue.push(number(j3,l3,k3,num_cells));
												 	 queue_checking[number(j3,l3,k3,num_cells)] = 1;
												 }
                                             }
                                         }
                                     }
                                 }
                                 NeighborsIDQueue.pop();
                             }
                         }
					 }

			 for (int j=0; j < num_cells; ++j)  
             {
				 for (int l=0; l< num_cells; ++l)
				 {
					 cellclass[j][l][num_cells] = cellclass[j][l][0];
					 cellclass[j][num_cells][l] = cellclass[j][0][l];
					 cellclass[num_cells][j][l] = cellclass[0][j][l];
					 cellclust[j][l][num_cells] = cellclust[j][l][0];
					 cellclust[j][num_cells][l] = cellclust[j][0][l];
					 cellclust[num_cells][j][l] = cellclass[0][j][l];
				 }
				 cellclass[j][num_cells][num_cells] = cellclass[j][0][0];
				 cellclass[num_cells][j][num_cells] = cellclass[0][j][0];
				 cellclass[num_cells][num_cells][j] = cellclass[0][0][j];
				 cellclust[j][num_cells][num_cells] = cellclust[j][0][0];
				 cellclust[num_cells][j][num_cells] = cellclust[0][j][0];
				 cellclust[num_cells][num_cells][j] = cellclust[0][0][j];
			 };
			 cellclass[num_cells][num_cells][num_cells] = cellclass[0][0][0];
			 cellclust[num_cells][num_cells][num_cells] = cellclust[0][0][0];

			 int max_clust = 0;
  			 for (int j=0; j< num_cells; ++j) 
	  		 {
		  		 for (int l=0; l< num_cells; ++l)
	             {
					 for (int k=0; k< num_cells; ++k) 
		              {
						  if (cellclust[j][l][k] > max_clust) 
						  {
							 max_clust = cellclust[j][l][k];
						  }
					  }
				  }
			 };

			 int clust_size[max_clust+2]; 	
  
  			 for (int i=0; i< max_clust+1; ++i) 
			 {
	  			 clust_size[i]=0;
    		 };
    
    
    		 for (int j=0; j< num_cells; ++j) 
	  		 {
		  		 for (int l=0; l< num_cells; ++l)
	             {
					 for (int k=0; k< num_cells; ++k) 
		             {
						 if (cellclust[j][l][k]!= 0) 
						 {
							 ++clust_size[cellclust[j][l][k]];
						 }
					 }
				 }
	   		 };

						  
			 for (int i=1; i<= max_clust; ++i) 
			 {
				 if (ncell<clust_size[i]) 
				 {
					 ncell=clust_size[i];
				 }
			 };

			 f_result << num_reading << " " << ncell << " " << ncell*lcellx*lcelly*lcellz << std::endl;

			 num_reading++;
		 } 
		 f_result.close();	
		 
 f_result_gen.close();
}

