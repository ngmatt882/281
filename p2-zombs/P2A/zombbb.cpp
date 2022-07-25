// Project identifier: 9504853406CBAC39EE89AA3AD238AA12CA198043

#include <getopt.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <math.h>
#include <vector>
#include <queue>
#include <string>
#include <deque>
#include "P2random.h"

using namespace std;

struct Zombie{
    string name;
    int distance;
    int speed;
    int health;
    int rounds;  
};

class ZombieComparator{
    public:
        bool operator()(const Zombie* z1,const Zombie* z2){
            if(z1->distance/z1->speed > z2->distance/z2->speed)
                return true;
            else if(z1->distance/z1->speed == z2->distance/z2->speed && z1->health > z2->health)
                return true;
            else if(z1->distance/z1->speed == z2->distance/z2->speed && z1->health == z2->health 
                    && z1->name > z2->name)
                return true;
            else
                return false;
        }
}; // Zombie Compartor
class MostActive{
    public:
        bool operator()(const Zombie* z1, const Zombie* z2){
            if(z1->rounds < z2->rounds){
                return true;
            }
            else if(z1->rounds == z2->rounds && z1->name > z2->name){
                return true;
            }
            return false;
        }  
}; // Most Active Comparator

class LeastActive{
    public:
        bool operator()(const Zombie* z1, const Zombie* z2){
            if(z1->rounds > z2->rounds){
                return true;
            }
            else if(z1->rounds == z2->rounds && z1->name > z2->name){
                return true;
            }
            return false;
        }
}; // Least Active Comparator

class LastStand{    
    // Print help for the user when requested.
    // argv[0] is the name of the currently executing program
    public:
        LastStand(int argc, char *argv[]){
            getFormat(argc, argv);
            string comment;
            getline(cin, comment);
            int seed;
            int rDist;
            int rSpeed;
            int rHealth;
            cin >> comment >> capacity >> comment >> seed >> comment >> rDist
                >> comment >> rSpeed >> comment >> rHealth;
            P2random::initialize(seed, rDist, rSpeed, rHealth);
            eaten = false;
            killer = "";
            lastkilled = "";
            count = 0;
            medianNum = 0;  

        }
        void printHelp(char *argv[]){
            cout << "Usage: " << argv[0] << " -v | [-s num] | -m | -h" << endl;
        } // printHelp()

        void getFormat(int argc, char *argv[]){
            verbose = false;
            stat = false;
            median = false;
            string statSNum;
            // These are used with getopt_long()
            opterr = false; // Let us handle all error output for command line options
            int choice;
            int option_index = 0;
            option long_options[] = {{"verbose", no_argument, nullptr, 'v'},
                                    {"statistics", required_argument, nullptr, 's'},
                                    {"median",no_argument, nullptr, 'm'},
                                    {"help", no_argument, nullptr, 'h'},
                                    {nullptr, 0, nullptr, '\0'}
            };

            // TODO: Fill in the double quotes, to match the mode and help options.
            while ((choice = getopt_long(argc, argv, "hvms:", long_options, &option_index)) != -1){
                switch (choice){
                    case 'h':
                        printHelp(argv);
                        exit(0);

                    case 'm':
                        median = true;
                        break;
                    case 'v':
                        verbose = true;
                        break;
                    case 's':
                        stat = true;
                        statSNum = optarg;
                        statNum = stoi(statSNum);
                        if(statNum < 0){
                            // The first line of error output has to be a 'fixed' message for the autograder
                            // to show it to you.
                            cerr << "Error: invalid number" << endl;
                            // The second line can provide more information, but you won't see it on the AG.
                            cerr << "  I don't know recognize: " << stat << endl;
                            exit(1);
                        } // if
                        break;

                    default:
                        cerr << "Error: invalid option" << endl;
                        exit(1);
                } // switch
            }     // while
        } // getFormat()

        void startRound(){
            curRound = 0;
            loadRound1();
            while(!eaten && (!field.empty() || curRound + 1 <= nextRound)){
                ++curRound;
                if(verbose){
                    cout << "Round: " << curRound << "\n";
                }
                updateList();
                if(eaten){
                    break;
                }
                quiver = capacity;
                string comment;
                int num;
                if(curRound == nextRound){
                    cin >> comment >> num;
                    loadRZombie(num);
                    cin >> comment >> num;
                    createNZombie(num);
                    if(!cin.fail()){
                        cin >> comment >> comment >> nextRound;
                    }
                }

                kill();
                if(median && medianNum != 0){
                    cout << "At the end of round " << curRound << ", the median zombie lifetime is "
                        << (int)medianNum << "\n";
                }
            }
        } // startRound()

        void result(){
            if(eaten){
                cout << "DEFEAT IN ROUND " << curRound << "! "
                    << killer << " ate your brains!\n";
            }
            else{
                cout << "VICTORY IN ROUND " << curRound << "! "
                    << lastkilled << " was the last zombie.\n";
            }

            if(stat){
                statistics();
            }
        }

        void statistics(){
            cout << "Zombies still active: " << field.size() << "\n";
            cout << "First zombies killed:\n";
            int nkilled = (statNum < (int)killed.size()) ? statNum : (int)killed.size();
            for(int i = 0; i < nkilled; ++i){
                cout << killed[i] << " " << i+1 << "\n";
            }
            cout << "Last zombies killed:\n";
            for(int i = 1; i <= nkilled; ++i){
                cout << killed[killed.size()-i] << " " << nkilled + 1 - i << "\n";
            }

            int active = (statNum < (int)zList.size()) ? statNum : (int)zList.size();
            cout << "Most active zombies:\n";
            priority_queue<Zombie*, vector<Zombie*>, MostActive> ma(zList.begin(), zList.end());
            for(int x = 0; x < active; ++x){
                cout << ma.top()->name << " " << ma.top()->rounds << "\n";
                ma.pop();
            }
            cout << "Least active zombies:\n";
            priority_queue<Zombie*, vector<Zombie*>, LeastActive> la(zList.begin(), zList.end());
            for(int x = 0; x < active; ++x){
                cout << la.top()->name << " " << la.top()->rounds << "\n";
                la.pop();
            }

        } // statistics()   

        void loadRound1(){
            quiver = capacity;
            string comment;
            int num;
            cin >> comment >> comment >> num; //round: 1

            curRound = num;

            if(verbose){
                for(int x = 1; x <= curRound; ++x)
                    cout << "Round: " << x << "\n"; 
            }
            cin >> comment >> num; //rz
            for(int x = 0; x < num; ++x){
                ++count;
                Zombie* nz = createRZombie();
                field.push(nz);
                zList.push_back(nz);
            }

            cin >> comment >> num;
            createNZombie(num);
            if(!cin.fail()){
                cin >> comment >> comment >> nextRound;
            }
            kill();
            if(median && medianNum != 0){
                    cout << "At the end of round " << curRound << ", the median zombie lifetime is "
                        << (int)medianNum << "\n";
                }
        } // loadRound1()

        void kill(){
            while(quiver != 0 && !field.empty()){
                Zombie* zptr = field.top();
                if(zptr->health <= quiver){
                    quiver = quiver - zptr->health;
                    zptr->health = 0;
                    field.pop();
                    lastkilled = zptr->name;
                    if(verbose){
                        cout << "Destroyed: " << zptr->name << " (distance: " << zptr->distance
                            << ", speed: " << zptr->speed << ", health: " << zptr->health << ")\n";
                    }
                    if(median){
                        inMedian(zptr);
                    }
                    if(stat){
                        killed.push_back(zptr->name);
                    }
                }
                else{
                    zptr->health = zptr->health - quiver;
                    quiver = 0;
                }
            }
        } // kill()

        void inMedian(const Zombie* z){
            if(smaller.empty()){
                smaller.push(z->rounds);
                medianNum = z->rounds;
            }
            else if(smaller.size() > bigger.size()){
                if(z->rounds < medianNum){
                    bigger.push(smaller.top());
                    smaller.pop();
                    smaller.push(z->rounds);
                }
                else{
                    bigger.push(z->rounds);
                }
                medianNum = (smaller.top()+bigger.top())/2;
            }
            else if(smaller.size() == bigger.size()){
                if(z->rounds < medianNum){
                    smaller.push(z->rounds);
                    medianNum = smaller.top();
                }
                else{
                    bigger.push(z->rounds);
                    medianNum = bigger.top();
                }
            }
            else{
                if(z->rounds > medianNum){
                    smaller.push(bigger.top());
                    bigger.pop();
                    bigger.push(z->rounds);
                }
                else{
                    smaller.push(z->rounds);
                }
                medianNum = (smaller.top() + bigger.top())/2;
            }
        } // inMedian()

        Zombie* createRZombie(){
            string n = P2random::getNextZombieName();
            int d = P2random::getNextZombieDistance();
            int s = P2random::getNextZombieSpeed();
            int h = P2random::getNextZombieHealth();
            if(verbose){
                cout << "Created: " << n << " (distance: " << d << ", speed: " << s
                    << ", health: " << h << ")\n";
            }
            Zombie* z = new Zombie{n,d,s,h,1};
            return z;
        } // createRZombie()

        void loadRZombie(int num){
            for(int x = 0; x < num; ++x){
                ++count;
                Zombie* nz = createRZombie();
                field.push(nz);
                zList.push_back(nz);
            }
        } // loadRZombie()

        void createNZombie(int num){
            string comment;
            for(int i = 0; i < num; ++i){
                string n;
                int d;
                int s;
                int h;
                cin >> n >> comment >> d >> comment >> s >> comment >> h;
                Zombie* nz = new Zombie{n,d,s,h,1};
                if(verbose){
                    cout << "Created: " << n << " (distance: " << d << ", speed: " << s
                        << ", health: " << h << ")\n";
                }
                ++count;
                zList.push_back(nz);
                field.push(nz);
            }
        } // createNZombie()

        void updateList(){
            for(size_t x = 0; x < zList.size(); ++x){
                if(zList[x]->health > 0){
                    zList[x]->distance = max(0, zList[x]->distance - zList[x]->speed);
                    ++(zList[x]->rounds);
                    if(verbose){
                        cout << "Moved: " << zList[x]->name << " (distance: " << zList[x]->distance
                            << ", speed: " << zList[x]->speed << ", health: " << zList[x]->health << ")\n";
                    }
                    if(zList[x]->distance == 0 && killer == ""){
                        eaten = true;
                        killer = zList[x]->name;
                    }
                }
            }
        } // updateList()

        void cleanup(){
            while(!field.empty()){
                field.pop();
            }
            for(int x = 0; x < count; ++x){
                Zombie* z = zList.back();
                zList.pop_back();
                delete z;
            }
        }
    private:
        bool median;
        bool verbose;
        bool stat;
        int statNum;
        int capacity;
        int quiver;
        bool eaten;
        int count;
        int curRound;
        int nextRound;
        string killer;
        string lastkilled;
        vector<Zombie*> zList;
        priority_queue<Zombie*, vector<Zombie*>, ZombieComparator> field;
        //median priority queues
        priority_queue<int> smaller;
        priority_queue<int, vector<int>, std::greater<int> > bigger;
        int medianNum;
        //statistics
        deque<string> killed;

}; //LastStand Class
/*
class Zombie{
    public:
        string getName(){
            return name;
        }
        int getDistance(){
            return distance;
        }
        int getSpeed(){
            return speed;
        }
        int getHealth(){
            return health;
        }

        void changeDistance(int d){
            distance = d;
        }
        void changeHealth(int h){
            health = h;
        }
    private:
        string name;
        int health;
        int distance;
        int speed;
}; // Zombie Class
*/

int main(int argc, char *argv[]){
    //speed up I/O
    ios_base::sync_with_stdio(false);

    LastStand ZombSim(argc, argv);
    string comment;
    getline(cin, comment);
    ZombSim.startRound();
    ZombSim.result();

    ZombSim.cleanup();
    return 0;
} // main()


//dont shoot one at a time in loop
//create range based constructor of pq for least and msot active
//      do after results
//string &getName for zombies class to reduce copying
//
//Binary constructor
/*
data.resize();
data.inserst(data.end(), start, end);
updatePrio
*/