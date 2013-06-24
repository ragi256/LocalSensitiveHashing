// searchID.cpp
// ver 3.12
// 4/22 update

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
enum match_flag {MATCH_SIMILARITY, ELSE};
typedef boost::unordered_map<string,double> str_dbl_map;
typedef pair<string,vector<string> > IDset;
typedef boost::unordered_map<string,vector<string> > IDsetMap;

const double THRESHOLD_VALUE = 0.6;


class Tweet{//:public std::binary_function <const Tweet &,const Tweet &,bool>{
public:
  string ID;
  Tweet() {}
  Tweet(const string& _id) : ID(_id) {}

  str_dbl_map words_vector;
  inline void getID(string ID);
  void getWordVector(double element,vector<string> words);
  static bool LessID(const Tweet& rLeft, const Tweet& rRight) { return rLeft.ID < rRight.ID; }
  //  bool operator<(const Tweet& tweet){ return tweet.ID; }
  bool operator<(const Tweet &x){
    return (this->ID < x.ID); }
};


bool myfunction(Tweet tweet,string str){return (tweet.ID < str); }

typedef boost::unordered_map<string, pair<vector<string>,Tweet> > TARGET;
//typedef boost::unordered_map<string,Tweet> SIMILAR; //  <<<<<<<<<<<  memory size over
typedef vector<Tweet> SIMILAR;
inline void getLists(TARGET &target_tweets,SIMILAR &similar_tweets,ifstream &inputID);
void getTweet(TARGET &target_tweets,SIMILAR &similar_tweets,ifstream &inputLog);
void calculate(TARGET &target_tweets,SIMILAR &similar_tweets);



int main(int argc, char **argv){
  time_t start_time,end_time;
  time(&start_time);

  string line;
  ifstream inputID(argv[1]); // ID.txt
  getline(inputID, line);
  const long long int targets_size = atoi(line.c_str()); 
  TARGET target_tweets(targets_size);
  SIMILAR similarity_tweets(targets_size);

  std::clog << targets_size << " tweetあります。" << std::endl;
  const long long int CONSTANT = targets_size/10;
  std::cerr << "ID取得開始"<< std::endl;

  for(int i=0; i<targets_size; ++i){
    if(i%CONSTANT==0){
      std::clog << "\r" << i/CONSTANT << "0% tweet finish";
    }
    getLists(target_tweets,similarity_tweets,inputID);
  }
  std::clog << std::endl;
  inputID.close();
  std::cerr << "ID取得終了" << std::endl;

  sort(similarity_tweets.begin(),similarity_tweets.end(),Tweet::LessID);
  
  std::cerr <<  "ツイート取得開始" << std::endl;
  ifstream inputLog(argv[2]); // log.txt
  getTweet(target_tweets,similarity_tweets,inputLog);
  inputLog.close();
  std::cerr << "ツイート取得終了" << std::endl;
  
  std::cerr << "ID1 \t\t\tID2\t\t\t近似度" << std::endl;
  std::cerr << "===============================================================" << std::endl;
  calculate(target_tweets,similarity_tweets);
  std::cerr << "===============================================================" << std::endl;
  
  time(&end_time);
  std::cerr  << "所要時間:" << difftime(end_time, start_time) << "秒" << std::endl;
  
}

inline void getLists(TARGET &target_tweets,SIMILAR &similar_tweets,ifstream &inputID){
  string line,target_ID;
  getline(inputID,target_ID);
  getline(inputID,line);
  while(inputID && getline(inputID,line) && !line.empty()){
    target_tweets[target_ID].first.push_back(line);
    //    similar_tweets[line].ID = line;
    Tweet temp; temp.getID(line);
    similar_tweets.push_back(temp);
  }
  vector<string> &target_similars = target_tweets[target_ID].first;
  sort(target_similars.begin(),target_similars.end());
}

void getTweet(TARGET &target_tweets,SIMILAR &similar_tweets,ifstream &inputLog){
  using namespace boost;
  string line,ID,rubbish;
  line_flags checker = ID_LINE;
  match_flag search_flag = ELSE;
  double element;
  vector<string> words;
  
  while(inputLog && getline(inputLog,line)){
    switch(checker){
      
    case ID_LINE:{
      ID = line;
      //Tweet temp; temp.ID = line;
      std::pair<SIMILAR::iterator,SIMILAR::iterator> it_pair;
      it_pair= std::equal_range(similar_tweets.begin(),similar_tweets.end(),Tweet(ID));
      const int s_count = (int)distance(it_pair.first,it_pair.second);
      if(s_count)
	search_flag = MATCH_SIMILARITY;
      checker = ELEMENT_LINE;}
      break;
      
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
      target_tweets[ID].second.getWordVector(element,words);
      if(search_flag == MATCH_SIMILARITY)
	similar_tweets.at(it_pair.first).getWordVector(element,words);
      words.clear();
      search_flag = ELSE;
      checker = ID_LINE;}
      break;
      
    default:{ // std::cerr << "何かがおかしいです" << std::endl;
    }
      break;
    } //end_switch
  } //end_while
}

void calculate(TARGET &target_tweets,SIMILAR &similar_tweets){
  for(TARGET::iterator target=target_tweets.begin(),end_target=target_tweets.end();target!=end_target;++target){
    vector<string> &temp_similar = target->second.first;
    Tweet &t_tweet = target->second.second;
    for(vector<string>::iterator similar=temp_similar.begin(),end_similar=temp_similar.end();similar!=end_similar;++similar){
      std::pair<SIMILAR::iterator,SIMILAR::iterator> it_pair;
      it_pair = std::equal_range(similar_tweets.begin(),similar_tweets.end(),*similar);
      Tweet &s_tweet = *it_pair.first;
      double distance=0;
      for(str_dbl_map::iterator it=t_tweet.words_vector.begin(),end_it=t_tweet.words_vector.end();it!=end_it;++it){
	const string key = it->first;
	if(s_tweet.words_vector.count(key))
	  distance += t_tweet.words_vector[key] * s_tweet.words_vector[key];
      }
      if(THRESHOLD_VALUE <= distance){
	std::cout << target->first << "\t" << *similar << "\t";
	std::cout << std::setprecision(12) << std::fixed << distance << std::endl;
      }
    }
  }
}

inline void Tweet::getID(string IDstring){ 
  this->ID = IDstring; 
}

void Tweet::getWordVector(double element,vector<string> words){
  for(vector<string>::iterator it=words.begin(),it_end=words.end(); it!=it_end; ++it){
    if(this->words_vector.count(*it)){
      double &value = this->words_vector[*it];
      value = sqrt(value * value + element * element);
    }
    else this->words_vector[*it] = element;
  }
}
