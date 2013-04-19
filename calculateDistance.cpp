// searchID.cpp
// ver 2.31
// 4/10 update

// searchIDにて作成したIDリストから検索IDとのコサイン距離を計算
// 検索対象が固定 一対多

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "time.h"

using std::string;
using std::vector;
using std::ifstream;

enum line_flags {ID_line,element_line,words_line,EOD};
enum flag {MatchIDlist,IsTarget,ELSE};
typedef boost::unordered_map<std::string,double> str_dbl_map;
const double threshold_value = 0.6;

class tweet{
private:
  std::string ID;
  str_dbl_map words_vector;
  double distance;
  tweet(std::string str,str_dbl_map sdmap, const double value);
  ~tweet();
};

tweet::tweet(string str,str_dbl_map sdmap, const double value){
  ID = str;
  words_vector = sdmap;
  distance = value;
};

  
struct Set{ //ID,element,単語のセット
  string ID;
  str_dbl_map VECTOR;
  double distance; //コサイン距離
  Set(string a=""){
    ID = a;
    distance = 0;
  }
};

bool GreaterDistance(const Set Left, const Set Right){
  return Left.distance > Right.distance;}

inline Set setTweet(string &ID, double &element,vector<string> &words);
inline void setIDList(ifstream &inputID,string &target,vector<string> &ID_list);
void getTweetLog(ifstream &inputLog, vector<string> &ID_list, string &target);

vector<string> ID_list,t_words,words;
Set target_set;
vector<Set> extract_set;

int main(int argc, char **argv)
{
  using namespace boost;


  time_t start_time,end_time;
  time(&start_time);
  
  string target,ID,rubbish;



  std::cerr << "ID取得開始" << std::endl;
  ifstream inputID(argv[1]); //ID.txt    
  getline(inputID,rubbish);
  long long int times = atoi(rubbish.c_str()); //ID.txt 先頭の比較ID数を取得
  for(int i=0; i<times; ++i)
    setIDList(inputID,target,ID_list);
  std::cerr << "ID取得終了" << std::endl;
  
  std::cerr << "ツイート取得開始" << std::endl;
  ifstream inputLog(argv[2]); //log.txt
  for(int i=0; i<times; ++i)//比較ループ開始
    getTweetLog(inputLog, ID_list,target);
  const int ID_counter = extract_set.size();
  inputLog.close();
  std::cerr << "ツイート取得終了" << std::endl;
  
  std::cerr << "ID1 \t\t\tID2\t\t\t近似度" << std::endl;
  std::cerr << "===============================================================" << std::endl;
  
  for(int i=0; i<times; ++i){ //比較ループ開始    
    for(vector<Set>::iterator tweet=extract_set.begin(); tweet!=extract_set.end(); ++tweet){
      for(str_dbl_map::iterator it=target_set.VECTOR.begin(); it!=target_set.VECTOR.end(); ++it){
	string key = it->first;
	const str_dbl_map::iterator itv = tweet->VECTOR.find(key);
	if(itv != tweet->VECTOR.end())
	  tweet->distance += it->second * itv->second;
      }
    }
    sort(extract_set.begin(), extract_set.end(), GreaterDistance);

    const int RENGE = std::min(ID_counter,30);
    for(int i=0; i<RENGE; ++i){
      if(extract_set[i].distance >= threshold_value){
	std::cerr << target << "\t" << extract_set[i].ID << "\t";
	std::cerr << std::setprecision(12) << std::fixed << extract_set[i].distance << std::endl;
      }
    }
    extract_set.clear();
    t_words.clear();
  } //比較ループ終了 
  std::cerr << "===============================================================" << std::endl;
 
  ID_list.clear();
  words.clear();
  t_words.clear();
  extract_set.clear();

  time(&end_time);
  std::cerr << "所要時間:" << difftime(end_time, start_time) << "秒" << std::endl;
}
  
inline Set setTweet(string &ID, double &element,vector<string> &words){
  Set temp(ID);
  for(vector<string>::iterator it=words.begin(); it!=words.end(); ++it){
    bool exist_flag =(temp.VECTOR.find(*it) != temp.VECTOR.end());
    if(exist_flag){
      double &tmp_elmnt = temp.VECTOR[*it];
      tmp_elmnt = sqrt(tmp_elmnt * tmp_elmnt + element * element);
    }
    else temp.VECTOR[*it] = element;
  }
  return temp;
}

inline void setIDList(ifstream &inputID, string &target, vector<string> &ID_list){
  string rubbish,line;
  getline(inputID,target);
  getline(inputID,rubbish);// target直後にある空行
  while(inputID && getline(inputID,line) && !line.empty())
    ID_list.push_back(line);
}

void getTweetLog(ifstream &inputLog, vector<string> &ID_list, string &target){
  string line,ID,rubbish;
  line_flags checker = ID_line;
  flag search_flag = ELSE;
  double element;

  while(inputLog && getline(inputLog,line)){
    if(checker == ID_line){ // ID読み取り
      ID = line;
      if(ID == target)
	search_flag = IsTarget;
      else if( find(ID_list.begin(),ID_list.end(),ID) != ID_list.end() )
	search_flag = MatchIDlist;
      else{ // 関係ないツイート 
	getline(inputLog,rubbish); // element行
	getline(inputLog,rubbish); // 単語行
	getline(inputLog,rubbish); // 改行
	continue;
      }
      checker = element_line;
    }
    else if(checker == element_line){ //element読み取り
      element = atof(line.c_str());
      checker = words_line;
    }
    else if(checker == words_line){ //単語読み取り
      boost::algorithm::split(words,line,boost::is_space(),boost::token_compress_on); 
      words.pop_back(); //logの単語末尾に半角スペースがあるため
      checker = EOD;
    }
    else if(checker == EOD){ //終端処理
      if(search_flag == MatchIDlist){ //IDリスト内のID
	Set temp = setTweet(ID,element,words);
	extract_set.push_back(temp);
      }
      else if(search_flag == IsTarget){ //targetと同じID
	target_set = setTweet(ID,element,words);
	t_words = words;
      }
      words.clear();
      checker = ID_line;
      search_flag = ELSE;
    }
  }
  
}
