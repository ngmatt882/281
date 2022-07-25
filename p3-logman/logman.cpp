//Project Identifier: 01BD41C3BF016AD7E8B6F837DF18926EC3E83350

#include <map>
#include <unordered_map>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h> // needed for strcasecmp()
#include <cctype> // needed for isalnum()
#include <algorithm>
#include <vector>
#include <deque>

using namespace std;

void printHelp(char* argv[]){
    cout << "Usage:" << argv[0] << "[filename] | -h" << endl;
}

struct Entries{
    int logID;
    string ts; //later needs to be converted
    string cat;
    string msg;
    uint64_t tsi;
};
struct TimeSort{
    bool operator()(const Entries &a, const Entries &b){
        if(a.tsi < b.tsi){
            return true;
        }
        else if(a.tsi == b.tsi && strcasecmp((a.cat).c_str(), (b.cat).c_str()) < 0){
            return true;
        }
        else if(a.tsi == b.tsi && strcasecmp((a.cat).c_str(), (b.cat).c_str()) == 0 && a.logID < b.logID){
            return true;
        }
        return false;
    }
}; // TimeSort Functor

struct TimeComp{
    bool operator()(const Entries &a, const Entries &b){
        if(a.tsi < b.tsi)
            return true;
        return false;
    }
}; // TimeComp Functor

class Logman{
    public:
        Logman(string file){
            ifstream fin(file);
            if(!fin.is_open()){
                cout << "Invalid file" << endl;
                exit(0);
            }
            count = 0;
            string pts, cat, msg;
            while(getline(fin, pts, '|')){
                getline(fin,cat, '|');
                getline(fin, msg);
                string ts = pts;
                pts.erase(remove(pts.begin(), pts.end(), ':'), pts.end());
                uint64_t tsi = stol(pts); 
                Entries ne = {count, ts, cat, msg, tsi};
                masterlist.push_back(ne);
                ++count;
            }
            
            hasSearched = false;
            cout << count << " entries read\n";
        } // Logman()
        
        void setUp(){
            sort(masterlist.begin(), masterlist.end(), TimeSort());
            masterIndex.resize(count);
            for(size_t i = 0; i < masterlist.size(); ++i){
                masterIndex[masterlist[i].logID] = (int)i;
                string lcat = masterlist[i].cat;
                makeLower(lcat);
                clist[lcat].push_back((int)i);
                setUpCK(lcat, (int)i);
                setUpK(masterlist[i].msg, (int)i);
            }


        } // setUp()

        void setUpK(const string& s, int i){
            bool hasBegin = false;
            int b = -1;

            for(size_t x = 0; x < s.length(); ++x){
                if(!hasBegin && isalnum(s[x])){
                    hasBegin = true;
                    b = (int)x;
                }
                else if(!isalnum(s[x]) && hasBegin){
                    string sub = s.substr(b, x-b);
                    makeLower(sub);
                    hasBegin = false;
                    auto it = klist.find(sub);
                    if(it == klist.end()){
                        klist[sub].push_back(i);
                    }
                    else if(it->second.back() != i){
                        it->second.push_back(i);
                    }
                }
            }
            if(hasBegin){
                string sub = s.substr(b);
                makeLower(sub);
                auto it = klist.find(sub);
                if(it == klist.end()){
                    klist[sub].push_back(i);
                }
                else if(it->second.back() != i){
                    it->second.push_back(i);
                }
            }
        } // setUpK()

        void setUpCK(const string& s, int i){
            bool hasBegin = false;
            int b = -1;

            for(size_t x = 0; x < s.length(); ++x){
                if(!hasBegin && isalnum(s[x])){
                    hasBegin = true;
                    b = (int)x;
                }
                else if(!isalnum(s[x]) && hasBegin){
                    string sub = s.substr(b, x-b);
                    hasBegin = false;
                    auto it = klist.find(sub);
                    if(it == klist.end()){
                        klist[sub].push_back(i);
                    }
                    else if(it->second.back() != i){
                        it->second.push_back(i);
                    }
                }
            }
            if(hasBegin){
                string sub = s.substr(b);
                auto it = klist.find(sub);
                if(it == klist.end()){
                    klist[sub].push_back(i);
                }
                else if(it->second.back() != i){
                    it->second.push_back(i);
                }
            }
        } // setUpCK()

        void makeLower(string& s){
            std::for_each(s.begin(), s.end(), [](char &c){
                c = (char)::tolower(c);
            });
        } // makeLower()

        void readCommands(){
            char cmd;
            do{
                cout << "% ";
                cin >> cmd;

                if(cmd == '#'){
                    string comment;
                    getline(cin, comment);
                }
                else if(cmd == 't'){
                    tCMD();
                }
                else if(cmd == 'm'){
                    mCMD();
                }
                else if(cmd == 'c'){
                    cCMD();
                }
                else if(cmd == 'k'){
                    kCMD();
                }
                else if(cmd == 'a'){
                    aCMD();
                }
                else if(cmd == 'r'){
                    rCMD();
                }
                else if(cmd == 'd'){
                    dCMD();
                }
                else if(cmd == 'b'){
                    bCMD();
                }
                else if(cmd == 'e'){
                    eCMD();
                }
                else if(cmd == 's'){
                    sCMD();
                }
                else if(cmd == 'l'){
                    lCMD();
                }
                else if(cmd == 'g'){
                    gCMD();
                }
                else if(cmd == 'p'){
                    pCMD();
                }
                else{
                    cerr << "Invalid command" << endl;
                }
            } while(cmd != 'q');


        } // readCommands()
        
        //finds entries from two timestamps
        void tCMD(){
            string searchts;
            getline(cin, searchts);
            searchts = searchts.substr(1);
            if(searchts.length() != 29 && searchts[14] != '|'){
                cerr << "Invalid timestamp(s)" << endl;
            }
            else{
                searchts.erase(remove(searchts.begin(), searchts.end(), ':'), searchts.end());
                string ts1 = searchts.substr(0, 10);
                string ts2 = searchts.substr(11, 10);
                uint64_t tsi1 = stol(ts1);
                uint64_t tsi2 = stol(ts2);

                Entries tss1, tss2;
                tss1.tsi = tsi1;
                tss2.tsi = tsi2;
                auto b = lower_bound(masterlist.begin(), masterlist.end(), tss1, TimeComp());
                auto e = upper_bound(masterlist.begin(), masterlist.end(), tss2, TimeComp());
                cout << "Timestamps search: " << (e-b) << " entries found\n";

                hasSearched = true;
                searched.clear();
                for(auto x = b-masterlist.begin(); x < e-masterlist.begin(); ++x){
                    searched.push_back((int)x);
                }
            }
        } // tCMD()

        //finds matching timestamps
        void mCMD(){
            string searchts;
            getline(cin, searchts);
            searchts = searchts.substr(1);
            if(searchts.length() != 14){
                cerr << "Invalid timestamp" << endl;
            }
            else{
                searchts.erase(remove(searchts.begin(), searchts.end(), ':'), searchts.end());
                uint64_t searchtsi = stol(searchts);

                Entries tss;
                tss.tsi = searchtsi;
                auto b = lower_bound(masterlist.begin(), masterlist.end(), tss, TimeComp());
                auto e = upper_bound(masterlist.begin(), masterlist.end(), tss, TimeComp());
                cout << "Timestamp search: " << (e-b) << " entries found\n";

                hasSearched = true;
                searched.clear();
                for(auto x = b-masterlist.begin(); x < e-masterlist.begin(); ++x){
                    searched.push_back((int)x);
                }
            }
        } // mCMD()

        //category search
        void cCMD(){
            string searchCat;
            getline(cin, searchCat);
            searchCat = searchCat.substr(1);
            makeLower(searchCat);
            auto it = clist.find(searchCat);
            if(it == clist.end()){
                //swap might be faster
                searched.clear();
                cout << "Category search: 0 entries found\n";
            }
            else{
                cout << "Category search: " << it->second.size() << " entries found\n";
                searched = it->second;
            }

            hasSearched = true;
        } // cCMD()

        void kCMD(){
            string keywords;
            getline(cin, keywords);
            keywords = keywords.substr(1);
            vector<string> keys;
            parseKeywords(keywords, keys);
            bool hasFirst = false;
            vector<int> ans;
            for(size_t x = 0; x < keys.size(); ++x){
                auto itf = klist.find(keys[x]);
                if(itf == klist.end()){
                    ans.clear();
                    break;
                }
                if(!hasFirst){
                    ans = itf->second;
                    hasFirst = true;
                }
                else{
                    vector<int> &comp = itf->second;
                    vector<int> temp;
                    if(comp.size() < ans.size()){
                        temp.resize(comp.size());
                    }
                    else{
                        temp.resize(ans.size());
                    }
                    auto it = set_intersection(ans.begin(), ans.end(), comp.begin(), comp.end(), temp.begin());
                    temp.resize(it-temp.begin());
                    swap(temp, ans);
                }
            }

            cout << "Keyword search: " << ans.size() << " entries found\n";

            searched.clear();
            if(!ans.empty())
                searched = ans;

            hasSearched = true;
        } // kCMD()

        void parseKeywords(const string& s, vector<string> &v){
            bool hasBegin = false;
            int b = -1;

            for(size_t x = 0; x < s.length(); ++x){
                if(!hasBegin && isalnum(s[x])){
                    hasBegin = true;
                    b = (int)x;
                }
                else if(!isalnum(s[x]) && hasBegin){
                    string sub = s.substr(b, x-b);
                    makeLower(sub);
                    hasBegin = false;
                    v.push_back(sub);
                }
            }
            if(hasBegin){
                string sub = s.substr(b);
                makeLower(sub);
                v.push_back(sub);
            }
        } // parseKeywords()

        //appends index from masterlist to back of excerpt list
        void aCMD(){
            int index;
            cin >> index;
            if (index < 0 || index >= count){
                cerr << "Invalid Index" << endl;
            }
            else{
                excerptlist.push_back(masterIndex[index]);
                cout << "log entry " << index << " appended\n";
            }
        } // aCMD()

        // appends from search list
        void rCMD(){
            if(!hasSearched){
                cerr << "Previous search required" << endl;
            }
            else{
                sort(searched.begin(), searched.end());
                for(size_t i = 0; i < searched.size(); ++i){
                    excerptlist.push_back(searched[i]);
                }
                cout << searched.size() << " log entries appended\n";
            }
        } // rCMD()

        void dCMD(){
            int index;
            cin >> index;
            if(index < 0 || index >= (int)excerptlist.size()){
                cerr << "Invalid Index" << endl;
            }
            else{
                excerptlist.erase(excerptlist.begin()+index);
                cout << "Deleted excerpt list entry " << index << "\n";
            }
        } // dCMD()

        void bCMD(){
            int index;
            cin >> index;
            if(index < 0 || index >= (int)excerptlist.size()){
                cerr << "Invalid Index" << endl;
            }
            else{
                cout << "Moved excerpt list entry " << index << "\n";
                int en = excerptlist[index];
                excerptlist.erase(excerptlist.begin()+index);
                excerptlist.push_front(en);
            }
        } // bCMD()

        //moves entry to end of excerpt
        void eCMD(){
            int index;
            cin >> index;
            if(index < 0 || index >= (int)excerptlist.size()){
                cerr << "Invalid Index" << endl;
            }
            else{
                cout << "Moved excerpt list entry " << index << "\n";
                int en = excerptlist[index];
                excerptlist.erase(excerptlist.begin()+index);
                excerptlist.push_back(en);
            }
        } // eCMD()

        //sorts excerpt list
        void sCMD(){
            if(excerptlist.empty()){
                cout << "excerpt list sorted\n(previously empty)\n";
            }
            else{
                cout << "excerpt list sorted\nprevious ordering:\n";
                Entries &first = masterlist[excerptlist.front()];
                Entries &last = masterlist[excerptlist.back()];
                cout << "0|";
                printEntry(first);
                cout << "\n...\n" << excerptlist.size()-1 << "|";
                printEntry(last);
                cout << "\nnew ordering:\n";
                std::sort(excerptlist.begin(), excerptlist.end());
                Entries &nfirst = masterlist[excerptlist.front()];
                Entries &nlast = masterlist[excerptlist.back()];
                cout << "0|";
                printEntry(nfirst);
                cout << "\n...\n" << excerptlist.size()-1 << "|";
                printEntry(nlast);
                cout << "\n";
            }
            
        } // sCMD

        //clears excerpt list
        void lCMD(){
            cout << "excerpt list cleared\n";
            if(excerptlist.empty()){
                cout << "(previously empty)\n";
            }
            else{
                cout << "previous contents:\n";
                Entries &first = masterlist[excerptlist.front()];
                Entries &last = masterlist[excerptlist.back()];
                cout << "0|";
                printEntry(first);
                cout << "\n...\n" << excerptlist.size()-1 << "|";
                printEntry(last);
                cout << "\n";
                excerptlist.clear();
            }
        } // lCMD()
        //prints excerpt list
        void pCMD(){
            for(size_t i = 0; i < excerptlist.size(); ++i){
                int index = excerptlist[i];
                Entries &en = masterlist[index];
                cout << i << "|" << en.logID << "|" << en.ts << "|" << en.cat << "|" << en.msg << "\n";
            }
        } // pCMD()

        //prints search list
        void gCMD(){
            // TODO: make sure search cmd has been called before
            if(searched.empty()){
                cerr << "Previous search required" << endl;
            }
            else{
                for(size_t i = 0; i < searched.size(); ++i){
                    int index = searched[i];
                    Entries &en = masterlist[index];
                    cout << en.logID << "|" << en.ts << "|" << en.cat << "|" << en.msg << "\n";
                }
            }
        } // gCMD()

        void printEntry(const Entries& en){
            cout << en.logID << "|" << en.ts << "|" << en.cat << "|" << en.msg;
        }// printEntry()
    // public members
    
    private:
        vector<Entries> masterlist;
        vector<int> masterIndex;
        int count;
        unordered_map<string, vector<int>> clist;
        unordered_map<string, vector<int>> klist;
        deque<int> excerptlist;
        vector<int> searched;
        bool hasSearched;
    // private members
}; // Logman Class

int main(int argc, char *argv[]){
    //speed up I/O
    ios_base::sync_with_stdio(false);
    string args = argv[1];
    if(args == "--help" || args == "-h" || argc > 2){
        printHelp(argv);
        exit(0);
    }
    string filename = args;

    Logman logging(filename);
    logging.setUp();
    logging.readCommands();

    return 0;
} // main()