#include "parser.hpp"
#include <iostream>
#include <climits>
#include <LEDA/graph/graph.h>
#include <LEDA/graph/min_cut.h>
#include <LEDA/graph/min_cost_flow.h>
using namespace std;

int main(int argc, char *argv[])
{
	Parser parser;
	parser.parse(argv);
	parser.first_stage();
	//parser.traverseGraph();
	parser.labeling();
	parser.mapping(argv);

	return 0;
}
