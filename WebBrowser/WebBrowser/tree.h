#ifndef DOMTREE_H
#define DOMTREE_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <stack>
#include <vector>

using namespace std;

#define BUFSIZE 1024

struct ImgAttr {
	wstring src;
	wstring width;
	wstring height;
};

struct FormAttr {
	wstring action;
	wstring method;
};

struct InputAttr {
	wstring type;
	wstring name;
	wstring value;
};

struct FontAttr {
	wstring size;
	wstring weight;
	wstring style;
	bool underscore;
};

struct Attribute {
	ImgAttr img;
	FormAttr form;
	InputAttr input;
	FontAttr font;
};

struct Node {
	wstring tag;
	wstring content;
	Attribute attributes;
	vector<Node> children;
};

class Tree {
	public:
		Tree();
		Node createTree(wstring htmlcontents);
		wstring removeHttpHeader(wstring contents);
		wstring removeComments(wstring page);
		wstring removeCDATA(wstring contents);
};
#endif