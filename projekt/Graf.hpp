#pragma once
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Graf
{
	// od razu musimy zadeklarowac maksymalna ilosc wierzcholkow - tutaj 300;
public:
	int matrix[300][300] = {};

	std::vector<std::vector<int>> procs;
	std::vector<std::vector<int>> times;
	std::vector<std::vector<int>> costs;
	std::vector<std::vector<int>> comms;

	int numberOfVertices = 0;
	int numberOfEdges    = 0;
	int numberOfPE       = 0;

public:
	Graf()  = default;
	~Graf() = default;

	void createVertices(int ile) { numberOfVertices = ile; }

	void addEdge(int i_Vertex_Index_1, int i_Vertex_Index_2, int weight)
	{
		if (i_Vertex_Index_1 >= numberOfVertices || i_Vertex_Index_2 >= numberOfVertices)
		{
			std::cerr << "Niepoprawne indeksy wierzcho�k�w\n";
			return;
		}
		if (i_Vertex_Index_1 == i_Vertex_Index_2)
		{
			std::cerr << "Nie mo�na doda� kraw�dzi do wierzcho�ka\n";
			return;
		}

		matrix[i_Vertex_Index_1][i_Vertex_Index_2] = weight;
		++numberOfEdges;
	}

	void removeEdge(int i_Vertex_Index_1, int i_Vertex_Index_2)
	{
		if (matrix[i_Vertex_Index_1][i_Vertex_Index_2] >= 1)
		{
			matrix[i_Vertex_Index_1][i_Vertex_Index_2] = 0;
			--numberOfEdges;
		}
	}

	bool checkEdge(int i_Vertex_Index_1, int i_Vertex_Index_2)
	{
		if (matrix[i_Vertex_Index_1][i_Vertex_Index_2] >= 1)
			return true;
		return false;
	}

	int vertexDegree(int idx)
	{
		int sum = 0;

		for (int i = 0; i < numberOfVertices; i++)
			// if (matrix[idx][i] >= 1)
			// 	sum++;
			sum += (matrix[idx][i] >= 1) ? 1 : 0;

		return sum;
	}

	std::vector<int> getNeighbourIndices(int idx)
	{
		std::vector<int> neighbours;
		for (int i = 0; i < numberOfVertices; i++)
		{
			if (matrix[idx][i] >= 1)
				neighbours.push_back(i);
		}
		return neighbours;
	}

	std::vector<int> getPredecessorIndices(int idx)
	{
		std::vector<int> predecessors;
		for (int i = 0; i < numberOfVertices; i++)
		{
			if (matrix[i][idx] >= 1)
				predecessors.push_back(i);
		}
		return predecessors;
	}
	void printNeighbourIndices(int idx)
	{
		std::vector<int> neighbours = getNeighbourIndices(idx);
		for (auto index : neighbours)
			std::cout << index << " ";

		std::cout << std::endl;
	}

	int getNumberOfEdges() { return numberOfEdges; }
	int getNumberOfVertices() { return numberOfVertices; }
	int getNumberOfPE() { return numberOfPE; }
	bool is_PP(int idx) { return procs[idx][1] == 1; }

	void readFromFile(std::string path);

private:
	void clear();
};

// Leaving it here outside the class since it's a big boi
void Graf::readFromFile(std::string path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cerr << "Nie można otworzyć pliku " << path << std::endl;
		return;
	}

	std::string line;
	// Wczytywanie tasków
	std::getline(file, line);
	std::istringstream ss(line);
	std::string tasksNum;
	ss >> tasksNum >> tasksNum;

	int num = std::stoi(tasksNum);
	createVertices(num);
	int parentnum = 0;
	std::string token;
	for (int i = 0; i < num; i++)
	{
		getline(file, line);
		std::istringstream iss(line);

		iss >> token;

		iss >> token;
		while (iss >> token)
		{
			if (token.find('c') == std::string::npos)
			{
				token.pop_back();
				size_t pos = token.find("(");
				int child  = std::stoi(token.substr(0, pos));
				int weight = std::stoi(token.substr(pos + 1));
				addEdge(parentnum, child, weight);
			}
			else
			{
				size_t pos = token.find("c");
				int child  = std::stoi(token.substr(0, pos));
				// tu coś dalej
			}
		}
		parentnum++;
	}

	// Wczytywanie jednostek
	getline(file, line);
	std::istringstream sss(line);
	std::string proc_s;
	sss >> proc_s >> proc_s;
	numberOfPE = stoi(proc_s);
	procs.resize(numberOfPE, std::vector<int>(2, 0));  // Inicjalizacja proc jako wektora wektorów z dwiema kolumnami

	for (int i = 0; i < numberOfPE; i++)
	{
		getline(file, line);
		std::istringstream iss(line);
		iss >> token;
		int token_i = stoi(token);
		procs[i][0] = token_i;  // Ustawienie wartości pierwszej kolumny na wartość token_i

		// Pomijam drugą kolumnę, bo jest w sumie bezużyteczna
		iss >> token;
		iss >> token;
		token_i     = stoi(token);
		procs[i][1] = token_i;  // Ustawienie wartości drugiej kolumny na wartość token_i
	}

	// Wczytywanie czasów
	getline(file, line);
	times.resize(num, std::vector<int>(numberOfPE, 0));  // Inicjalizacja times
	for (int i = 0; i < num; i++)
	{
		getline(file, line);
		std::istringstream iss(line);
		for (int j = 0; j < numberOfPE; j++)
		{
			iss >> token;
			int token_i = stoi(token);
			times[i][j] = token_i;
		}
	}

	// Wczytywanie kosztów
	getline(file, line);
	costs.resize(num, std::vector<int>(numberOfPE, 0));  // Inicjalizacja costs
	for (int i = 0; i < num; i++)
	{
		getline(file, line);
		std::istringstream iss(line);
		for (int j = 0; j < numberOfPE; j++)
		{
			iss >> token;
			int token_i = stoi(token);
			costs[i][j] = token_i;
		}
	}

	file.close();
}