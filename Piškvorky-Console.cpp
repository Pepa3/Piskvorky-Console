// Piškvorky-Console.cpp

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <time.h>

using namespace std;

class Board {
public:

	enum class BoardState {
		NONE, X, O
	};

	BoardState player;
	const unsigned PIECES_FOR_WIN = 5;

	Board(int w, int h, BoardState defval=BoardState(0))	{
		state = vector<vector<BoardState>>(w,vector<BoardState>(h,defval));
		center = {w/2, h/2};
		player = BoardState(1);
	}
	Board(BoardState defval) : Board(15,15,defval){}
	Board() : Board(BoardState(0)) {}

	~Board() {
		state.~vector();
		center.~pair();
		player.~BoardState();
	}

	BoardState getState(int x, int y) {
		if (x<0 || y<0 || x>state.size() || y> state[0].size()) return BoardState::NONE;
		return state.at(x).at(y);
	}

	bool setState(BoardState s, int x, int y) {
		if (state[x][y] == BoardState::NONE) {
			state[x][y] = s;
		} else {
			return false;
		}
		return true;
	}

	static char s2c(BoardState stat) {
		switch (stat){
		case Board::BoardState::NONE:
			return '.';
		case Board::BoardState::X:
			return 'X';
		case Board::BoardState::O:
			return 'O';
		default:
			unexpected();
		}
	}

	pair<int, int> getCenter() { return center; }
	void setCenter(pair<int, int> c) { center = c; }

	string window(unsigned w, unsigned h) {
		int x = center.first - w/2;
		int y = center.second - h/2;
		string ret = "";
		ret.append("   0 1 2 3 4 5 6 7 8 9 1011121314\n");
		for (size_t i = 0; i < w; i++) {
			char* str = new char[4];
			sprintf_s(str,4,"%3d", i);
			ret.append(str);
			for (size_t j = 0; j < h; j++) {
				ret.append({ s2c(state[x+i][y+j])});
				ret.append(" ");
			}
			ret.append("\n");
		}
		return ret;
	}
	bool checkEndgame(int x, int y) {
		BoardState cur = state[x][y];
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		unsigned dx = 1, dy = 1, dxy = 1, dmxy = 1;
		for (size_t i = 1; i < PIECES_FOR_WIN; i++){//px = ->; py = v
			pxy  &= (getState(x + i,y    ) == cur);
			pxpy &= (getState(x + i,y + i) == cur);
			xpy  &= (getState(x    ,y + i) == cur);
			mxpy &= (getState(x - i,y + i) == cur);
			mxy  &= (getState(x - i,y    ) == cur);
			mxmy &= (getState(x - i,y - i) == cur);
			xmy  &= (getState(x    ,y - i) == cur);
			pxmy &= (getState(x + i,y - i) == cur);
			if (pxy || mxy) dx++;
			if (xpy || xmy) dy++;
			if (pxpy || mxmy) dxy++;
			if (pxmy || mxpy) dmxy++;
		}
		return dx>= PIECES_FOR_WIN || dy>= PIECES_FOR_WIN || dxy>= PIECES_FOR_WIN || dmxy>= PIECES_FOR_WIN;
	}
//	/*does the best move*/
//	void bot() {
//		int x = rand() % (center.first * 2) - 1;
//		int y = rand() % (center.second * 2) - 1;
//		setState(player, x, y);
//	}

private:
	vector<vector<BoardState>> state;
	pair<int, int> center;
};

int main() {
	srand(time(NULL));
	Board board = Board();
	int ww = 15, wh = 15;

	string in;
	bool endgame = false;
	while(!endgame) {
		printf("Player %c is playing\n", board.s2c(board.player));
		cout << board.window(ww, wh);
		printf("Enter position('Y X'): ");
		getline(cin, in);
		int del = in.find(" ");
		std::string tok1 = in.substr(0, del);
		std::string tok2 = in.substr(del + 1, in.length());
		int a = -1, b = -1;
		try	{
			a = stoi(tok1);
			b = stoi(tok2);
		} catch(const exception&) {
			if (in.compare("end") == 0) endgame = true;
//			if (in.compare("bot") == 0) board.bot();//TODO:
			printf("Error, please try again\n");
			continue;
		}
		int x = board.getCenter().first - ww / 2;
		int y = board.getCenter().second - wh / 2;
		if (x + a >= ww || y + b >= wh) { printf("Position %d %d doesn't exist\n",a,b); continue; }
		if (!board.setState(board.player, x + a, y + b)) { printf("Position already has a piece! Try again.\n"); continue; }
		if (board.checkEndgame(x+a,y+b)) {
			printf("Player %c wins!\n",board.s2c(board.player));
			endgame = true;
			cout << board.window(ww, wh);
			break;
		}
		board.player = Board::BoardState((uint8_t)board.player % 2 + 1);
		printf("Got position: %d %d\n",a,b);

	}

	board.~Board();
}
