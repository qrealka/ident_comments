// IdentComments.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <iterator>
#include <string>

using namespace std;

typedef string::const_iterator ParserPosition;

class ParserState
{
public:
	ParserState() : m_needEndOfLine(false), m_firstLine(false), m_phase(&ParserState::StartPhase)
	{}

	void BeforeParseLine()
	{
		m_needEndOfLine = false;
	}

	ParserPosition Parse(ParserPosition begin, ParserPosition end)
	{
		return (*this.*m_phase)(begin, end);
	}
	void AfterParseLine()
	{
		if (m_needEndOfLine)
			cout << endl;
	}

private:
	typedef ParserPosition(ParserState::*ParserPhase)(ParserPosition begin, ParserPosition end);

	ParserPosition StartPhase(ParserPosition begin, ParserPosition end)
	{
		m_firstLine = false;

		for (; begin != end; ++begin)
		{
			switch (*begin)
			{
			case '"':
				m_phase = &ParserState::StringPhase;
				return ++begin;
			case '/':
				if (++begin == end)
					return begin;
				if (*begin == '/')
				{
					m_phase = &ParserState::OneLineCommentPhase;
					return --begin;
				} 
				if (*begin == '*')
				{
					m_firstLine = true;
					m_phase = &ParserState::MultilineCommentPhase;
					return --begin;
				}
				break;
			}
		}
		return end;
	}

	ParserPosition StringPhase(ParserPosition begin, ParserPosition end)
	{
		begin = find(begin, end, '"');
		if (begin != end)
		{
			m_phase = &ParserState::StartPhase;
			return ++begin;
		}
		return begin; // multiline string const
	}

	ParserPosition OneLineCommentPhase(ParserPosition begin, ParserPosition end)
	{
		m_needEndOfLine = true;
		copy(begin, end, ostream_iterator<char>(cout));
		m_phase = &ParserState::StartPhase;
		return end;
	}

	ParserPosition MultilineCommentPhase(ParserPosition begin, ParserPosition end)
	{
		m_needEndOfLine = true;
		for (auto needTrim = !m_firstLine; begin != end; ++begin)
		{
			if (!isspace(*begin) || !needTrim)
			{
				needTrim = false;
				cout << *begin;
			}

			if (*begin == '*')
			{
				if (end != begin+1 && *(begin+1) == '/')
				{
					cout << *(++begin);
					m_phase = &ParserState::StartPhase;
					return ++begin;
				}
			}
		}
		m_firstLine = false;
		return end;
	}

private:
	bool m_needEndOfLine;
	bool m_firstLine;
	ParserPhase m_phase;
};



int _tmain(int argc, _TCHAR* argv[])
{
	ParserState parser;
	for (string line; getline(cin, line);)
	{
		parser.BeforeParseLine();
		for (auto position = line.cbegin(); position != line.cend(); position = parser.Parse(position, line.cend()));
		parser.AfterParseLine();
	}
	return 0;
}

