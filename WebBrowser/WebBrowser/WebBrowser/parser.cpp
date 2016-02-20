#include "parser.h"

Parser::Parser()
{
}

string Parser::getValueFromHTMLTag(string content)
{
	string plainText = content;
	//http://www.cplusplus.com/forum/beginner/134420/
	const regex re(R"(\s*<([A-Z][A-Z0-9]*)\b[^>]*>(.*?)</\1>\s*)", regex::icase);
	
	smatch results;
	if (std::regex_match(plainText, results, re))
	{
		cout << "tag: '" << results[1] // first sub-match - for ([A-Z][A-Z0-9]*)
			<< "'\tvalue: '" << results[2] // second sub-match - for (.*?)
			<< "'\n";
	}

	return results[2];
}

char* Parser::removeHTMLTag(char* content)
{
	char plainText[SIZE];
	int n = 0;
	int i = 0;
	bool isSaved = false;
	char temp[2];

	//plainText = (char *)malloc(sizeof(char *) * strlen(content));
	memset(&plainText, '\0', sizeof(plainText));
	
	while (strlen(content) > n)
	{
		if (content[n] == '<')
			isSaved = false;
			
		if (content[n] == '>' && strlen(content) > (n+1))
		{
			isSaved = true;
			n++;
		}
		if (isSaved)
		{
			temp[0] = content[n];
			temp[1] = '\0';
			strcat(plainText, temp);
			i++;
		}
		n++;
	}
	
	return plainText;
}
