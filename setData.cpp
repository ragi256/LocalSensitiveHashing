// setData.cpp
// ver 1.10
// 2/26 update

//「tweet.*」データを整形する
//ID、ベクトル要素、単語の3ブロックに分けて出力

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include "math.h"

using namespace std;

inline string BOD(string line);
inline void EOS(list<string> words);
inline void EOD(ofstream &log, list<string> words, string ID);
inline void ELSE(string line, list<string> &words);

double dimension;
double element;
string ID;
int flag; //BODからEODまでの間に自立語が存在するか

string hinshi[3] = {"動詞","名詞","形容詞"};

int main(int argc, char **argv)
{

  ifstream ifs(argv[1]);
  ofstream ofs(argv[2], ios::out);
  list<string> words;
  string line;
  unsigned long counter;
  
  while( ifs && getline(ifs,line)){
    if( line[0] == '*' ) continue;
    else if(line.find("BOD\t") == 0){
      ID = BOD(line);
      dimension = 0;
    }
    else if(line.find("EOS") == 0){
      EOS(words); 
    }
    else if(line.find("EOD") == 0){
      EOD(ofs, words, ID);
      words.clear();
    }
    else{
      ELSE(line, words);
    }
    ++counter;
    if(counter%1000000 == 0)
      cout << counter/1000000 << "million line finish" <<endl;
  }
  words.clear();
}

inline string BOD(string line){
  vector<string> temp;
  boost::algorithm::split(temp,line,boost::is_any_of("\t"));
  ID = temp[1];
  return ID;  
}

inline void EOS(list<string> words){
  if(flag == 1){ //自立語があるならば
    words.push_back("\n");
    words.push_back("\n");
  }
  flag = 0;
}

inline void EOD(ofstream &log, list<string> words, string ID){
  if(dimension != 0){
    log << ID << endl; 
    element = 1 / (double)sqrt(dimension);
    log << setprecision(12) << fixed << element << endl; 
    
    BOOST_FOREACH(string x, words){
      if( x.compare("\n") == 0){
	log << endl;
      }
      else log << x << ' '; 
    }
    log << endl << endl;
  }
}

inline void ELSE(string line, list<string> &words){
  vector<string> blocks;
  boost::algorithm::split(blocks,line,boost::is_any_of(",\t"));
  for(int i=0;i<3;++i){
    if(blocks[1].compare(hinshi[i]) == 0){
      flag = 1; //自立語がある
      ++dimension;
      if(blocks[7].compare("*") == 0) words.push_back(blocks[0]);
      else words.push_back(blocks[7]);
    }
  }
  blocks.clear();
}
  

