#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>
#include <queue>
#include <regex>

#include "Graf.hpp"


int total_cost = 0, total_time = 0;
int Tmax;
struct Result
{
	std::vector<int> path;
	int total_time;
};

const int MAX = std::numeric_limits<int>::max();
// struktura potrzebna do przechowywania wszytskich informacji o zadaniu na jednostce, wykorzystana w wekotrze poniżej
struct unit_for_task
{
	int procNum;
	int procNumIndex;
	int time;
	int cost;
	int start_time;
	int end_time;
};
struct task_with_parent {
	int task;
	int parent;
};
class TaskGraph : public Graf
{
public:
	std::vector<unit_for_task> chosen;  // wektor do monitorowania jaka jednostka została przydzielona do jakiego
										// zadania, indeks to numer zadania
	int num_of_tasks;
	int numOfPE;
	// tablica typów jednostek, indeks to typ jednostki, dla każdego typu wektor instancji tych jednostek,
	// każda jednostka posiada wektor par, czyli interwały swojej pracy
	std::vector<std::vector<std::pair<int, int>>> work_times[30];  // indeks to numer procesora
	bool standardized = false;
	std::vector<std::vector<double>> standarized_procs;
	std::vector<std::vector<double>> standarized_times;
	std::vector<std::vector<double>> standarized_costs;
	std::vector<std::vector<double>> all_criteria;
	std::vector<bool> visited;
	std::queue<task_with_parent> tasks_to_do;
	std::vector<bool> parent_executed;
	double x = 1.0, y = 1.0, z = 1.0;
	const double increaseFactor = 1.5; // Współczynnik wzmacniający wagę y
	const double decreaseFactor = 0.7;

	TaskGraph(std::string path)
	{
		readFromFile(path);
		numOfPE      = getNumberOfPE();
		num_of_tasks = getNumberOfVertices();
		visited.resize(num_of_tasks, false);
		parent_executed.resize(num_of_tasks, false);

		for (int i = 0; i < num_of_tasks; i++)
			chosen.push_back({-1, -1, -1, -1, -1, -1});
	}
	void clear()
	{
		for (int i = 0; i < num_of_tasks; i++)
			chosen[i]={-1, -1, -1, -1, -1, -1};

		for (int i = 0; i < numOfPE; ++i)
			work_times[i].clear();
		
		for (int i = 0; i < num_of_tasks; i++)
			visited[i] = false;
		
		for (int i = 0; i < num_of_tasks; i++)
			parent_executed[i] = false;
		
		
		
	}

	// obliczanie odchylenia standardowego
	template <typename T>
	double std_dev(const std::vector<std::vector<T>>& data, double mean_val)
	{
		double sum = 0.0;
		int count  = 0;
		for (const auto& row : data)
		{
			for (const auto& val : row)
			{
				sum += (val - mean_val) * (val - mean_val);
				count++;
			}
		}
		return std::sqrt(sum / count);
	}
	// obliczanie średniej
	template <typename T>
	double mean(const std::vector<std::vector<T>>& data)
	{
		double sum = 0.0;
		int count  = 0;
		for (const auto& row : data)
		{
			for (const auto& val : row)
			{
				sum += val;
				count++;
			}
		}
		return sum / count;
	}
	// standaryzacja tablic z jednostkami, czasami, kosztami
	void standardize_all_arrays()
	{	
		standarized_costs.clear();
		standarized_procs.clear();
		standarized_times.clear();
		double mean_procs    = mean(procs);
		double std_dev_procs = std_dev(procs, mean_procs);
		for (int i = 0; i < procs.size(); i++)
		{
			std::vector<double> row;
			row.push_back((procs[i][0] - mean_procs) / std_dev_procs);
			row.push_back(procs[i][1]);
			standarized_procs.push_back(row);
		}

		double mean_times    = mean(times);
		double std_dev_times = std_dev(times, mean_times);
		for (int i = 0; i < times.size(); i++)
		{
			std::vector<double> row;
			for (int j = 0; j < times[i].size(); j++)
				row.push_back((times[i][j] - mean_times) / std_dev_times);

			standarized_times.push_back(row);
		}

		double mean_costs    = mean(costs);
		double std_dev_costs = std_dev(costs, mean_costs);
		for (int i = 0; i < costs.size(); i++)
		{
			std::vector<double> row;
			for (int j = 0; j < costs[i].size(); j++)
				row.push_back((costs[i][j] - mean_costs) / std_dev_costs);

			standarized_costs.push_back(row);
		}
	}

	// przypisz typ jednostki na podstawie standaryzacji
	void assign_unit(int task, int start_time, int ancestor)
	{
		int assigned_unit;
		if (!standardized)
		{
			standardize_all_arrays();
			standardized = true;
			/*if (!all_criteria.empty()) {
				all_criteria.resize(0);
			}*/
			all_criteria.clear();
			for (int i = 0; i < num_of_tasks; ++i)
			{
				std::vector<double> row;
				for (int j = 0; j < numOfPE; ++j)
				{
					row.push_back(x * standarized_costs[i][j] + y * standarized_times[i][j] +
								  z * standarized_procs[j][0]);
				}
				all_criteria.push_back(row);
			}
		}
		double all_criteria_min = MAX;
		int min_index           = -1;
		for (int i = 0; i < procs.size(); i++)
		{
			if (all_criteria[task][i] < all_criteria_min)
			{
				all_criteria_min = all_criteria[task][i];
				min_index        = i;
			}
		}
		assigned_unit = min_index;
		// po wybraniu typu jednostki wybieram konkretną jej instancję, wpisuje czasy pracy i uzupełniam wektor
		// wybrnaych jednostek
		if (assigned_unit == -1)
			std::cout << "-1";
		// jeśli jednostka jest typu HC zawsze trzeba wybrać nową, bo może być użyta tylko raz
		if (!is_PP(assigned_unit))
		{
			work_times[assigned_unit].push_back(std::vector<std::pair<int, int>>());
			int idx      = work_times[assigned_unit].size();
			chosen[task] = {assigned_unit,
							idx,
							times[task][assigned_unit],
							costs[task][assigned_unit],
							start_time,
							start_time + times[task][assigned_unit]};
		}
		// jeśli typu PP ale jeszcze takiej nie używaliśmy wybieramy nową
		else if (work_times[assigned_unit].empty())
		{
			chosen[task] = {assigned_unit,
							0,
							times[task][assigned_unit],
							costs[task][assigned_unit],
							start_time,
							start_time + times[task][assigned_unit]};
			work_times[assigned_unit].push_back(std::vector<std::pair<int, int>>());
			work_times[assigned_unit][0].push_back(std::pair<int, int>(start_time, chosen[task].end_time));
		}
		else
		{
			// sprawdzam która jednostka zakończy pracę przed koniecznościa rozpoczęcia obecnego zadania
			int idx_of_PP      = 0;
			bool found_free_PP = false;
			for (auto& vec_of_times : work_times[assigned_unit])
			{
				if (vec_of_times.empty())
					continue;
				if (vec_of_times.back().second <= start_time)
				{
					chosen[task] = {assigned_unit,
									idx_of_PP,
									times[task][assigned_unit],
									costs[task][assigned_unit],
									start_time,
									start_time + times[task][assigned_unit]};
					vec_of_times.push_back(std::pair<int, int>(start_time, chosen[task].end_time));
					found_free_PP = true;
					break;
				}
				idx_of_PP++;
			}
			// jeśli nie ma wolnej jednostki wybieram nową jednostkę
			if (!found_free_PP)
			{
				chosen[task] = {assigned_unit,
								idx_of_PP,
								times[task][assigned_unit],
								costs[task][assigned_unit],
								start_time,
								start_time + times[task][assigned_unit]};
				work_times[assigned_unit].push_back(std::vector<std::pair<int, int>>());
				work_times[assigned_unit][idx_of_PP].push_back(std::pair<int, int>(start_time, chosen[task].end_time));
			}
		}
	}
	bool check_condtions(int task,int parent,int start_time) {
		std::string condition = conditions[task];
		std::regex minPattern(R"(min(\d+))");
		std::regex exLessPattern(R"(ex\(<(\d+)\))");
		std::regex exGreaterPattern(R"(ex\(>(\d+)\))");
		std::regex exRangePattern(R"(ex\((\d+)(;\d+)*\))");
		std::regex noPattern(R"(no)");

		std::smatch match;
		if (std::regex_match(condition, match, noPattern)) {
			return true;
		}
		else if (std::regex_match(condition, match, minPattern)) {
			int value = std::stoi(match[1]);
			if (edgeWeight(parent,task)>=value ){
				return true;
			}
		}
		else if (std::regex_match(condition, match, exLessPattern)) {
			int value = std::stoi(match[1]);
			int counter = 0;
			for (int i = 0; i < num_of_tasks; i++) {
				if (chosen[i].end_time < start_time) {
					counter++;
				}
			}
			if (counter < value) return true;
		}
		else if (std::regex_match(condition, match, exGreaterPattern)) {
			int value = std::stoi(match[1]);
			int counter = 0;
			for (int i = 0; i < num_of_tasks; i++) {
				if (chosen[i].end_time > start_time) {
					counter++;
				}
			}
			if (counter > value) return true;
		}
		else if (std::regex_match(condition, match, exRangePattern)) {
			std::vector<int> values;
			for (size_t i = 1; i < match.size(); ++i) {
				if (match[i].matched) {
					std::string valueStr = match[i].str();
					std::regex numberPattern(R"(\d+)");
					std::sregex_iterator begin(valueStr.begin(), valueStr.end(), numberPattern);
					std::sregex_iterator end;
					for (auto it = begin; it != end; ++it) {
						values.push_back(std::stoi(it->str()));
					}
				}
			}
			int new_start_time = -1;
			for (auto t : values) {
				
				if (chosen[t].end_time == -1) return false;
				if ( chosen[t].end_time > new_start_time) {
					 new_start_time = chosen[t].end_time;
				}
				
			}
			if (start_time != -1) {
				chosen[task].start_time = new_start_time;
				return true;
			}
			return true;
		}
		else {
			std::cerr << "Unknown condition type" << std::endl;
		}
		return false;
		
	}
	void assign_units() {
		add_tasks_to_queue(0);
		while (!tasks_to_do.empty()) {
			int task = tasks_to_do.front().task;
			int parent = tasks_to_do.front().parent;
			tasks_to_do.pop();
				
			
			if (chosen[task].procNum == -1) {
				if (check_condtions(task, parent, chosen[parent].end_time) && parent_executed[task]) {

					std::vector<int> children = getNeighbourIndices(task);
					for (int child : children)
					{
						parent_executed[child] = true;
					}
					if (chosen[task].start_time == -1) {
						assign_unit(task, chosen[parent].end_time + edgeWeight(parent, task) / 10, parent);
					}
					else {
						if (chosen[task].start_time > chosen[parent].end_time) {
							assign_unit(task, chosen[task].start_time, parent);
						}
						else {
							assign_unit(task, chosen[parent].end_time + edgeWeight(parent, task) / 10, parent);
						}
					}
				}

				else {
					tasks_to_do.push({ task,parent });
				}
			}
		}
	}
	void add_tasks_to_queue(int start)
	{	
		std::vector<int> children = getNeighbourIndices(start);
		
		
		for (int child : children)
		{
			if (!visited[child]) {
				tasks_to_do.push({ child,start });
				visited[child] = true;
			}
			add_tasks_to_queue(child);  // Rekurencyjne wywołanie dla dzieci dzieci
		}
		
	}
	int calulate_total_tasks_cost()
	{
		total_cost = 0;
		std::vector<std::pair<int, int>> v;

		for (int i = 0; i < num_of_tasks; i++)
		{
			total_cost += chosen[i].cost;
			if (is_PP(chosen[i].procNum))
			{
				std::pair<int, int> PE = {chosen[i].procNum, chosen[i].procNumIndex};
				auto it                = std::find(v.begin(), v.end(), PE);
				if (it == v.end())
				{
					total_cost += costs[i][chosen[i].procNum];
					v.push_back(PE);
				}
			}
		}
		return total_cost;
	}
	void assign_minimal_units_recursively(int start)
	{
		std::vector<int> children = getNeighbourIndices(start);
		for (int child : children)
		{
			if (chosen[child].procNum == -1)
			{
				assign_minimal_unit(child, chosen[start].end_time);
			}
			assign_minimal_units_recursively(child);  // Rekurencyjne wywołanie dla dzieci dzieci
		}
	}
	void assign_minimal_unit(int task, int start_time)
	{
		int min_time = MAX;
		int index    = -1;
		for (int i = 0; i < numOfPE; i++)
		{
			int time = times[task][i];
			if (time < min_time)
			{
				min_time = time;
				index    = i;
			}
		}
		int assigned_unit = index;
		work_times[assigned_unit].push_back(std::vector<std::pair<int, int>>());
		int idx      = work_times[assigned_unit].size();
		chosen[task] = {assigned_unit,
						idx,
						times[task][assigned_unit],
						costs[task][assigned_unit],
						start_time,
						start_time + times[task][assigned_unit]};
	}
	Result find_critical_path(int start, int previous_times) {
		std::vector<int> children = getNeighbourIndices(start);

		if (children.empty()) {
			Result result;
			result.path.push_back(start);
			result.total_time = previous_times;
			return result;
		}

		Result longest_path_result;
		longest_path_result.total_time = 0;

		for (auto child : children) {
			int total_time = chosen[child].end_time;
			Result current_path_result = find_critical_path(child, total_time);
			int current_time = current_path_result.total_time;
			if (current_time > longest_path_result.total_time) {
				longest_path_result = current_path_result;
			}
		}

		longest_path_result.path.insert(longest_path_result.path.begin(), start);
		return longest_path_result;
	}
	int calculate_minimal_time()
	{
		assign_minimal_unit(0, 0);
		assign_minimal_units_recursively(0);
		int minimal_time = find_critical_path(0, chosen[0].time).total_time;
		clear();
		return minimal_time;
	}
};

int main()
{
	srand(time(nullptr));
	TaskGraph graph("graf.txt");
	int minimal_time = graph.calculate_minimal_time();
	std::cout << "Podaj maksymalny czas pracy systemu: " << std::endl;
	std::cin >> Tmax;

	while (Tmax < minimal_time) {
		std::cout << "Podano zbyt restrykcyjne ograniczenie czasowe, wprowadz nowa wartosc: " << std::endl;
		std::cin >> Tmax;
	}
	while (true) {
		int min_time_T0 = MAX;
		int min_index = 0;

		for (int i = 0; i < graph.numOfPE; i++)
		{
			// najmniejszy spośród uniwersalnych
			if (graph.times[0][i] < min_time_T0 && graph.procs[i][1] > 0)
			{
				min_time_T0 = graph.times[0][i];
				min_index = i;
			}
		}

		//first task allocation
		graph.chosen[0] = {
			min_index, 0, graph.times[0][min_index], graph.costs[0][min_index], 0, graph.times[0][min_index] };
		if (graph.is_PP(min_index))
		{
			graph.work_times[min_index].push_back(std::vector<std::pair<int, int>>());
			graph.work_times[min_index][0].push_back(std::pair<int, int>(0, graph.chosen[0].end_time));
		}
		total_cost += graph.procs[min_index][0];
		std::vector<int> children = graph.getNeighbourIndices(0);
		for (int child : children)
		{
			graph.parent_executed[child] = true;
		}

		//assign units
		graph.assign_units();

		int max_time = 0, max_time_idx = -1;
		for (int i = 0; i < graph.num_of_tasks; i++)
		{
			// obliczanie najdluzszego czasu wykonania zadania
			if (graph.chosen[i].end_time > max_time)
			{
				max_time = graph.chosen[i].end_time;
				max_time_idx = i;
			}
		}
		total_time = max_time;
		if (total_time <= Tmax) break;
		else {
			graph.clear();
			graph.standardized = false;
			graph.y *= graph.increaseFactor;
			graph.x *= graph.decreaseFactor;
			graph.z *= graph.decreaseFactor;
		}
	}
	for (int i = 0; i < graph.num_of_tasks; i++)
	{
		// obliczanie najdluzszego czasu wykonania zadania

		if (graph.is_PP(graph.chosen[i].procNum))
		{
			std::cout << "Zadanie T" << i << " wykonano na PP" << graph.chosen[i].procNum << "_"
				<< graph.chosen[i].procNumIndex << " w czasie " << graph.chosen[i].start_time << " - "
				<< graph.chosen[i].end_time << std::endl;
		}
		else
		{
			std::cout << "Zadanie T" << i << " wykonano na HC" << graph.chosen[i].procNum << "_"
				<< graph.chosen[i].procNumIndex << " w czasie " << graph.chosen[i].start_time << " - "
				<< graph.chosen[i].end_time << std::endl;
		}
	}
	std::cout << "Czas wykonania wszystkich zadan: " << total_time << std::endl;
	std::cout << "Calkowity koszt systemu: " << graph.calulate_total_tasks_cost();
	return 0;
}
