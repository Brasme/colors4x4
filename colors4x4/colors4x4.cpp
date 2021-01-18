// colors4x4.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <sstream>

static uint32_t Pack(const size_t *idx,size_t num)
{
	uint32_t val=0;
	for (size_t i = 0; i < 8 && i < num; ++i)
		val |= (idx[i] & 0xf) << (4 * i);
	return val;
}

static size_t Unpack(const size_t *idx, size_t num)
{
	uint32_t val = 0;
	for (size_t i = 0; i <8; ++i) {
		size_t match = 0;
		for (int m = 0; m < 8; ++m) {
			if (idx[m] == i)
				match = m;
		}
		val |= i << (4* match);
	}
	return val;
}

static uint32_t Reorder(uint32_t v, size_t idx)
{
	uint32_t val = 0;
	for (size_t i=0;i<8;++i) {
		size_t n = idx & 0x7;
		idx = idx >> 4;
		val |= ((v >> (4*n)) &0xf) << (4 * i);
	}
	return val;
}


uint32_t NextModulo(uint32_t hash, uint8_t m)
{
	if (m > 16) m = 16;
	for (auto i=0;i<8;++i) {
		const uint32_t mask = 0xf << (i * 4);
		auto v = (hash & mask) >> (i * 4);
		v = (v + 1) % m;
		hash = (hash & ~mask) | (v << (i * 4));
		if (v != 0)
			return hash;
	}
	return hash;
}

class Board
{
public:
	static const char nc;
	static const char* ch;

	explicit Board(uint32_t val32,size_t mod=3)
	{
		m_ = mod > 0x10 ? 0x10 : mod<2?2:static_cast<uint8_t>(mod);
		size_t i = 0;
		while (i < 8) {
			uint8_t v = val32 & 0xf;
			if (v >= m_)
				v = m_ - 1;
			v_[i] = ch[v];
			val32 = val32 >> 4;
			i++;
		}
		while (i < 8) {
			v_[i] = ch[i % m_];
			i++;
		}
	}

	explicit Board(const char* str=nullptr, size_t mod = 3)
	{ 
		m_ = mod > 8 ? 8 : mod < 2?2:static_cast<uint8_t>(mod);
		size_t i = 0;
		while (i < 8 && str && str[i]) {
			char val = str[i];
			if (val < nc) val = nc;
			else if (val > ch[m_-1]) val = ch[m_-1];
			v_[i] = val;
			i++;
		}
		while (i < 8) {
			v_[i] = ch[i % m_];
			i++;
		}
	}
	
	char At(size_t i) const { return i<8?v_[i]:'\0'; }
	char operator[](size_t i) const { return At(i); }

	explicit operator uint32_t() const
	{
		uint32_t v32 = 0;
		for (size_t i=0;i<8;++i) {
			v32 |= static_cast<uint32_t>(v_[i] - nc) << 4 * i;
		}
		return v32;
	}
	bool operator==(const Board& o) const { return operator uint32_t() == o.operator uint32_t(); }
	bool operator!=(const Board& o) const { return !operator ==(o); }

	std::string BoardStr() const {
		std::stringstream ss;
		ss << "+---+---+---+---+\n";
		ss << "+---+ " << At(C) << " +---+ " << At(E) << " +\n";
		ss << "+---+---+---+---+\n";
		ss << "+ " << At(D) << " +---+ " << At(F) << " +---+\n";
		ss << "+---+---+---+---+\n";
		ss << "+---+ " << At(G) << " +---+ " << At(A) << " +\n";
		ss << "+---+---+---+---+\n";
		ss << "+ " << At(H) << " +---+ " << At(B) << " +---+\n";
		ss << "+---+---+---+---+\n";
		return ss.str();
	}

	Board& Next() {
		for (size_t i = 0; i < 8; ++i) {
			if (NextV(i))
				return *this;			
		}
		return *this;
	}
	Board Next() const {
		Board b = *this;
		return b.Next();
	}

	bool NextValid() {		
		Board b = *this; 
		b.Next();
		while (b != *this) {
			if (b.IsValid()) {
				*this = b;
				return true;
			}
			b.Next();
		}
		return false;
	}


	
	bool NextTraverse(const Board &end)
	{
		static const size_t order_list[] = {  C,D,A,B,E,F,G,H };
		const size_t reorder = Pack(order_list, 8);
		const size_t back    = Unpack(order_list, 8);

		uint32_t hash = Reorder(operator uint32_t(),reorder);
		hash = Reorder(NextModulo(hash,m_), back);
		*this = Board(hash, m_);

		while (!IsValid() && !operator==(end))
		{
			hash = Reorder(operator uint32_t(), reorder);
			hash = Reorder(NextModulo(hash,m_), back);
			*this = Board(hash, m_);
		}
		while (IsValid() && !operator==(end))
			return true;

		return false;
	}

	static int ValidChar(char c)
	{
		for (int i = 0; i < 16; ++i)
			if (c == Board::ch[i])
				return i;
		return -1;
	}

	enum :size_t { A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H=7 };
	const char* Str() const {
		return reinterpret_cast<const char*>(&v_[0]);
	}
	bool IsValid() const
	{
		const bool bad= 
			v_[A]==v_[B] || v_[C]==v_[D] || v_[E]==v_[F] || v_[F]==v_[G] || 
			v_[G]==v_[H] || v_[A]==v_[F] || v_[C]==v_[F] || v_[D]==v_[G] || v_[G]==v_[B];
		return !bad;
	}
private:
	uint8_t m_;
	char v_[9]{};
	bool NextV(size_t idx) {
		char& v = v_[idx];
		v += 1;
		if (v >= (nc + m_))
		{
			v = nc;
			return false;
		}
		return true;
	}

};

const char Board::nc = '0';
const char* Board::ch = "0123456789ABCDEF";

int main(const int argc,char *argv[])
{
	Board  traverse_start_b;
	size_t traverse_start_idx=0;
	std::string last_line;

	const char* def = argc>1?argv[1]:"00000000";
	Board b(0x1234567, 8);
	std::cout << "Board b(0x1234567, 8) = \n" << b.BoardStr();
	b = Board(def, 3);
	std::cout << "Board(" << def << ",3) = " << b.Str() << " (valid=" << (b.IsValid()?"Yes":"No") << ") =\n" << b.BoardStr() << "\n";

	std::cout << "Press q+enter to exit\n%> ";
	bool done = false;
	for (std::string line; !done && std::getline(std::cin, line);) 
	{		
		if (line.empty())
			line = last_line;
		
		if (!done && line == "h") {
			std::cout <<
				"Help info:\n" <<
				"    Start application with argument with start pattern, modulo = max of pattern\n" <<
				" q : quit\n" <<
				" n : next (ignore valid)\n" <<
				" N : next (valid)\n" <<
				" V : Scan valid boards\n" <<
				" <pattern> : A combination of 0-9,A-F\n" <<
				" t : Traverse another order (ignore valid)\n" <<
				" T : Traverse another order (valid)\n" <<
				" s : Show simple board info\n" <<
				" S : Show extended board\n";
		}
		else {
			last_line = line;
		}
		std::cout << "Handle: \"" << line << "\"\n%> ";
		done = line == "q";

		
		if (!done && line == "n") {
			b.Next();
			line = "s";
		}
		if (!done && line == "N") {
			b.NextValid();
			line = "s";
		}

		if (!done && line == "V") {
			auto i = 0;
			auto start = b;
			if (!start.IsValid())
				start.NextValid();
			Board scan = start;
			std::cout << "Scan valid boards:\n";
			do
			{
				i++;
				std::cout << "Valid," << scan.Str() << "," << i << "\n";				
			} while (scan.NextValid() && scan != start);
			std::cout << "Found " << i << " combinations\n";
		}

		if (!done && Board::ValidChar(line[0])>=0) {
			int max = 2;
			const char* s = line.c_str();
			for (int i = 0; const int idx = Board::ValidChar(s[i]) >= 0;++i)
				if (idx > max) max = s[i];
			size_t mod = static_cast<size_t>(max)+1;
			if (mod < 2) mod = 2;
			b = Board(line.c_str(),mod);
			line = "S";
		}

		if (!done && line == "t") {
			if (traverse_start_idx == 0) {
				while (!b.IsValid())
					b.NextTraverse(b);
				traverse_start_b = b;
				std::cout << "Traverse first match\n";
				traverse_start_idx++;
			} else {
				b.NextTraverse(traverse_start_b);
				while (!b.IsValid())
					b.NextTraverse(traverse_start_b);

				if (b == traverse_start_b) {
					std::cout << "Traverse next complete\n";
					traverse_start_idx = 0;
				}
				else {
					traverse_start_idx++;
					std::cout << "Traverse next : found match " << traverse_start_idx << "\n";
				}
			}			
			line = "S";
		}

		if (!done && line == "T") {
			int i=0;
			std::cout << "Traverse boards:\n";
			Board start = b;
			if (!start.IsValid())
				start.NextValid();
			Board scan = start;
			do {
				i++;
				std::cout << "Valid," << scan.Str() << "," << i << "\n";				
			} while (scan.NextTraverse(start));
			std::cout << "Found " << i << " combinations\n";
		}


		if (!done && line == "h") {
			const uint32_t v = static_cast<uint32_t>(b);
			std::cout << "Board = 0x" << std::hex << v << " // valid=" << (b.IsValid() ? "Yes" : "No") << "\n";
		}
		if (!done && line == "s")
			std::cout << "Board = " << b.Str() << " // valid=" << (b.IsValid() ? "Yes" : "No") << "\n";
		if (!done && line == "S")
			std::cout << "Board = // valid=" << (b.IsValid() ? "Yes" : "No") << "\n" << b.BoardStr() << "\n";

		
	}
	return 0;
}

