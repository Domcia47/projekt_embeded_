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
	
	std::vector<std::string> conditions;

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
			std::cerr << "Niepoprawne indeksy wierzchołków\n";
			return;
		}
		if (i_Vertex_Index_1 == i_Vertex_Index_2)
		{
			std::cerr << "Nie można dodać krawędzi do wierzchołka\n";
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

	int edgeWeight(int i_Vertex_Index_1, int i_Vertex_Index_2)
	{
		if (matrix[i_Vertex_Index_1][i_Vertex_Index_2] == 0)
		{
			std::cerr << "Krawedz nie istnieje";
			return 0;
		}
		else
			return matrix[i_Vertex_Index_1][i_Vertex_Index_2];
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

// Parsing conditional shenanigans
struct Task
{
	int child;
	int weight;
	std::string conditional;
};

Task parseTaskConnection(const std::string& input)
{
	Task result;
	result.conditional = "";

	// Find the number at the front
	size_t pos = 0;
	while (pos < input.length() && isdigit(input[pos]))
	{
		pos++;
	}
	result.child = std::stoi(input.substr(0, pos));

	// Check if there is a 'c' between the number and '('
	size_t cPos           = input.find('c', pos);
	size_t openBracketPos = input.find('(', pos);

	if (cPos != std::string::npos && cPos < openBracketPos)
	{
		// Extract conditional part
		size_t commaPos    = input.rfind(',', input.length() - 1);
		result.conditional = input.substr(openBracketPos + 1, commaPos - openBracketPos - 1);

		// Extract weight
		size_t closeBracketPos = input.find(')', commaPos);
		result.weight          = std::stoi(input.substr(commaPos + 1, closeBracketPos - commaPos - 1));
	}
	else
	{
		// No 'c' found, just extract the number between the brackets
		size_t closeBracketPos = input.find(')', openBracketPos);
		result.weight          = std::stoi(input.substr(openBracketPos + 1, closeBracketPos - openBracketPos - 1));
	}

	return result;
}
std::vector<Task> parseTaskLine(const std::string& line)
{
	std::istringstream iss(line);
	std::vector<Task> Tasks;
	std::string token;
	// Skip the first token (T<number>)
	iss >> token;

	int numTasks;
	iss >> numTasks;

	for (int i = 0; i < numTasks; ++i)
	{
		std::string TaskStr;
		iss >> TaskStr;

		Tasks.push_back(parseTaskConnection(TaskStr));
	}

	return Tasks;
}

// Leaving it here outside the class since it's a big boi
void Graf::readFromFile(std::string path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		std::cerr << "Nie mozna otworzyc pliku " << path << std::endl;
		return;
	}

	std::string line;
	// Wczytywanie tasków
	std::getline(file, line);
	std::istringstream ss(line);
	std::string tasksNum;
	ss >> tasksNum >> tasksNum >> tasksNum;

	int num = std::stoi(tasksNum);
	createVertices(num);
	conditions.resize(num, "no");
	
	int parentnum = 0;
	std::string token;
	for (int i = 0; i < num; i++)
	{
		getline(file, line);

		auto tasks = parseTaskLine(line);
		for (const auto& task : tasks)
		{
			if(!task.conditional.empty()) conditions[task.child] = task.conditional;
			addEdge(parentnum, task.child, task.weight);

		}

		parentnum++;
	}

	// Wczytywanie jednostek
	getline(file, line);
	std::istringstream sss(line);
	std::string proc_s;
	sss >> proc_s >> proc_s >> proc_s;
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
		iss >> token;
		for (int j = 0; j < numberOfPE; j++)
		{
			iss >> token;
			int token_i = stoi(token);
			costs[i][j] = token_i;
		}
	}

	file.close();
}
