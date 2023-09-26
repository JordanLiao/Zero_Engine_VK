#ifndef _COLORID_H_
#define _COLORID_H_

class ColorID {
private:
	static int counter;
public:
	//need to add mutex support!
	static int getNewId();
};

#endif