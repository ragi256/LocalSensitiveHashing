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
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "time.h"

using namespace std;
using namespace boost;

enum line_flags {ID_line,element_line,words_line,EOD};
enum flag {MatchIDlist,IsTarget,ELSE};
typedef unordered_map<string,double> str_dbl_map;

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

int main(int argc, char **argv)
{
  time_t start_time,end_time;
  time(&start_time);
  
  ifstream ifs1(argv[1]); //ID.txt
  ifstream ifs2; //log.txt
  string line,target,ID,rubbish;
  vector<string> ID_list,words,t_words;
  double element;
  const double threshold_value = 0.6;
  vector<Set> extract_set;
  Set target_set;
  line_flags checker = ID_line;
  flag search_flag = ELSE;

  getline(ifs1,rubbish);
  long long int times = atoi(rubbish.c_str()); //ID.txt 先頭の比較ID数を取得
  
  cout << "ID1 \t\t\tID2\t\t\t近似度" <<endl;
  cout << "===============================================================" << endl;

  for(int i=0; i<times; ++i){ //比較ループ開始
    
    getline(ifs1,target);
    getline(ifs1,rubbish);// target直後にある空行
    while(ifs1 && getline(ifs1,line) && !line.empty()){
      ID_list.push_back(line);
    }
    
    ifs2.open(argv[2]);
    while(ifs2 && getline(ifs2,line)){
      if(checker == ID_line){ // ID読み取り
	ID = line;
	if(ID == target)
	   search_flag = IsTarget;
	else if( find(ID_list.begin(),ID_list.end(),ID) != ID_list.end() )
	  search_flag = MatchIDlist;
	else{ // 関係ないツイート 
	  getline(ifs2,rubbish); // element行
	  getline(ifs2,rubbish); // 単語行
	  getline(ifs2,rubbish); // 改行
	  continue;
	}
	checker = element_line;
      }
      else if(checker == element_line){ //element読み取り
	element = atof(line.c_str());
	checker = words_line;
      }
      else if(checker == words_line){ //単語読み取り
	algorithm::split(words,line,is_space(),token_compress_on); 
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
    ifs2.close();
    const int ID_counter = extract_set.size();
    
    for(vector<Set>::iterator tweet=extract_set.begin(); tweet!=extract_set.end(); ++tweet){
      for(str_dbl_map::iterator it=target_set.VECTOR.begin(); it!=target_set.VECTOR.end(); ++it){
	string key = it->first;
	const str_dbl_map::iterator itv = tweet->VECTOR.find(key);
	if(itv != tweet->VECTOR.end())
	  tweet->distance += it->second * itv->second;
      }
    }
    sort(extract_set.begin(), extract_set.end(), GreaterDistance);

    const int RENGE = min(ID_counter,30);
    for(int i=0; i<RENGE; ++i){
      if(extract_set[i].distance >= threshold_value){
	cout << target << "\t" << extract_set[i].ID << "\t";
	cout << setprecision(12) << fixed << extract_set[i].distance << endl;
      }
    }
    ID_list.clear();
    extract_set.clear();
    t_words.clear();
  } //比較ループ終了
  ID_list.clear();
  words.clear();
  t_words.clear();
  extract_set.clear();

  time(&end_time);
  cerr << "所要時間:" << difftime(end_time, start_time) << "秒" << endl;
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
