// searchID.cpp
// ver 4.11
// 4/10 update

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
inline void readData(ifstream &input);
inline string convertHex(string &hash);
inline void makeList(mt19937 &r_engine, uniform_int<> &dst, v_ID_hash *shuffle_list, v_ID_hash *sort_list);
void filterSycle(v_ID_hash *shuffle_list,v_ID_hash *sort_list, ofstream &output);
inline void shuffle(mt19937 &r_engine, uniform_int<> &dst);
inline void extractID(v_ID_hash &sort_list,const int &ti, set<LL> &nearlist);

struct hash_less{
  bool operator()(const p_ID_hash &x, const p_ID_hash &y)const{
    return x.second < y.second;};
};

int TIME,NEARNESS,LIMIT;
LL LIST_SIZE;
v_ID_hash datalist;

int main(int argc, char **argv){
  ifstream input(argv[1]); //check.txt
  ofstream output(argv[2], ios::out); //ID.txt
  time_t start_time,end_time;
  
  cerr << "評価回数と検索対象との近似幅を入力してください" << endl;
  cin >> TIME; cin >> NEARNESS;
  v_ID_hash shuffle_list[TIME],sort_list[TIME];

  time(&start_time);

  cerr << "ID,ストリーム読み込み処理開始" <<endl;
  readData(input);
  cerr << "ID,ストリーム読み込み処理終了" << endl;

  LIMIT = datalist[0].second.size(); // シャッフルする際スワップする箇所は
  mt19937 r_engine;
  uniform_int<> dst(1, LIMIT); // ハッシュの桁数を上限とするランダムで決める
  LIST_SIZE = datalist.size();
  output << LIST_SIZE << endl;
  cerr << "全ツイート数: " << LIST_SIZE << endl;
  
  cerr << "リスト生成処理開始" << endl;
  makeList(r_engine,dst,shuffle_list,sort_list);
  cerr << "リスト生成処理終了" << endl;
  
  cerr << "近似ツイート抽出処理開始" << endl;
  filterSycle(shuffle_list,sort_list,output);
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

inline void readData(ifstream &input){
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

inline void makeList(mt19937 &r_engine, uniform_int<> &dst,
		     v_ID_hash *shuffle_list, v_ID_hash *sort_list){
  for(int i=0; i<TIME; ++i){
    shuffle(r_engine, dst);
    shuffle_list[i].assign(datalist.begin(),datalist.end());
    sort_list[i].assign(datalist.begin(), datalist.end());
    sort(sort_list[i].begin(),sort_list[i].end(),hash_less());
    cerr << "\t" << i+1 << "/" << TIME << "times finish" << endl;
  }
}

inline void shuffle(mt19937 &r_engine, uniform_int<> &dst){
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

void filterSycle(v_ID_hash *shuffle_list,v_ID_hash *sort_list, ofstream &output){
  set<LL> near_list;
  int tenth = LIST_SIZE/10, counter = 0;
  for(int i=0; i<LIST_SIZE; ++i){
      for(int j=0; j<TIME; ++j){
	  v_ID_hash::iterator target = 
	      lower_bound(sort_list[j].begin(), sort_list[j].end(), shuffle_list[j][i]);
	  extractID(sort_list[j], target-sort_list[j].begin(), near_list);
      }
      output << shuffle_list[0][i].first << "\n\n";
      copy(near_list.begin(), near_list.end(), ostream_iterator<LL>(output,"\n"));
      output << "\n";
      near_list.clear();

      counter++;
      if(counter%tenth==0)
	  cerr << counter/tenth << "0% tweet" << endl;
  }
}

<<<<<<< HEAD

inline void extractID(v_ID_hash &sort_list, const int &ti,set<LL> &nearlist){
=======
inline void extractID(v_ID_hash &datalist, const int &ti,set<LL> &nearlist){
>>>>>>> add CompareTestProgram(ver.spread) and minor change
  int index;
  for(int point=0; point<NEARNESS; ++point){
    index = ti - (NEARNESS - point);
    if( index < 0 ); // pass
    else nearlist.insert(sort_list[index].first);
    index = ti + (NEARNESS - point);
    if( (LIST_SIZE-1) < index ); //pass
    else nearlist.insert(sort_list[index].first);
  }
}
