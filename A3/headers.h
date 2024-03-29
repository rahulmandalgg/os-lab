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
#include <cstring>
#include <sstream>
#include <climits>
#include <chrono>
#include <random>

using namespace std;

#define SHMSIZE 4294967296
#define MAX_COUNT 8192
#define CONSUMER_COUNT 10
#define PRODUCER_TIMEOUT 50
#define CONSUMER_TIMEOUT 30

void* global_gptr;

// struct used to store the adjacency list for a node
typedef struct AdjList
{
    public:
    int current; // current node
    int neighborCount; // no. of neighbors
    int neighborlist[MAX_COUNT]; // array of neighbors of current node
} AdjList;

// object definition to store graph in adjacency list form, along with its shortest paths from each source node
class Graph
{
    public:
    int nodeCount; // total nodes
    AdjList nodelist[MAX_COUNT]; // adjacency list
    int shortest_path[MAX_COUNT][MAX_COUNT]; // shortest path matrix

    Graph()
    {
        nodeCount = 0;
        for(int i=0; i<MAX_COUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
        for(int i=0; i<MAX_COUNT; i++)
            for(int j=0; j<MAX_COUNT; j++)
                shortest_path[i][j] = INT_MAX;
    }
    int init(string filepath);
    int addEdge(int x, int y);
    void print_graph(string filepath, int startidx, int endidx);
    void print_path(string filepath, int startidx, int endidx);
    int dijkstra(int source, string filename);
};

// initializes the graph object with the textfile data
int Graph::init(string filepath)
{
    // initialize
    nodeCount = 0;
    for(int i=0; i<MAX_COUNT; i++) { this->nodelist[i].current = -1; this->nodelist[i].neighborCount = 0; }
    for(int i=0; i<MAX_COUNT; i++) for(int j=0; j<MAX_COUNT; j++) shortest_path[i][j] = INT_MAX;

    fstream fs;
    fs.open(filepath, ios::in); 
    if(!fs) return -1;
    if(fs.is_open())
    {
        string lin;
        int x, y;
        while(getline(fs, lin))
        {
            // read x,y from input file
            stringstream slin(lin);
            slin>>x>>y;

            int ix = nodelist[x].current, iy = nodelist[y].current;
            if(ix>=0 && iy>=0) // if both x,y are old nodes
            {
                // add y into x-list if y isn't already present
                for(int j=0; j<nodelist[x].neighborCount; j++) // iterate through neighbors of x
                    if(nodelist[x].neighborlist[j] == y) goto y_repeated; // if y already found, skip addition step

                nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
                nodelist[x].neighborCount += 1;
                if(nodelist[x].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
                y_repeated:

                // add x into y-list if x isn't already present
                for(int j=0; j<nodelist[y].neighborCount; j++) // iterate through neighbors of y
                    if(nodelist[y].neighborlist[j] == x) goto x_repeated; // if x already found, skip addition step
                // else add x as neighbor
                nodelist[y].neighborlist[nodelist[iy].neighborCount] = x;
                nodelist[y].neighborCount += 1;
                if(nodelist[y].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
                x_repeated:
                ;
            }
            else if(ix>=0 && iy==-1) // x is old node, y is new
            {
                // add y to x-list
                nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
                nodelist[x].neighborCount += 1;
                if(nodelist[x].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
                // init y-list
                nodelist[y].current = y;
                nodelist[y].neighborlist[0] = x;
                nodelist[y].neighborCount = 1;
                nodeCount++;
            }
            else if(ix==-1 && iy>=0) // x is new node, y is old
            {
                // add x to y-list
                nodelist[y].neighborlist[nodelist[y].neighborCount] = x;
                nodelist[y].neighborCount += 1;
                if(nodelist[y].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
                // init x-list
                nodelist[x].current = x;
                nodelist[x].neighborlist[0] = y;
                nodelist[x].neighborCount = 1;
                nodeCount++;
            }
            else // both are new nodes
            {
                // init x-list
                nodelist[x].current = x;
                nodelist[x].neighborlist[0] = y;
                nodelist[x].neighborCount = 1;
                nodeCount++;
                // init y-list
                nodelist[y].current = y;
                nodelist[y].neighborlist[0] = x;
                nodelist[y].neighborCount = 1;
                nodeCount++;
            }
        }
    }
    return 0;
}

// adds an edge (x, y) to the graph
int Graph::addEdge(int x, int y)
{
    int ix = nodelist[x].current, iy = nodelist[y].current;

    if(ix>=0 && iy>=0) // if both x,y are old nodes
    {
        // add y into x list only if y not found
        for(int j=0; j<nodelist[x].neighborCount; j++) // iterate through neighbors of x
            if(nodelist[x].neighborlist[j] == y) goto y_repeated; // if y already found, skip addition step
        // else add y as neighbor
        nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
        nodelist[x].neighborCount += 1;
        if(nodelist[x].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
        y_repeated:

        // add x into y list only if x not found
        for(int j=0; j<nodelist[y].neighborCount; j++) // iterate through neighbors of y
            if(nodelist[y].neighborlist[j] == x) goto x_repeated; // if x already found, skip addition step
        // else add x as neighbor
        nodelist[y].neighborlist[nodelist[iy].neighborCount] = x;
        nodelist[y].neighborCount += 1;
        if(nodelist[y].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
        x_repeated:
        ;
    }
    else if(ix>=0 && iy==-1) // x found but not y
    {
        // add y to x list
        nodelist[x].neighborlist[nodelist[x].neighborCount] = y;
        nodelist[x].neighborCount += 1;
        if(nodelist[x].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
        // init y list
        nodelist[y].current = y;
        nodelist[y].neighborlist[0] = x;
        nodelist[y].neighborCount = 1;
        nodeCount++;
    }
    else if(ix==-1 && iy>=0) // x is new node, not y
    {
        // add x to y list
        nodelist[y].neighborlist[nodelist[y].neighborCount] = x;
        nodelist[y].neighborCount += 1;
        if(nodelist[y].neighborCount >= MAX_COUNT+1) return -1; // check if limit reached
        // init x list
        nodelist[x].current = x;
        nodelist[x].neighborlist[0] = y;
        nodelist[x].neighborCount = 1;
        nodeCount++;
    }
    else // both new nodes
    {
        // init x list
        nodelist[x].current = x;
        nodelist[x].neighborlist[0] = y;
        nodelist[x].neighborCount = 1;
        nodeCount++;
        // init y list
        nodelist[y].current = y;
        nodelist[y].neighborlist[0] = x;
        nodelist[y].neighborCount = 1;
        nodeCount++;
    }
    return 0;
}

// single source Dijkstra implementation
int Graph::dijkstra(int source, string filename)
{
    // Create a priority queue to hold the nodes to be visited, in min-heap priority of their distance
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

    // Initialize single source
    if(nodelist[source].current == -1) return -1; // source not in graph
    pq.push(make_pair(0, source));
    shortest_path[source][source] = 0;

    // Loop until the priority queue is empty
    while (!pq.empty())
    {
        // Dequeue the node with the smallest distance (call it u)
        int u = pq.top().second;
        pq.pop();
        if(nodelist[u].current == -1) return -1; // u not in graph

        // Loop over the neighbors of the dequeued node
        for (int i = 0; i < nodelist[u].neighborCount; i++)
        {

            int v = nodelist[u].neighborlist[i];

            int weight = 1; // assuming all edge weights are 1
            // Update the distance if a shorter path is found
            if ((shortest_path[source][v] == INT_MAX) || (shortest_path[source][v] > shortest_path[source][u] + weight))
            {
                shortest_path[source][v] = shortest_path[source][u] + weight;
                pq.push(make_pair(shortest_path[source][v], v));
            }
        }
    }
    return 0;
}

void Graph::print_graph(string filepath, int startidx, int endidx)
{
    // cout<<"Total Nodes: "<<nodeCount<<endl;
    ios_base::sync_with_stdio(false);
    ofstream outfile;
    outfile.open(filepath, ios_base::app);
    if(!outfile) { cerr<<"ERROR: Cannot open file."<<endl; return; }
    for(int i=startidx; i<=endidx;)
    {
        if(nodelist[i].current == -1) continue;
        outfile << nodelist[i].current<<"\t:\t";
        // cout<<nodelist[i].current<<":";
        for(int j=0; j<nodelist[i].neighborCount; j++) 
        {
            outfile<<nodelist[i].neighborlist[j]<< " ";
            // cout<<nodelist[i].neighborlist[j]<< " ";
        }
        outfile<<"\n";
        // cout<<endl;
        i++;
    }
    outfile<<"----------------------------------------------"<<endl;
    outfile.close();
    ios_base::sync_with_stdio(true);
}

void Graph::print_path(string filepath, int startidx, int endidx)
{
    ios_base::sync_with_stdio(false);
    ofstream outfile;
    outfile.open(filepath, ios_base::app);
    if(!outfile) { cerr<<"ERROR: Cannot open file."<<endl; return; }
    for(int i=startidx; i<=endidx; i++)
        for(int j=0; j<nodeCount; j++)
        {
            if(shortest_path[i][j] == INT_MAX) outfile<<i<<"->"<<j<<"\t\t:\tINF\n";
            else outfile<<i<<"->"<<j<<"\t\t:\t"<<shortest_path[i][j]<<"\n";
        }
    outfile<<"----------------------------------------------"<<endl;
    outfile.close();
    ios_base::sync_with_stdio(true);
}

#endif
