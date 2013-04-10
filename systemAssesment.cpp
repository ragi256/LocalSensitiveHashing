// systemAssesment.cpp
// ver 1.10
// 2/26 updata

// LSHプログラムのRecall(再現率)とPrecision(正解率)を算出します
// 算出した値を元に評価値、F-measureを算出します
// 比較対象プログラムの検出したペア集合をright
// LSHプログラムの検出したペア集合をsystemとします

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

typedef vector<string> v_str;

int main(int argc, char **argv)
{
  ifstream cmp_res(argv[1]); //result_compare.txt
  ifstream lsh_res(argv[2]); //result_lsh.txt
  string line;
  vector<string> IDpair,right,system;
  long r_size,s_size,rs_size=0; //rs_sizeはrightとsystemが一致した数
  double recall,precision,f_measure;
  
  cerr << "\nRecall/Precision/F値を計算します" << endl;
  while( cmp_res && getline(cmp_res, line)){
    algorithm::split(IDpair,line,is_any_of("\t"));
    right.push_back(IDpair[0]+IDpair[1]);
  }
  while( lsh_res && getline(lsh_res, line)){
    algorithm::split(IDpair,line,is_any_of("\t"));
    system.push_back(IDpair[0]+IDpair[1]);
  }
  r_size = right.size();
  s_size = system.size();
  for(v_str::iterator target=system.begin(); target!=system.end(); ++target){
    v_str::iterator pos=find(right.begin(),right.end(),*target);
    if(pos!=right.end()) ++rs_size;
  }  
  IDpair.clear();
  right.clear();
  system.clear();

  recall = rs_size/(double)r_size;
  precision = rs_size/(double)s_size;
  f_measure = (recall*precision) / (double)((recall+precision)/2);

  cout << "right  " << r_size << endl;
  cout << "system " << s_size << endl;
  cout << "r and s" << rs_size << endl;

  cout << "+++++++++++++++++++++++++++++++++" << endl;
  cout << "  Reacall:" << setprecision(12) << fixed << recall << endl;
  cout << "Precision:" << setprecision(12) << fixed << precision << endl;
  cout << "F-measure:" << setprecision(12) << fixed << f_measure << endl;
    
}

