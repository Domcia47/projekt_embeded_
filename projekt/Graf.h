
#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Graf {
    // od razu musimy zadeklarowac maksymalna ilosc wierzcholkow - tutaj 300;
public:
    int matrix[300][300];
    std::vector<std::vector<int>> procs;
    std::vector<std::vector<int>> times;
    std::vector<std::vector<int>> costs;
    std::vector<std::vector<int>> comms;
    int numberOfVertices;
    int numberOfEdges;
    int numberOfPE;

public:
    Graf();
    void createVertices(int ile);
    void addEdge(int i_Vertex_Index_1, int i_Vertex_Index_2);
    void removeEdge(int i_Vertex_Index_1, int i_Vertex_Index_2);
    bool checkEdge(int i_Vertex_Index_1, int i_Vertex_Index_2);
    int vertexDegree(int idx);
    std::vector<int> getNeighbourIndices(int idx);
    std::vector<int> getPredecessorIndices(int idx);
    void printNeighbourIndices(int idx);
    int getNumberOfEdges();
    int getNumberOfVertices();
    int getNumberOfPE();
    int is_PP(int idx);
    void readFromFile(std::string path);

private:
    void clear();
};
