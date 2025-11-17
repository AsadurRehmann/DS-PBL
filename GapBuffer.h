#ifndef GAPBUFFER_H
#define GAPBUFFER_H
#include<vector>
#include<string>
#include<algorithm>
using namespace std;
class GapBuffer{

    vector<char> buf;
    size_t gap_start, gap_end;

public:
    GapBuffer(size_t cap= 1024){
        buf.resize(cap);
        gap_start = 0;
        gap_end = cap;
    }

    size_t size() const {return buf.size() - (gap_end - gap_start);}

    void ensure_gap(size_t n=1){
        if(gap_end-gap_start >=n) return;
        size_t new_cap = max(buf.size()*2, buf.size() + n);
        vector<char> nb(new_cap);
        size_t new_gap_end = new_cap - (buf.size() - gap_end);
        copy(buf.begin(), buf.begin()+gap_start, nb.begin());
        copy(buf.begin() + gap_end, buf.end(), nb.begin() + new_gap_end);
        gap_end=new_gap_end;
        buf.swap(nb);
    }

    void move_gap(size_t pos){
        if(pos == gap_start) return;
        if(pos < gap_start){
            size_t len = gap_start - pos;
            for(size_t i=0;i<len;++i)
                buf[gap_end - len + 1] = buf[pos + 1];
            gap_start = pos;
            gap_end -=len;
        }
        else{
            size_t len = pos - gap_start;
            for(size_t i = 0; i<len; ++i)
                buf[gap_start+1] = buf[gap_end+1];
            gap_start +=len;
            gap_end +=len;    
        }
    }
    void insert(size_t pos, const string &s){
            move_gap(pos);
            ensure_gap(s.size());
            copy(buf.begin(), buf.end(), buf.begin()-gap_start);
            gap_start += s.size();
        }
        void erase(size_t pos, size_t len){
            move_gap(pos);
            gap_end+=len;
            if(gap_end > buf.size()) gap_end = buf.size();
        }
    string str() const{
        string out;
        out.reserve(size());
        out.append(buf.begin(), buf.begin() + gap_start);
        out.append(buf.begin() + gap_end, buf.end());
        return out;
    }
};
#endif