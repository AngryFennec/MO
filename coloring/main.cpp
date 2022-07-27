#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <random>
#include <utility>
#include <algorithm>
#include <unordered_set>
#include <time.h>
using namespace std;

struct Vertex
{
    int num;
    unordered_set<int> neibs;
};

class ColoringProblem
{
public:
    int GetRandom(int a, int b)
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
                colors.resize(vertices);
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

    }


    void SortByDegree() {
        sort(vertexes.begin(), vertexes.end(), [](const Vertex & a, const Vertex & b){ return a.neibs.size() > b.neibs.size(); });
    }

    void RemoveLargest() {
    	int v = vertexes[0].num;
    	vertexes.erase(vertexes.begin());
    	for (size_t i=0; i < vertexes.size(); i++) {
    	    vertexes[i].neibs.erase(v);
    	}
    }

    int FindColor(Vertex v) {
    	vector<int> used_colors;
    	vector<int> free_colors;
    	for (int neib: v.neibs) {
    		if (colors[neib] != -1) {
    			used_colors.push_back(colors[neib]);
    		}
    	}

    	for (int c = 1; c <= maxcolor; c++) {
    		int it = -1;
    		for (size_t k = 0; k < used_colors.size(); k++) {
    			if (c == used_colors[k]) {
    				it = k;
    			}
    		}
    		if (it == -1) {
    			free_colors.push_back(c);
    		}
    	}
    	if (free_colors.size() == 0) {
    		maxcolor++;
    		return maxcolor;
    	}

    	return free_colors[0];
    }




    bool Check()
    {
        for (size_t i = 0; i < init_vertexes.size(); ++i)
        {
            if (colors[init_vertexes[i].num] == -1)
            {
                cout << "Vertex " << i + 1 << " is not colored\n";
                return false;
            }
            for (int neighbour : init_vertexes[i].neibs)
            {
                if (colors[neighbour] == colors[init_vertexes[i].num])
                {
                    cout << "Neighbour vertices " << init_vertexes[i].num << ", " << neighbour <<  " have the same color\n";
                    return false;
                }
            }
        }
        return true;
    }

    int GetNumberOfColors()
    {
    	set<int> s(colors.begin(), colors.end());
        return s.size();
    }

    int GetFreeColors() {
    	int free_colors = 0;
    	for (size_t i = 0; i < colors.size(); ++i) {
    		if (colors[i] == -1) {
    			free_colors++;
    		}
    	}
    	return free_colors;
    }

    const vector<int>& GetColors()
    {
        return colors;
    }

    const string JoinColors() {
    	std::stringstream result;
    	set<int> s(colors.begin(), colors.end());
    	for (auto c: s) {
    		vector<int> class_vertexes;
    		for(size_t i = 0; i < colors.size(); ++i) {
    			if (colors[i] == c) {
    				class_vertexes.push_back(i+1);
    			}
    		}
    		result << "{";
    		for(size_t j = 0; j < class_vertexes.size(); ++j) {
    			result << class_vertexes[j];
    			if (j != class_vertexes.size()-1) {
    				 result << ",";
    			}
    		}
    		result << "},";
    	}
    	string res = result.str();
    	return res.substr(0, res.size()-1);
    }

    int GetVertexIndex(int num) {
        for (size_t i = 0; i < init_vertexes.size(); ++i) {
        	if (init_vertexes[i].num == num) {
        		return i;
        	}
        }
        return -1;
    }

    void Greedy() {
        for (size_t i = 0; i < colors.size(); ++i) {
        	colors[i] = -1;
        }
        while (GetFreeColors() != 0) {
        	SortByDegree();
        	Vertex current_vertex = vertexes[0];
        	int index = FindColor(init_vertexes[GetVertexIndex(current_vertex.num)]);
        	colors[current_vertex.num] = index;
        	RemoveLargest();
        }
    }

public:
    vector<int> colors;
    int maxcolor = 1;
    vector<unordered_set<int>> neighbour_sets;
    vector<Vertex> init_vertexes;
    vector<Vertex> vertexes;
};

int main()
{
    vector<string> files = { "myciel3.col", "myciel7.col", "school1.col", "school1_nsh.col",
        "anna.col", "miles1000.col", "miles1500.col", "le450_5a.col", "le450_15b.col",
        "queen11_11.col" };


    ofstream fout("color.csv");
    fout << "Instance; Colors; Time (sec); Colors\n";
    cout << "Instance; Colors; Time (sec); Colors\n";
    for (string file : files)
    {
        ColoringProblem problem;
        problem.ReadGraphFile("graphs/" + file);
        for (size_t i = 0; i <  problem.neighbour_sets.size(); ++i) {
        	Vertex vert;
        	vert.num = i;
        	vert.neibs =  problem.neighbour_sets[i];
        	problem.vertexes.push_back(vert);
        	problem.init_vertexes.push_back(vert);
        }

        clock_t start = clock();
        problem.Greedy();
        if (! problem.Check())
        {
            fout << "*** WARNING: incorrect coloring: ***\n";
            cout << "*** WARNING: incorrect coloring: ***\n";
        }
        fout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << "; " << problem.JoinColors() << '\n';
        cout << file << "; " << problem.GetNumberOfColors() << "; " << double(clock() - start) / 1000 << "; " << problem.JoinColors() << '\n';
    }
    fout.close();
    return 0;
}
