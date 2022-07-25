// 950181F63D0A883F183EC0A5CC67B19928FE896A

#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <deque>

using namespace std;

class Navigation{
public:
    // Print help for the user when requested.
    // argv[0] is the name of the currently executing program
    void printHelp(char *argv[]){
        cout << "Usage: " << argv[0] << " [-m resize|reserve|nosize]|-h" << endl;
        cout << "This program is to help you learn command-line processing, " << endl;
        cout << "reading data into a vector, the difference between resize and reserve," << endl;
        cout << "and how to properly read until end-of-file." << endl;
    } // printHelp()

    void getFormat(int argc, char *argv[]){
        bool modeSpecified = false;
        bool formatSpecified = false;

        // These are used with getopt_long()
        opterr = false; // Let us handle all error output for command line options
        int choice;
        int option_index = 0;
        option long_options[] = {{"stack", no_argument, nullptr, 's'},
                                {"queue", no_argument, nullptr, 'q'},
                                {"output", required_argument, nullptr, 'o'},
                                {"help", no_argument, nullptr, 'h'},
                                {nullptr, 0, nullptr, '\0'}
        };

        // TODO: Fill in the double quotes, to match the mode and help options.
        while ((choice = getopt_long(argc, argv, "hsqo:", long_options, &option_index)) != -1){
            switch (choice){
            case 'h':
                printHelp(argv);
                exit(0);

            case 's':
                modeSpecified = true;
                if(mode == "queue"){
                    cerr << "Error: more than one mode" << endl;
                    exit(1);
                }
                mode = "stack";
                break;
            case 'q':
                modeSpecified = true;
                if(mode == "stack"){
                    cerr << "Error: more than one mode" << endl;
                    exit(1);
                }
                mode = "queue";
                break;
            case 'o':
                format = optarg;
                if (format != "M" && format != "L"){
                    // The first line of error output has to be a 'fixed' message for the autograder
                    // to show it to you.
                    cerr << "Error: invalid mode" << endl;
                    // The second line can provide more information, but you won't see it on the AG.
                    cerr << "  I don't know recognize: " << format << endl;
                    exit(1);
                } // if
                formatSpecified = true;
                break;

            default:
                cerr << "Error: invalid option" << endl;
                exit(1);
            } // switch
        }     // while
    } // getFormat()

    void readInfo(){
        char inputFormat;
        cin >> inputFormat;

        if(inputFormat != 'M' && inputFormat != 'L'){
            cerr << "Error: invalid inputs" << endl;
            exit(1);
        }

        cin >> floors;
        cin >> dimensions;

        resizeMap(floors, dimensions);

        if(inputFormat == 'M'){
            readMap();
        }
        else if(inputFormat == 'L'){
            readList();
        }
    }

    //resizes and fills with '.'
    void resizeMap(int f, int d){
        map.resize(f);
    
        for(int i = 0; i < f; ++i){
            map[i].resize(d);
            for(int j = 0; j < d; ++j){
                Tiles floor;
                floor.symbol = '.';
                floor.discovered = false;
                map[i][j].resize(d, floor);
            }
        }
    } // readInfo()

    void readMap(){
        for(int l = 0; l < floors; ++l){
            for(int r = 0; r < dimensions; ++r){
                for(int c = 0; c < dimensions; ++c){
                    inputData(l,r,c);
                }
            }
        }
    } // readMap()

    void inputData(int level, int row, int column){
        char inputs;
        cin >> inputs;
        while(inputs == '/'){
            string comments;
            getline(cin, comments);
            cin >> inputs;
        }

        if(inputs == '.'){
            //do nothing
        }
        else{
            checkChar(inputs);
            map[level][row][column].symbol = inputs;
            setStartOrEnd(inputs, level, row, column);
        }
    } //inputData()

    //checks for illegal characters
    void checkChar(char s){
        if(s != '#' && s != 'E' && s != 'S' && s != 'H' && s != '.'){
            cerr << "Error: invalid character" << endl;
            exit(1);
        }
    } // checkChar()

    void readList(){
        char inputs;
        while(cin >> inputs){
            if(inputs == '/'){
                string comments;
                getline(cin, comments);
            }
            else{
                int level;
                int row;
                int column;
                char sym;
                cin >> level >> inputs >> row >> inputs >> column >> inputs >> sym >> inputs;
                checkChar(sym);
                checkBounds(level, row, column);
                map[level][row][column].symbol = sym;
                setStartOrEnd(sym, level, row, column);
            }
        }
    } // readList()

    // checks if the list coordinates are out of bounds
    void checkBounds(int l, int r, int c){
        if(l < 0 || l >= floors){
            cerr << "Error: invalid coordinates" << endl;
            exit(1);
        }
        if(r < 0 || r >= dimensions || c < 0 || c >= dimensions){
            cerr << "Error: invalid coordinates" << endl;
            exit(1);
        }
    } // checkBounds()

    // keeps track of the start and hanger location
    void setStartOrEnd(char in, int l, int r, int c){
        if(in == 'S'){
            start = {l, r, c};
        }
        else if(in == 'H'){
            hanger = {l, r, c};
        }
    } // setStartOrEnd()
    
    // takes out of searchCont based on mode
    void searchMode(){
        if(mode == "stack"){
            curLoc = searchCont.back();
            searchCont.pop_back();
        }
        else if(mode == "queue"){
            curLoc = searchCont.front();
            searchCont.pop_front();
        }    
    } // searchMode()

    void searchMap(){
        solution = false;
        searchCont.push_back(start);
        while(!(searchCont.empty()) && !solution){
            searchMode();
            int curLevel = curLoc[0];
            int curRow = curLoc[1];
            int curColumn = curLoc[2];
            // search NESW in that order
            if(!(curRow-1 < 0))
                searchNorth(curLevel, curRow, curColumn);
            if(!(curColumn+1 >= dimensions))
                searchEast(curLevel, curRow, curColumn);
            if(!(curRow+1 >= dimensions))
                searchSouth(curLevel, curRow, curColumn);
            if(!(curColumn-1 < 0))
                searchWest(curLevel, curRow, curColumn);
            if(!solution && map[curLevel][curRow][curColumn].symbol == 'E'){
                checkElevator(curLevel, curRow, curColumn);
            }
            
        }
        searchCont.clear();

    } // searchMap()

    void searchNorth(int l, int r, int c){
        Tiles *locN = &map[l][r-1][c];
        if(locN->discovered || locN->symbol == '#'){
            //Do nothing
        }
        else{
            locN->discovered = true;
            locN->direction = 'n';
            searchCont.push_back({l, r-1, c});
            if(locN->symbol == 'H')
                solution = true;
        }

    } // searchNorth()

    void searchEast(int l, int r, int c){
        Tiles *locE = &map[l][r][c+1];
        if(locE->discovered || locE->symbol == '#'){
            //Do nothing
        }
        else{
            locE->discovered = true;
            locE->direction = 'e';
            searchCont.push_back({l,r, c+1});
            if(locE->symbol == 'H')
                solution = true;
        }
    } // searchEast()

    void searchSouth(int l, int r, int c){
        Tiles *locS = &map[l][r+1][c];
        if(locS->discovered || locS->symbol == '#'){
            //Do nothing
        }
        else{
            locS->discovered = true;
            locS->direction = 's';
            searchCont.push_back({l, r+1, c});
            if(locS->symbol == 'H')
                solution = true;
        }
    } // searchSouth()

    void searchWest(int l, int r, int c){
        Tiles *locW = &map[l][r][c-1];
        if(locW->discovered || locW->symbol == '#'){
            //Do nothing
        }
        else{
            locW->discovered = true;
            locW->direction = 'w';
            searchCont.push_back({l, r, c-1});
            if(locW->symbol == 'H')
                solution = true;
        }
    } // searchWest()

    void checkElevator(int l, int r, int c){
        for(int i = 0; i < floors; ++i){
            if(i != l && map[i][r][c].symbol == 'E' && !(map[i][r][c].discovered)){
                map[i][r][c].discovered = true;
                map[i][r][c].direction = static_cast<char>(l + '0');
                searchCont.push_back({i, r, c});
            }
        }
    } // checkElevator() 

    void generatePath(){
        if(!solution){
            noPath();
        }
        else{
            helpGenPath();
            printPath();
        }
    } // generatePath()

    void helpGenPath(){
        curLoc = hanger;
        while(curLoc != start){
            int l = curLoc[0];
            int r = curLoc[1];
            int c = curLoc[2];
            if(map[l][r][c].symbol != 'E'){
                findPath(l,r,c);
            }
            else if(elevDirect(l,r,c)){
                findPath(l,r,c);
            }
            else{
                char floorNum = static_cast<char>(l+'0');
                int f = static_cast<uint32_t>(map[l][r][c].direction - '0');
                Path path = {floorNum,f, r, c};
                gps.push_back(path);
                curLoc = {f, r, c};
            }
        }
    } // helpGenPath()

    bool elevDirect(int l, int r, int c){
        Tiles elev = map[l][r][c];
        if(elev.direction == 'n')
            return true;
        else if(elev.direction == 'e')
            return true;
        else if(elev.direction == 's')
            return true;
        else if(elev.direction == 'w')
            return true;
        else
            return false;
    } // elevDirect()

    void findPath(int l, int r, int c){
        if(map[l][r][c].direction == 'n'){
            Path path = {'n', l, r+1, c};
            gps.push_back(path);
            curLoc = {l, r+1, c};
        }
        else if(map[l][r][c].direction == 'e'){
            Path path = {'e', l, r, c-1};
            gps.push_back(path);
            curLoc = {l, r, c-1};
        }
        else if(map[l][r][c].direction == 's'){
            Path path = {'s', l, r-1, c};
            gps.push_back(path);
            curLoc = {l, r-1, c};
        }
        else if(map[l][r][c].direction == 'w'){
            Path path = {'w', l, r, c+1};
            gps.push_back(path);
            curLoc = {l, r, c+1};
        }
        else{
            // should not happen
            cerr << "Error: invalid direction" << endl;
            exit(1);
        }
    } // findPath()

    void noPath(){
        if(format == "L"){
            cout << "//path taken" << endl;
        }
        else if(format == "M"){
            printMap();
        }
        else{
            // should not be able to get to this point
            cerr << "Error: invalid output format" << endl;
            exit(1);
        }
    } // noPath()

    void printMap(){
        cout << "Start in level " << start[0] << ", row " << start[1]
            << ", column " << start[2] << "\n";
        for(int l = 0; l < floors; ++l){
            cout << "//level " << l << "\n";
            for(int r = 0; r < dimensions; ++r){
                for(int c = 0; c < dimensions; ++c){
                    cout << map[l][r][c].symbol;
                }
                cout << "\n";
            }
        }
    } // printMap();

    void printPath(){
        if(format == "L"){
            printList();
        }
        else if(format == "M"){
            makeMap();
            printMap();
        }
        else{
            // should not be able to get to this point
            cerr << "Error: invalid output format" << endl;
            exit(1);
        }
    } // printPath()

    void printList(){
        cout << "//path taken\n";
        Path *coordinates;
        while(!gps.empty()){
            coordinates = &gps.back();
            cout << "(" << coordinates->level << "," <<
                coordinates->row << "," <<
                coordinates->column << "," <<
                coordinates->direction << ")\n";
            gps.pop_back();
        }
    } // printList()

    void makeMap(){
        while(!(gps.empty())){
            Path *coord = &gps.back();
            map[coord->level][coord->row][coord->column].symbol = coord->direction;
            gps.pop_back();
        }
    } // makeMap()

//public members
private:
    struct Tiles{
        char symbol;
        bool discovered;
        char direction;
    };
    struct Path{
        char direction;
        int level;
        int row;
        int column;
    };
    vector<vector<vector<Tiles>>> map;
    deque<Path> gps;
    int floors;
    int dimensions;
    vector<int> start;
    vector<int> hanger;
    string mode;
    string format;
    deque<vector<int>> searchCont;
    vector<int> curLoc;
    bool solution;

// private members
}; // Class Navigation

int main(int argc, char *argv[]){
    //speed up I/O
    ios_base::sync_with_stdio(false);

    Navigation navBot;
    navBot.getFormat(argc, argv);
    navBot.readInfo();
    navBot.searchMap();
    navBot.generatePath();
    
    return 0;
}
