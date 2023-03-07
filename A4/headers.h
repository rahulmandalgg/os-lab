#ifndef __HEADERS_H
#define __HEADERS_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <semaphore.h>
#include <fcntl.h>
#include <cstring>
#include <sstream>
#include <climits>
#include <chrono>
#include <random>

using namespace std;

#define MAX_COUNT 37701
int x, y;

// struct used to store the adjacency list for a node
typedef struct AdjList
{
    int current;       // current node
    int neighborCount; // no. of neighbors
    vector<int> nbrs;  // array of neighbors of current node
    // queue <int> wall; // queue to store action generated by myself
    // queue <int> feed; // queue to store action generated by friends
} AdjList;

// object definition to store graph in adjacency list form, along with its shortest paths from each source node
class Graph
{
public:
    vector<AdjList> nodelist; // adjacency list

    int init(string filepath);

    void print_graph();

    void populate_wall();
    void print_wall();
};

void Graph::populate_wall()
{
    // populate wall
    // nodelist[0].wall.push(0);
    // nodelist[0].wall.push(1);
    // nodelist[0].wall.push(2);
    // nodelist[0].wall.push(3);
}

void Graph::print_wall()
{
    // cout << nodelist[0].wall.size() << endl;
    // while(!nodelist[0].wall.empty())
    // {
    //     int x = nodelist[0].wall.front();
    // 	cout<<" "<<x;
    // 	nodelist[0].wall.pop();
    // }
}

// initializes the graph object with the textfile data
int Graph::init(string filepath)
{
    fstream fs;
    // int x, y;
    fs.open(filepath, ios::in);
    if (!fs)
    {
        return -1; // error opening file
    }
    int k = 0;
    if (fs.is_open())
    {
        string line;

        getline(fs, line); // skip first line (header)
        while (getline(fs, line))
        {
            k++;
            if(k==11) break;
            stringstream slin(line);
            string token;
            getline(slin, token, ',');
            x = stoi(token);
            getline(slin, token, ',');
            y = stoi(token);
            // cout<<x<<" "<<y<<endl;
            // find x and y in graph if x and y both are found in the graph, add y to x's neighbor list and vice versa
            bool foundx = false, foundy = false;
            for (auto &node : nodelist)
            {
                if (node.current == x)
                {
                    node.nbrs.push_back(y);
                    foundx = true;
                }
                if (node.current == y)
                {
                    node.nbrs.push_back(x);
                    foundy = true;
                }
                if (foundx && foundy)
                    break;
                if (!foundx)
                {
                    AdjList newnode;
                    newnode.current = x;
                    newnode.nbrs.push_back(y);
                    nodelist.push_back(newnode);
                }
                if (!foundy)
                {
                    AdjList newnode;
                    newnode.current = y;
                    newnode.nbrs.push_back(x);
                    nodelist.push_back(newnode);
                }
                foundx=false;
                foundy=false;
            }
        }
    }

    return 0; // success
}

void Graph::print_graph()
{
    for (auto &node : nodelist)
    {
        cout << node.current << ": ";
        for (auto &nbr : node.nbrs)
        {
            cout << nbr << " ";
        }
        cout << endl;
    }
}

#endif