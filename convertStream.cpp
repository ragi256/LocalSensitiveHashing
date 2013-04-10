// convertStream.cpp
// ver 2.31
// 4/10 update

// 整形されたデータからビットストリームを作成
// ID, streamの2ブロックに分けて出力

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "stdlib.h"
#include "time.h"

using namespace std;
using namespace boost;

typedef unordered_map<string, double> word_vector;

enum line_flags {ID_line, element_line, words_line, EOD};

inline void addResult(int n, double element, double *result, vector< double* > r_vector);

int main(int argc, char **argv)
{
  cout << "何bitのストリームを作りますか？" << endl;
  int K;
  cin >> K;
  
  time_t start_time,end_time;
  time(&start_time);
  
  ifstream ifs(argv[1]); //log.txt
  ofstream ofs(argv[2], ios::out); //check.txt
  string line,ID;
  double element;
  line_flags checker = ID_line;
  vector<string> words;
  double result[K];
  fill_n(result,sizeof(result)/sizeof*(result),0);
  uniform_real<> dst(-0.5, 0.5);
  mt19937 r_engine;
  word_vector r_vector[K];
  unsigned long long int counter=0;
  
  cout<< "ツイート読み込み開始" << endl;
  while( ifs && getline(ifs,line)){
    if(checker == ID_line){ // ID読み込み
      ID = line;
      words.clear();
      checker = element_line;
    }
    else if(checker == element_line){ // element読み込み
      element = atof(line.c_str());
      checker = words_line;
    }
    else if(checker == words_line){ // words読み込み
      algorithm::split(words,line,is_any_of(" "));      
      checker = EOD;
    }
    else if(checker == EOD){ // 文章終末処理
      for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it){
	bool not_exist_flag = ( r_vector[0].find(*it) == r_vector[0].end() );
	for(int i=0; i<K; ++i){
	  if(not_exist_flag) r_vector[i][*it] = dst(r_engine);
	  result[i] += element * r_vector[i][*it];
	} 
      }
      
      ofs << ID << "\t";      
      for(int i=0; i<K; ++i){
	int res;
	if(result[i] >= 0) res = 1;	
	else if(result[i] < 0) res = 0;
	result[i] = 0;
	ofs << res;
      }
      ofs << endl;
      checker = ID_line;
    }
    ++counter;
    if(counter % 100000 == 0)
      cout << counter << " line finish" << endl;
  }
  cout << "ビットストリーム変換終了" << endl;
  words.clear();
  for(int i=0; i<K; ++i) r_vector[i].clear();
  time(&end_time);
  cerr << "所要時間:" << difftime(end_time, start_time) << "秒" << endl;
}


