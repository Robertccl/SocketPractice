#include "MyUtil.h"

#include <iostream>
using namespace std;




void Util::HandleError(string error_msg)
{
	if (error_msg != "")
		cout << error_msg << endl;
}