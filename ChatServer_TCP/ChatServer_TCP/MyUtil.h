#ifndef __MYUTIL__H
#define __MTUTIL__H

#include <string>
using namespace std;

class Util
{
private:
	Util() {};

	Util(const Util&);

	const Util& operator=(const Util&);


public:
	static Util& GetInstance()
	{
		static Util onlyUtil;
		return onlyUtil;

	}

public:
	void HandleError(string error_msg);
};


#endif // !

