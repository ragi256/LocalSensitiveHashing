// searchID.cpp
// ver 4.10
// 3/19 update

// ビットストリーム列を複数回シャッフル→ソート
// これを繰り返して全ID対全IDの近似度を調べ近似IDの組を抽出

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <utility>
#include <boost/random.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/unordered_map.hpp>

#include "time.h"

using namespace std;
using namespace boost;

typedef long long LL;
typedef pair<LL,string> p_ID_hash;
typedef vector<p_ID_hash> v_ID_hash;
typedef unordered_map<LL,set<LL> > NearMap;

inline void stop(void);
inline void readData(ifstream &input, v_ID_hash &datalist);
inline string convertHex(string &hash);
inline void makeList(v_ID_hash &datalist, mt19937 &r_engine, uniform_int<> &dst, v_ID_hash *near_list);
void filterSycle(v_ID_hash &datalist, v_ID_hash *near_list, ofstream &output);
inline void shuffle(v_ID_hash &datalist, mt19937 &r_engine, uniform_int<> &dst);
inline void extractID(v_ID_hash &datalist, const int &ti, set<LL> &nearlist);

struct hash_less{
  bool operator()(const p_ID_hash &x, const p_ID_hash &y)const{
    return x.second < y.second;};
};


int TIME,NEARNESS,LIMIT;
LL LIST_SIZE;

int main(int argc, char **argv){
  ifstream input(argv[1]); //check.txt
  ofstream output(argv[2], ios::out); //ID.txt
  v_ID_hash datalist;
  time_t start_time,end_time;
  
  cerr << "評価回数と検索対象との近似幅を入力してください" << endl;
  cin >> TIME; cin >> NEARNESS;
  v_ID_hash sort_list[TIME];

  time(&start_time);

  cerr << "ID,ストリーム読み込み処理開始" <<endl;
  readData(input,datalist);
  cerr << "ID,ストリーム読み込み処理終了" << endl;

  LIMIT = datalist[0].second.size(); // シャッフルする際スワップする箇所は
  mt19937 r_engine;
  uniform_int<> dst(1, LIMIT); // ハッシュの桁数を上限とするランダムで決める
  LIST_SIZE = datalist.size();
  output << LIST_SIZE << endl;
  cerr << "全ツイート数: " << LIST_SIZE << endl;
  
  cerr << "リスト生成処理開始" << endl;
  makeList(datalist,r_engine,dst,sort_list);
  cerr << "リスト生成処理終了" << endl;
  
  cerr << "近似ツイート抽出処理開始" << endl;
  filterSycle(datalist,sort_list,output);
  cerr << "近似ツイート抽出処理終了" << endl;

  
  input.close();
  output.close();
  datalist.clear();

  time(&end_time);
  cerr << "所要時間:" << difftime(end_time, start_time) << "秒" <<endl;
}

inline void stop(void){
  cout << "入力IDが見つかりません。" << endl;
  cout << "プログラムを終了します。" << endl;
  exit(1);
}

inline void readData(ifstream &input, v_ID_hash &datalist){
  string bits,line;
  vector<string> ID_and_hash;
  long long ll_ID;

  while(input && getline(input, line)){
    algorithm::split(ID_and_hash,line,is_any_of("\t"));
    string &str_ID = ID_and_hash[0];
    string &hash = ID_and_hash[1];
    ll_ID = atoll(str_ID.c_str());
    bits = convertHex(hash);
    p_ID_hash temp = make_pair(ll_ID,bits);
    datalist.push_back(temp);
  }
}

inline string convertHex(string &hash){
  string bits="",four_bits,hex_str;
  short int size,cash;  ostringstream oss;
  size = hash.size();
  for(int i=0; i<size/4; ++i){
    four_bits = hash.substr(4*i,4);
    cash=0x00;
    for(int j=0; j<4; ++j){
      if(four_bits[j]=='1') cash = cash<<1 | 0x01;
      else if(four_bits[j]=='0') cash = cash<<1;
    }
    oss << hex << cash;
    hex_str = oss.str();
    bits += hex_str;
    four_bits.clear();
    cash=0x00;
  }
  return bits;
}

inline void makeList(v_ID_hash &datalist, mt19937 &r_engine, uniform_int<> &dst, v_ID_hash *sort_list){
  for(int i=0; i<TIME; ++i){
    shuffle(datalist, r_engine, dst);
    sort(datalist.begin(),datalist.end(),hash_less());
    sort_list[i].assign(datalist.begin(), datalist.end());
    cerr << "\t" << i+1 << "/" << TIME << "times finish" << endl;
  }
}

inline void shuffle(v_ID_hash &datalist, mt19937 &r_engine, uniform_int<> &dst){
  int shuffle_times = dst(r_engine)/2; 
  int key1,key2;
  for(int j=0; j<shuffle_times; ++j){
    key1 = dst(r_engine);
    key2 = dst(r_engine);
    for(int k=0; k < LIST_SIZE; ++k){
      string &temp=datalist[k].second;
      swap(temp[key1],temp[key2]);
    }
  }
}

void filterSycle(v_ID_hash &datalist, v_ID_hash *sort_list, ofstream &output){
  set<LL> near_list;
  int tenth = LIST_SIZE/10, counter = 0;
  for(int i=0; i<LIST_SIZE; ++i){
    LL &target = datalist[i].first;
    for(int j=0; j<TIME; ++j){
      for(int k=0; k<LIST_SIZE; ++k){
	p_ID_hash &temp = sort_list[j].at(k);
	if(temp.first==target){
	  extractID(sort_list[j], k, near_list);
	  break;
	}
      }
    }
    output << target << "\n\n";
    copy(near_list.begin(),near_list.end(),ostream_iterator<LL>(output,"\n"));
    output << "\n";
    near_list.clear();

    counter++;
    if(counter%tenth==0)
      cerr << counter/tenth << "0% tweet" << endl;
  }
}


inline void extractID(v_ID_hash &datalist, const int &ti,set<LL> &nearlist){
  int index;
  for(int point=0; point<NEARNESS; ++point){
    index = ti - (NEARNESS - point);
    if( index < 0 ); // pass
    else nearlist.insert(datalist[index].first);
    index = ti + (NEARNESS - point);
    if( (LIST_SIZE-1) < index ); //pass
    else nearlist.insert(datalist[index].first);
  }
}
