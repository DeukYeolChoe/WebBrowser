#include "validate.h"

using namespace std;

Validate::Validate()
{
}

int Validate::isURLValid(string content)
{
	string protocol;
	int foundS = 0;
	int foundE = 0;

	if (content.empty()) return -1;

	foundE = content.find("http://");
	if (foundE != 0) return -1;
	return 1;
}

int Validate::isHTMLValid(string content)
{
	// 열리고 닫히는것만 확인하자
	string lower = content;
	transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	lower.erase(remove(lower.begin(), lower.end(), '\t'), lower.end());
	lower.erase(remove(lower.begin(), lower.end(), '\r'), lower.end());
	lower.erase(remove(lower.begin(), lower.end(), '\n'), lower.end());

	int foundS = 0;
	int foundE = 0;
	int i = 0;
	bool opening = false;
	bool closing = false;
	string tag ="";
	string entire_tag = "";

	tag = "<!doctype html>";
	foundS = lower.find(tag);
	if (foundS >= 0)
		lower.erase(foundS, tag.length());

	//<html>가 있으면 반드시 </html>도 존재해야한다.
	for (i = 0; i < lower.length(); i++)
	{
		if (opening && !closing && lower[i] == '/')
		{
			if (entire_tag != "br" && entire_tag != "hr" && entire_tag != "input")
				return -1;
		}
		if (opening && !closing && lower[i] != '>' && lower[i] != '/')
		{
			entire_tag.append(lower.substr(i,1));
		}

		if (opening && closing)
		{
			//entire_tag에서 태그만 뽑아내야한다.
			foundE = entire_tag.find(" ");
			if (foundE < 0) tag = entire_tag;
			else
			{
				tag = entire_tag.substr(0, foundE);
			}
			if (tag != "br" && tag != "hr" && tag != "img" && tag != "input")
			{
				//</tag>가 없으면 리턴 -1 있으면 지워버리자
				if ((foundS = lower.find("</" + tag + ">")) < 0) return -1;
				else
				{
					//찾은 </tag>를 지우자
					string temp = "</" + tag + ">";
					lower.erase(foundS, temp.length());
				}
			}
			opening = false;
			closing = false;
		}

		if (lower[i] == '<')
		{
			foundS = i;
			opening = true;
			entire_tag = "";
		}
		if (lower[i] == '>') closing = true;
	}

	return 1;
}
