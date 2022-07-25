// Project Identifier: 3E33912F8BAA7542FC4A1585D2DB6FE0312725B9

#include <getopt.h>
#include <vector>
#include <string>
#include <deque>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>


using namespace std;

struct Prim{
    bool inPath = false;
    double dist = numeric_limits<double>::infinity();
    int parent;
};

struct Cage{
    int cageNum;
    int xCoord;
    int yCoord;
    bool tame;
    bool wild;
};

class ZooKeeper{
    public:
        void printHelp(char *argv[]){
            cout << "Usage: " << argv[0] << "|-h" << endl;
        }// printHelp()

        void getFormat(int argc, char *argv[]){
            bool modeSpecified = false;

            // These are used with getopt_long()
            opterr = false; // Let us handle all error output for command line options
            int choice;
            int option_index = 0;
            option long_options[] = {{"mode", required_argument, nullptr, 'm'},
                                    {"help", no_argument, nullptr, 'h'},
                                    {nullptr, 0, nullptr, '\0'}
            };

        // TODO: Fill in the double quotes, to match the mode and help options.
            while ((choice = getopt_long(argc, argv, "hm:", long_options, &option_index)) != -1){
                switch (choice){
                case 'h':
                    printHelp(argv);
                    exit(0);

                case 'm':
                    mode = optarg;
                    if(mode != "MST" && mode != "OPTTSP" && mode != "FASTTSP"){
                        cerr << "Invalid mode" << endl;
                        exit(1);
                    }
                    modeSpecified = true;
                    break;

                default:
                    cerr << "Error: invalid option" << endl;
                    exit(1);
                } // switch
            } // while

            if(!modeSpecified){
                cerr << "Mode needs to be specified" << endl;
                exit(1);
            }
        } // getFormat()

        void getZoo(){
            int size;
            cin >> size;

            zoo.reserve(size);

            int x;
            int y;
            count = 0;
            bool w;
            bool s;

            while(cin >> x >> y){
                if(x < 0 && y < 0){
                    w = true;
                    s = false;
                }
                else if(x > 0 || y > 0){
                    w = false;
                    s = true;
                }
                else{
                    w = true;
                    s = true;
                }

                Cage nc = {count, x, y, s, w};
                zoo.push_back(nc);
                ++count;
            }
        }// getZoo()

        void chooseMode(){
            if(mode == "MST"){
                partA();
            }
            else if(mode == "FASTTSP"){
                partB(true);
            }
            else{ // OPTTSP
                partC();
            }
        }// chooseMode()

        double getDist(const Cage &a, const Cage &b){
            double x1x2 = (b.xCoord + 0.0)-a.xCoord;
            double y1y2 = (b.yCoord + 0.0)-a.yCoord;
            return x1x2*x1x2+y1y2*y1y2;
        }// getDist()

        // Prim's Algorithm
        // Do not use priority queues
        void partA(){
            mst = 0;
            vector<Prim> prim;
            prim.resize(zoo.size());
            int cagesIn = 0;

            prim[0].dist = 0;

            for(auto a = 0; a < count; ++a){
                int index = -1;
                double mindist = numeric_limits<double>::infinity();

                for(auto b = 0; b < count; ++b){
                    if(!prim[b].inPath && prim[b].dist < mindist){
                        mindist = prim[b].dist;
                        index = b;
                    }
                }
                if(index < 0){
                    cerr << "Cannot construct MST" << endl;
                    exit(1);
                }
                prim[index].inPath = true;
                mst = mst + sqrt(prim[index].dist);
                ++cagesIn;

                for(auto c = 0; c < count; ++c){
                    if(!prim[c].inPath && 
                        (zoo[index].tame == zoo[c].tame || zoo[index].wild == zoo[c].wild)){

                        double newDist = getDist(zoo[c], zoo[index]);
                        if(prim[c].dist > newDist){
                            prim[c].dist = newDist;
                            prim[c].parent = index;
                        }
                    }
                }
            }

            if(cagesIn != count){
                cerr << "Cannot construct MST" << endl;
                exit(1);
            }

            //print results

            cout << mst << "\n";
            for(auto i = 1; i < count; ++i){
                if(i < prim[i].parent){
                    cout << i << " " << prim[i].parent << "\n";
                }
                else{
                    cout << prim[i].parent << " " << i << "\n";
                }
            }

        } // partA()

        //Insertion Huerestics
        void partB(bool isPartB){
            vector<double> tsp;
            tsp.resize(count);
            bestPath.push_back(0);
            bestPath.push_back(1); 
            bestPath.push_back(0);// {0,1,0}

            tsp[1] = sqrt(getDist(zoo[0], zoo[1]));
            tsp[0] = tsp[1];
            bestLen = 2 * tsp[1];

            for(auto k = 2; k < count; ++k){
                double minDist = numeric_limits<double>::infinity();
                //have to initialize for some reason
                double minDist1 = numeric_limits<double>::infinity();
                double minDist2 = numeric_limits<double>::infinity();
                size_t index = -1;
                for(size_t j = 0; j < bestPath.size()-1; ++j){
                    double distIK = sqrt(getDist(zoo[bestPath[j]], zoo[k]));
                    double distKJ = sqrt(getDist(zoo[bestPath[j+1]], zoo[k]));
                    double addDist = distKJ + distIK - tsp[bestPath[j+1]];
                    if(addDist < minDist){
                        minDist = addDist;
                        minDist1 = distIK; 
                        minDist2 = distKJ;
                        index = j+1;
                    }
                }
                tsp[k] = minDist1;
                bestLen += minDist;
                tsp[bestPath[index]] = minDist2;
                bestPath.insert(bestPath.begin()+index, k);
            }

            if(isPartB){
                cout << bestLen << "\n";
                for(auto i = 0; i < count; ++i){
                    cout << bestPath[i] << " ";
                }
                cout << "\n";
            }

        }// partB()
        
        //Calculate Part B
        //Use Part A for promising, limit at 4 
        void partC(){
            partB(false);
            bestPath.pop_back();
            potPath = bestPath;
            reverse(potPath.begin() + 1, potPath.end()); //dont go down the same path
            potLen = 0;
            genPerms(1);

            cout << bestLen << "\n";
            for(auto i = 0; i < count; ++i){
                cout << bestPath[i] << " ";
            }
            cout << "\n";
        }// part C()

        void genPerms(size_t permLength){
            if(permLength == potPath.size()){
                //Do something with the path
                double closeEdge = sqrt(getDist(zoo[0], zoo[potPath.back()]));
                potLen += closeEdge;
                if(potLen < bestLen){
                    bestLen = potLen;
                    bestPath = potPath;
                }
                potLen -= closeEdge;
                return;
            }
            if(!promising(permLength)){
                return;
            }
            for(size_t i = permLength; i < potPath.size(); ++i){
                swap(potPath[permLength], potPath[i]);
                double newEdge = sqrt(getDist(zoo[potPath[permLength]], zoo[potPath[permLength-1]]));
                //curCost += cost of new edge
                potLen += newEdge;
                genPerms(permLength + 1);
                //curCose -= cost of that same edge
                potLen -= newEdge;
                swap(potPath[permLength], potPath[i]);
            }
        }// genPerms()

        bool promising(size_t permLength){
            //At this point calculate everything anyways, do not go through MST
            if(count - permLength <= 4){
                return true;
            }
            double branch1 = numeric_limits<double>::infinity();
            double branch2 = numeric_limits<double>::infinity();
            for(auto b = permLength; b < potPath.size(); ++b){
                double closing1 = getDist(zoo[potPath[b]], zoo[potPath[permLength-1]]);
                double closing2 = getDist(zoo[0], zoo[potPath[b]]);
                if(closing1 < branch1){
                    branch1 = closing1;
                }
                if(closing2 < branch2){
                    branch2 = closing2;
                }
            }
            double branches = sqrt(branch1) + sqrt(branch2);
            double estimate = underestimate(permLength);
            if(potLen + branches + estimate >= bestLen){
                return false;
            }
            return true;
        }// promising()

        //MST calculation
        double underestimate(size_t permLength){
            double newMST = 0;
            vector<Prim> potMST;
            potMST.resize(count);

            potMST[permLength].dist = 0;

            for(size_t a = permLength; a < potPath.size(); ++a){
                size_t index = -1;
                double mindist = numeric_limits<double>::infinity();

                for(size_t b = permLength; b < potPath.size(); ++b){
                    if(!potMST[b].inPath && potMST[b].dist < mindist){
                        mindist = potMST[b].dist;
                        index = b;
                    }
                }

                potMST[index].inPath = true;
                newMST += sqrt(potMST[index].dist);

                for(size_t c = permLength; c < potPath.size(); ++c){
                    if(!potMST[c].inPath){
                        double newDist = getDist(zoo[potPath[c]], zoo[potPath[index]]);
                        if(potMST[c].dist > newDist){
                            potMST[c].dist = newDist;
                            potMST[c].parent = (int)index;
                        }
                    }
                }
            }

            return newMST;
        } //underestimate()

    // public members
    private:
        string mode;
        vector<Cage> zoo;
        vector<int> path;
        double mst;
        int count;

        vector<int> bestPath;
        double bestLen;
        vector<int> potPath;
        double potLen;

    // private members
}; // Zoo Class

int main(int argc, char *argv[]){
    ios_base::sync_with_stdio(false);
    cout << std::setprecision(2);
    cout << std::fixed; 
    ZooKeeper zookeeper;

    zookeeper.getFormat(argc, argv);
    zookeeper.getZoo();
    zookeeper.chooseMode();

    return 0;
}// main()

