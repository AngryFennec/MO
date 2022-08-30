#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <random>
#include <unordered_set>
#include <algorithm>
#include <deque>
using namespace std;

struct Vertex
{
    int num;
    unordered_set<int> neibs;
};

class MaxCliqueTabuSearch
{
public:
    static int GetRandom(int a, int b)
    {
        static mt19937 generator;
        uniform_int_distribution<int> uniform(a, b);
        return uniform(generator);
    }

    void ReadGraphFile(string filename)
    {
        ifstream fin(filename);
        string line;
        int vertices = 0, edges = 0;
        while (getline(fin, line))
        {
            if (line[0] == 'c')
            {
                continue;
            }

            stringstream line_input(line);
            char command;
            if (line[0] == 'p')
            {
                string type;
                line_input >> command >> type >> vertices >> edges;
                neighbour_sets.resize(vertices);
                qco.resize(vertices);
                tau.resize(vertices);
                index.resize(vertices, -1);
                non_neighbours.resize(vertices);
            }
            else
            {
                int start, finish;
                line_input >> command >> start >> finish;
                // Edges in DIMACS file can be repeated, but it is not a problem for our sets
                neighbour_sets[start - 1].insert(finish - 1);
                neighbour_sets[finish - 1].insert(start - 1);
            }
        }
        for (int i = 0; i < vertices; ++i)
        {
            for (int j = 0; j < vertices; ++j)
            {
                if (neighbour_sets[i].count(j) == 0 && i != j)
                    non_neighbours[i].insert(j);
            }
        }
    }

    void RunSearch(int starts, int randomization)
    {
        for (int iter = 0; iter < starts; ++iter)
        {
            ClearClique();
            for (size_t i = 0; i < neighbour_sets.size(); ++i)
            {
                qco[i] = i;
                index[i] = i;
            }
            FindClique(randomization, 5);
            c_border = q_border;
            int swaps = 0;
            while (swaps < 100)
            {
                if (! Move())
                {
                    if (! Swap1To1())
                    {
                        break;
                    }
                    else
                    {
                        ++swaps;
                    }
                }
            }
            if (q_border > best_clique.size())
            {
                best_clique.clear();
                for (int i = 0; i < q_border; ++i)
                    best_clique.push_back(qco[i]);
            }
        }
    }

    const vector<int>& GetClique()
    {
        return best_clique;
    }

    bool Check()
    {
        for (int i : best_clique)
        {
            for (int j : best_clique)
            {
                if (i != j && neighbour_sets[i].count(j) == 0)
                {
                    cout << "Returned subgraph is not clique\n";
                    return false;
                }
            }
        }
        return true;
    }

    void ClearClique()
    {
        q_border = 0;
        c_border = 0;
        for (size_t i = 0; i < neighbour_sets.size(); ++i) {
        	qco[i] = i;
        	tau[i] = 0;
        	index[i] = i;
        }
    }

private:
    const int max_tabu_size = 50;
    vector<unordered_set<int>> non_neighbours;
    vector<int> qco;
    vector<int> tau;
    vector<int> index;
    int q_border = 0;
    int c_border = 0;
    deque<int> added_tabu_list;
    deque<int> removed_tabu_list;



    void SwapVertices(int vertex, int border)
    {
        int vertex_at_border = qco[border];
        swap(qco[index[vertex]], qco[border]);
        swap(index[vertex], index[vertex_at_border]);
    }

    void InsertToClique(int i)
    {
        for (int j : non_neighbours[i])
        {
            if (tau[j] == 0)
            {
                --c_border;
                SwapVertices(j, c_border);
            }
            tau[j]++;
        }
        SwapVertices(i, q_border);
        ++q_border;
    }

    void RemoveFromClique(int k)
    {
        for (int j : non_neighbours[k])
        {
            if (tau[j] == 1)
            {
                SwapVertices(j, c_border);
                c_border++;
            }
            tau[j]--;
        }
        --q_border;
        SwapVertices(k, q_border);
    }

   void insertToTabuAdded(int vertex)
   {
       added_tabu_list.push_back(vertex);
       if (added_tabu_list.size() > max_tabu_size)
           added_tabu_list.pop_front();
   }

   void insertToTabuRemoved(int vertex)
      {
          removed_tabu_list.push_back(vertex);
          if (removed_tabu_list.size() > max_tabu_size)
              removed_tabu_list.pop_front();
      }

   bool Swap1To1() {
        for (int counter = 0; counter < q_border; ++counter) {
            int vertex = qco[counter];
            if (find(added_tabu_list.begin(), added_tabu_list.end(), vertex) == added_tabu_list.end()) {

            	vector<int> L;
            	for (int i : non_neighbours[vertex]) {
            		if (tau[i] == 1)
            			if (find(removed_tabu_list.begin(), removed_tabu_list.end(), vertex) != removed_tabu_list.end())
            				L.push_back(i);
            		if (L.size() != 0) {

            			RemoveFromClique(vertex);
            			insertToTabuRemoved(vertex);
            			int rnd = GetRandom(0, L.size() - 1);
            			insertToTabuAdded(L[rnd]);
            			InsertToClique(L[rnd]);

            			return true;
            		}
            	}
            }
        }
        return false;
    }

    bool Move()
    {
        if (c_border == q_border)
            return false;
        int vertex = qco[GetRandom(q_border, c_border - 1)];
        InsertToClique(vertex);
        return true;
    }

    void FindClique(int randomization, int iterations)
        {
            static mt19937 generator;
            //sorting by degree
            sort(vertexes.begin(), vertexes.end(), [](const Vertex & a, const Vertex & b){ return a.neibs.size() > b.neibs.size(); });
            for (int iteration = 0; iteration < iterations; ++iteration)
            {
                vector<int> clique;
                vector<Vertex> candidates;
                for (size_t i = 0; i < vertexes.size(); ++i)
                {
                    candidates.push_back(vertexes[i]);
                }
                while (!candidates.empty())
                {
                    int last = candidates.size() - 1;
                    int rnd = GetRandom(0, min(randomization - 1, last));
                    Vertex vertex = candidates[rnd];
                    clique.push_back(vertex.num);
                    for (int c = 0; c < candidates.size(); ++c)
                    {
                        int candidate = candidates[c].num;
                        if (vertex.neibs.count(candidate) == 0)
                        {
                        	candidates.erase(candidates.begin() + c);
                            --c;
                        }
                    }
                }
                if (clique.size() > best_clique.size())
                {
                    best_clique = clique;
                }
            }
        }


	public:
    	vector<unordered_set<int>> neighbour_sets;
        vector<int> best_clique;
    	vector<Vertex> init_vertexes;
    	vector<Vertex> vertexes;

    	const string JoinClique() {
    		stringstream result;
    		for(size_t i = 0; i < best_clique.size(); ++i) {
    			if(i != 0)
    				result << ",";
    			result << best_clique[i];
    		}
    		return result.str();
    	}
};


int main()
{
    int iterations;
    cout << "Number of iterations: ";
    cin >> iterations;
    int randomization;
    cout << "Randomization: ";
    cin >> randomization;
    vector<string> files = { "C125.9.clq", "johnson8-2-4.clq", "johnson16-2-4.clq", "MANN_a9.clq", "MANN_a27.clq",
        "p_hat1000-1.clq", "keller4.clq", "hamming8-4.clq", "brock200_1.clq", "brock200_2.clq", "brock200_3.clq", "brock200_4.clq",
        "gen200_p0.9_44.clq", "gen200_p0.9_55.clq", "brock400_1.clq", "brock400_2.clq", "brock400_3.clq", "brock400_4.clq",
        "sanr400_0.7.clq", "p_hat1000-2.clq", "p_hat500-3.clq", "p_hat1500-1.clq", "p_hat300-3.clq", "san1000.clq",
        "sanr200_0.9.clq" };
    ofstream fout("clique_tabu.csv");
    fout << "File; Clique; Time (sec)\n";
    for (string file : files)
    {
    	cout << file << endl;
        MaxCliqueTabuSearch problem;
        problem.ReadGraphFile("graphs/" + file);
        for (size_t i = 0; i <  problem.neighbour_sets.size(); ++i) {
        	Vertex vert;
        	vert.num = i;
        	vert.neibs =  problem.neighbour_sets[i];
        	problem.vertexes.push_back(vert);
        	problem.init_vertexes.push_back(vert);
        }
        clock_t start = clock();
        problem.RunSearch(iterations, randomization);
        if (!problem.Check())
        {
            cout << "*** WARNING: incorrect clique ***\n";
            fout << "*** WARNING: incorrect clique ***\n";
        }
        fout << file << "; " << problem.GetClique().size() << "; " << double(clock() - start) / 1000  << " " << problem.JoinClique() << '\n';
        cout << file << ", result - " << problem.GetClique().size() << ", time - " << double(clock() - start) / 1000 << '\n';
    }
    fout.close();
    return 0;
}
