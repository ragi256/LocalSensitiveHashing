// searchID.cpp
// ver 3.10
// 4/19 update

// searchIDにて作成したIDリストから検索IDとのコサイン距離を計算
// 検索対象が固定 一対多

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "time.h"

using std::string;
using std::vector;
using std::pair;
using std::ifstream;

enum line_flags {ID_LINE, ELEMENT_LINE, WORDS_LINE, EOD};
enum match_flag {MATCH_SIMILARITY, IS_TARGET, ELSE};
typedef boost::unordered_map<string,double> str_dbl_map;
typedef pair<string,vector<string> > IDset;
typedef boost::unordered_map<string,vector<string> > IDsetMap;

const double THRESHOLD_VALUE = 0.6;

class tweet{
public:
  string ID;
  str_dbl_map words_vector;
  double distance;
  void getData(string ID, double element,vector<string> words);
};

typedef boost::unordered_map<string, pair<vector<string>,tweet> > TARGET;
typedef boost::unordered_map<string,tweet> SIMILAR;

inline void getLists(IDsetMap &target_set,vector<string> &similarity_list,ifstream &inputID);
void getTweet(IDsetMap target_set_list,vector<string> similarity_list,ifstream &inputLog,
	      vector<tweet> target_tweets,vector<tweet> similarity_tweets);
void calculate(vector<tweet> target_tweets,vector<tweet> similarity_tweets,IDset &target_set);

bool LessID(const tweet Left, const tweet Right){
  return Left.ID < Right.ID;}
  
int main(int argc, char **argv){
  time_t start_time,end_time;
  time(&start_time);

  string target, ID, rubbish;
  ifstream inputID(argv[1]); // ID.txt
  getline(inputID, rubbish);
  const long long int targets_size = atoi(rubbish.c_str()); 
  //  vector<IDset> target_set_list(targets_size);
  IDsetMap target_set_list(targets_size);
  vector<string> similarity_list(targets_size);

  std::cerr << "ID取得開始"<< std::endl;
  for(int i=0; i<targets_size; ++i)
    //    getLists(target_set_list[i],similarity_list,inputID);
    getLists(target_set_list,similarity_list,inputID);
  inputID.close();
  std::cerr << "ID取得終了" << std::endl;
  
  //  sort(target_set_list.begin(),target_set_list.end());
  std::cerr << "ID sorting now " << std::endl;
  sort(similarity_list.begin(),similarity_list.end());
  std::cerr << std::flush << "sorting finish" << std::endl;
  
  vector<tweet> target_tweets(targets_size),similarity_tweets(targets_size);

  std::cerr << std::flush <<  "ツイート取得開始" << std::endl;
  ifstream inputLog(argv[2]);
  getTweet(target_set_list,similarity_list,inputLog,target_tweets,similarity_tweets);
  inputLog.close();
  std::cerr << "ツイート取得終了" << std::endl;
  similarity_list.clear();

  std::cerr << "ID sorting now " << std::endl;
  sort(target_tweets.begin(),target_tweets.end(),LessID);
  sort(similarity_tweets.begin(),similarity_tweets.end(),LessID);
  std::cerr << std::flush << "sorting finish" << std::endl;
  
  std::cerr << "ID1 \t\t\tID2\t\t\t近似度" << std::endl;
  std::cerr << "===============================================================" << std::endl;
  IDset temp;
  for(IDsetMap::iterator it=target_set_list.begin(),end_it=target_set_list.end();
      it!=end_it;++it){
    temp = *it;
    calculate(target_tweets,similarity_tweets,temp);
  }
  std::cerr << "===============================================================" << std::endl;
  target_set_list.clear();
  
  time(&end_time);
  std::cerr  << "所要時間:" << difftime(end_time, start_time) << "秒" << std::endl;
  
}

// inline void getLists(IDset &target_set,vector<string> &similarity_list,ifstream &inputID){
//   string line;
//   getline(inputID,line);
//   target_set.first = line;
//   getline(inputID,line);
//   while(inputID && getline(inputID,line) && !line.empty()){
//     target_set.second.push_back(line);
//     similarity_list.push_back(line);
//   }
//   sort(target_set.second.begin(),target_set.second.end());
// }
inline void getLists(IDsetMap &target_set,vector<string> &similarity_list,ifstream &inputID){
  string line,ID;
  getline(inputID,ID);
  getline(inputID,line);
  while(inputID && getline(inputID,line) && !line.empty()){
    target_set[ID].push_back(line);
    similarity_list.push_back(line);
  }
  sort(target_set[ID].begin(),target_set[ID].end());
}

// void getTweet(vector<IDset> target_set_list,vector<string> similarity_list,ifstream &inputLog,
// 	      tweet *target_tweets,tweet *similarity_tweets){
//   using namespace boost;
//   string line,ID,rubbish;
//   line_flags checker = ID_LINE;
//   match_flag search_flag = ELSE;
//   double element;
//   const int THREE=3;
//   int target_pointer=0,similar_pointer=0;
//   vector<std::string> words;
  
//   while(inputLog && getline(inputLog,line)){
//     switch(checker){

//     case ID_LINE:{
//       ID = line;
//       if(binary_search(target_set_list.begin(),target_set_list.end(),line,FirstLess))
// 	search_flag = IS_TARGET;
//       else if(binary_search(similarity_list.begin(),similarity_list.end(),line))
// 	search_flag = MATCH_SIMILARITY;
//       else{
// 	for(int i=0; i<THREE; ++i)
// 	  getline(inputLog,rubbish);
//       checker = ELEMENT_LINE;}
//       break;
//     }
//     case ELEMENT_LINE:{
//       element = atof(line.c_str());
//       checker = WORDS_LINE;}
//       break;
      
//     case WORDS_LINE:{
//       algorithm::split(words,line,is_space(),token_compress_on);
//       words.pop_back(); //logの単語末尾に半角スペースがあるため
//       checker = EOD;}
//       break;
      
//     case EOD:{
//       if(search_flag == IS_TARGET){
// 	target_tweets[target_pointer].getData(ID,element,words);
// 	++target_pointer;
//       }
//       else if(search_flag == MATCH_SIMILARITY){
// 	similarity_tweets[similar_pointer].getData(ID,element,words);
// 	++similar_pointer;
//       }
//       checker = ID_LINE;}
//       break;
      
//     default:{
//       std::cerr << "何かがおかしいです" << std::endl;}
//       break;
//     }
//   }
//   words.clear();
// }

void getTweet(IDsetMap target_set_list,vector<string> similarity_list,ifstream &inputLog,
	      vector<tweet> target_tweets,vector<tweet> similarity_tweets){
  using namespace boost;
  string line,ID,rubbish;
  line_flags checker = ID_LINE;
  match_flag search_flag = ELSE;
  double element;
  const int THREE=3;
  int target_pointer=0,similar_pointer=0;
  vector<std::string> words;
  
  while(inputLog && getline(inputLog,line)){
    switch(checker){

    case ID_LINE:{
      ID = line;
      if(target_set_list.count(ID))
	search_flag = IS_TARGET;
      else if(binary_search(similarity_list.begin(),similarity_list.end(),ID))
	search_flag = MATCH_SIMILARITY;
      else{
	for(int i=0; i<THREE; ++i)
	  getline(inputLog,rubbish);
      checker = ELEMENT_LINE;}
      break;
    }
    case ELEMENT_LINE:{
      element = atof(line.c_str());
      checker = WORDS_LINE;}
      break;
      
    case WORDS_LINE:{
      algorithm::split(words,line,is_space(),token_compress_on);
      words.pop_back(); //logの単語末尾に半角スペースがあるため
      checker = EOD;}
      break;
      
    case EOD:{
      if(search_flag == IS_TARGET){
	target_tweets[target_pointer].getData(ID,element,words);
	++target_pointer;
      }
      else if(search_flag == MATCH_SIMILARITY){
	similarity_tweets[similar_pointer].getData(ID,element,words);
	++similar_pointer;
      }
      checker = ID_LINE;}
      break;
      
    default:{
      std::cerr << "何かがおかしいです" << std::endl;}
      break;
    }
  }
  words.clear();
}
struct LessID{
  bool operator ()(const tweet &Left, const tweet &Right){
    return Left.ID < Right.ID;}
};

void calculate(vector<tweet> target_tweets,vector<tweet> similarity_tweets,IDset &target_set){
  for(vector<string>::iterator it=target_set.second.begin(),end_it=target_set.second.end();
      it!=end_it; ++it)
    std::upper_bound(target_tweets.begin(),target_tweets.end(),target_set.first,LessID);
  std::find_if(target_tweets.begin(),target_tweets.end(),target_set.first,LessID);
}

void tweet::getData(string ID, double element,vector<string> words){
  this->ID = ID;
  for(vector<string>::iterator it=words.begin(),it_end=words.end(); it!=it_end; ++it){
    if(this->words_vector.count(*it)){
      double &value = this->words_vector[*it];
      value = sqrt(value * value + element * element);
    }
    else this->words_vector[*it] = element;
  }
}
