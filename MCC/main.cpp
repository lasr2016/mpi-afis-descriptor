
#include <iostream>
#include "MCC.h"
#include <string.h>

#define MAX_MINUCIAS 96

using namespace std;

int main(int argc, char * argv[])
{
//	MCC a2;
	int num_minucias, i, j, num;

	if(argc < 2){
			cout << "Usage: MCC <fingerprint1>  -N {8|16} -C {LSS|LSSR|LSA|LSAR|LGS|NHS} [-H] [-B]" << endl;
			return 0;
	}

	MCC::configureAlgorithm(argc, argv);

	scanf("%d", &num_minucias);

	Matrix<int> M_xyt(num_minucias, 3);

	for (i=0; i<num_minucias; i++)
	{
		for (j=0; j<4; j++)
		{
			scanf("%d", &num);
			if (j < 3)
				M_xyt[i][j] = num;
		}
	}

	MCC a1(M_xyt);


//    M_xyt.resize(new_rows, new_columns);
/*
	if(a1.readFile(argv[1])!=0){
			cout << "Error opening fingerprint files: " << argv[1] << endl;
			return 0;
	}
	if(a2.readFile(argv[2])!=0){
			cout << "Error opening fingerprint files: " << argv[2] << endl;
			return 0;
	}
*/
	//Calculando el descriptor MCC
	a1.initialize();
	//a2.initialize();
	
	//cout << "First fingerprint: " << endl;
	a1.printCylinders(cout);

	//cout << a1.match(a2) << endl;

	return 0;
}
