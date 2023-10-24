// Piškvorky-Console.cpp

#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <time.h>
#include <chrono>

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

	BoardState getState(unsigned x, unsigned y) {
		if (x<0 || y<0 || x>state.size()-1 || y> state[0].size()-1) return BoardState::NONE;
		return state[x][y];
	}

	bool isX(unsigned x, unsigned y) { return getState(x, y) == BoardState::X; }
	bool isO(unsigned x, unsigned y) { return getState(x, y) == BoardState::O; }
	bool isXO(unsigned x, unsigned y) { return getState(x, y) != BoardState::NONE; }
	BoardState n(BoardState a) {
		if (a == BoardState::NONE) throw exception();
		return (a == BoardState::X) ? BoardState::O : BoardState::X;
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

	unsigned evalPoint(int x, int y) {
		BoardState cur = getState(x,y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		unsigned dx = 1, dy = 1, dxy = 1, dmxy = 1;
		for (size_t i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
			pxy &= (getState(x + i, y) == cur);
			pxpy &= (getState(x + i, y + i) == cur);
			xpy &= (getState(x, y + i) == cur);
			mxpy &= (getState(x - i, y + i) == cur);
			mxy &= (getState(x - i, y) == cur);
			mxmy &= (getState(x - i, y - i) == cur);
			xmy &= (getState(x, y - i) == cur);
			pxmy &= (getState(x + i, y - i) == cur);
			if (pxy) dx++;
			if (mxy) dx++;
			if (xmy) dy++;
			if (xpy) dy++;
			if (pxpy) dxy++;
			if (mxmy) dxy++;
			if (pxmy) dmxy++;
			if (mxpy) dmxy++;
		}
		return max(max(dx,dy), max(dxy, dmxy));
	}
	//Black(X) = + ; White(O) = -
	signed evalPoint1(int x, int y) {
		switch (getState(x,y)){
		case BoardState::NONE:
			return 0;
		case BoardState::X:
			return evalPoint(x, y);
		case BoardState::O:
			return -(signed)evalPoint(x, y);
		}
		throw exception();
	}
	/*
		. . ? . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . ? . . .
	*/
	pair<unsigned,pair<BoardState,BoardState>> evalLineBlocked(int x, int y) {
		BoardState cur = getState(x, y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		BoardState bpxy = cur, bpxpy = cur, bxpy = cur, bmxpy = cur, bmxy = cur, bmxmy = cur, bxmy = cur, bpxmy = cur;
		signed dx = 1, dy = 1, dxy = 1, dmxy = 1, dmx = 1, dmy = 1, dxmy = 1, dmxmy = 1;
		for (size_t i = 1; i <= 5; i++) {//px = ->; py = v
			pxy  &= (getState(x + i, y    ) == cur);
			pxpy &= (getState(x + i, y + i) == cur);
			xpy  &= (getState(x    , y + i) == cur);
			mxpy &= (getState(x - i, y + i) == cur);
			mxy  &= (getState(x - i, y    ) == cur);
			mxmy &= (getState(x - i, y - i) == cur);
			xmy  &= (getState(x    , y - i) == cur);
			pxmy &= (getState(x + i, y - i) == cur);
			if (!pxy  && bpxy == cur)  bpxy  = getState(x + i, y    );
			if (!pxpy && bpxpy == cur) bpxpy = getState(x + i, y + i);
			if (!xpy  && bxpy == cur)  bxpy  = getState(x    , y + i);
			if (!mxpy && bmxpy == cur) bmxpy = getState(x - i, y + i);
			if (!mxy  && bmxy == cur)  bmxy  = getState(x - i, y    );
			if (!mxmy && bmxmy == cur) bmxmy = getState(x - i, y - i);
			if (!xmy  && bxmy == cur)  bxmy  = getState(x    , y - i);
			if (!pxmy && bpxmy == cur) bpxmy = getState(x + i, y - i);
			if (pxy) dx++;
			if (mxy) dmx++;
			if (xmy) dy++;
			if (xpy) dmy++;
			if (pxpy) dxy++;
			if (mxmy) dxmy++;
			if (pxmy) dmxy++;
			if (mxpy) dmxmy++;
		}
		int mmax = max(max(dx+dmx,dy+dmy), max(dxy+dxmy,dmxy+dmxmy));
		pair<BoardState, BoardState> ret = {cur,cur};
		int bestb = 0;
		if ((dx + dmx) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			int bestb2 = 0;
			if (bpxy==BoardState::NONE) { ret2.first = BoardState::NONE; bestb2++; }
			if (bmxy==BoardState::NONE) { ret2.second = BoardState::NONE; bestb2++; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; }
		}
		if ((dy + dmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			int bestb2 = 0;
			if (bxmy == BoardState::NONE) { ret2.first = BoardState::NONE; bestb2++; }
			if (bxpy == BoardState::NONE) { ret2.second = BoardState::NONE; bestb2++; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; }
		}
		if ((dxy + dxmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			int bestb2 = 0;
			if (bpxpy == BoardState::NONE) { ret2.first = BoardState::NONE; bestb2++; }
			if (bmxmy == BoardState::NONE) { ret2.second = BoardState::NONE; bestb2++; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; }
		}
		if ((dmxy + dmxmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			int bestb2 = 0;
			if (bpxmy == BoardState::NONE) { ret2.first = BoardState::NONE; bestb2++; }
			if (bmxpy == BoardState::NONE) { ret2.second = BoardState::NONE; bestb2++; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; }
			printf("/:%d %c %c\n", bestb2, s2c(ret2.first), s2c(ret2.second));
		}



		return { bestb,ret };

	}

	signed evalBoard() {
		signed score = 0;
		for (size_t i = 0; i < state.size(); i++){
			for (size_t j = 0; j < state[0].size(); j++){
				signed tmp = evalPoint1(i, j);
				if (tmp >= (signed)PIECES_FOR_WIN) { return 2561; }
				if (tmp <= -(signed)PIECES_FOR_WIN) { return -2561; }
				score += tmp;
			}
		}
		return score;
	}

	bool hasNeighbor(int x, int y) {
		if (getState(x - 1, y) != BoardState::NONE || getState(x + 1, y) != BoardState::NONE ||
			getState(x, y - 1) != BoardState::NONE || getState(x, y + 1) != BoardState::NONE ||
			getState(x - 1, y + 1) != BoardState::NONE || getState(x + 1, y + 1) != BoardState::NONE ||
			getState(x - 1, y - 1) != BoardState::NONE || getState(x + 1, y - 1) != BoardState::NONE) {
			return true;
		}
		return false;
	}

	bool checkX(int x, int y, int a) {return evalPoint(x, y) >= a;}
	bool checkEndgame(int x, int y) {return evalPoint(x, y) >= PIECES_FOR_WIN;}

	pair<signed, pair<int, int>> bot() {
		pair<signed, pair<int, int>> ret = { 0, {-1, -1} };
		vector<pair<int, int>> c4 = vector<pair<int, int>>();
		for (size_t x = 0; x < state.size(); x++){
			for (size_t y = 0; y < state[0].size(); y++){
				if (getState(x, y) == BoardState::NONE) continue;
				if (checkX(x, y, 4)) c4.push_back({x,y});
			}
		}
		for (pair<int,int> c : c4){
			//Našli jsme 4 kameny stejné barvy za sebou, jsou zablokované z obou stran?
			evalLineBlocked(c.first, c.second);
		}

		return ret;
	}
	

private:
	vector<vector<BoardState>> state;
	pair<int, int> center;
};

int main() {
	srand((unsigned)time(NULL));
	Board board = Board();
	int ww = 15, wh = 15;

	string in;
	bool endgame = false;
	while(!endgame) {
		printf("Player %c is playing\n", board.s2c(board.player));
		cout << board.window(ww, wh);
		printf("Score: %d\n", board.evalBoard());
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
			if (in.compare("bot") == 0) {
				pair<signed,pair<int, int>> result = board.bot();
				printf("minmax X%d Y%d\n",result.second.first,result.second.second);
			}
			/*if (in.compare("time") == 0) {
				auto t1 = chrono::high_resolution_clock::now();
				board.minimax(&board,4);
				auto t2 = chrono::high_resolution_clock::now();
				cout << "minimax 4 takes " << chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
			}*/
			printf("Error, please try again\n");
			continue;
		}
		int x = board.getCenter().first - ww / 2;
		int y = board.getCenter().second - wh / 2;
		if (x + a >= ww || y + b >= wh) { printf("Position %d %d doesn't exist\n",a,b); continue; }
		if (!board.setState(board.player, x + a, y + b)) {
			auto tmp = board.evalLineBlocked(x + a, y + b);
			printf("Position already has a piece! (%d),(%c),(%c).\n", tmp.first,board.s2c(tmp.second.first),board.s2c(tmp.second.second));
			continue;
		}
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

/*pair<signed, pair<int, int>> minimax(Board* b, signed depth) {
		if (depth <= 0) { return { (*b).evalBoard(), { -1,-1 } }; }
		else if(depth>2){ printf("%d", depth); }
		if ((*b).player == BoardState::X) {
			signed score = -2561;
			pair<int, int> best = { -1,-1 };
			for (size_t i = 0; i < (*b).state.size(); i++) {
				for (size_t j = 0; j < (*b).state[0].size(); j++) {
					if (!hasNeighbor(i, j)) { continue; }
					Board b2 = *b;
					if (!b2.setState(b2.player, i, j)) continue;
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					int tmp = minimax(&b2, depth-1).first;
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					if (tmp > score) {
						score = tmp;
						best = { i,j };
						if (tmp >= 2500) { return { score,best }; }
					}
				}
			}
			return { score, best};
		}else if ((*b).player == BoardState::O) {
			signed score = 2561;
			pair<int, int> best = { -1,-1 };
			for (size_t i = 0; i < (*b).state.size(); i++) {
				for (size_t j = 0; j < (*b).state[0].size(); j++) {
					if (!hasNeighbor(i, j)) { continue; }
					Board b2 = *b;
					if(!b2.setState(b2.player, i, j)) continue;
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					int tmp = minimax(&b2, depth - 1).first;
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					if (tmp < score) {
						score = tmp;
						best = { i,j };
						if (tmp <= -2500) { return { score,best }; }
					}
					//score = min(score, minimax(&b2, depth-1));
				}
			}
			return { score, best};
		}
		return { 0, { -1, -1 } };
	}*/
	/*function minimax(node, depth, maximizingPlayer) is
	if depth = 0 or node is a terminal node then
		return the heuristic value of node
	if maximizingPlayer then
		value := −∞
		for each child of node do
			value := max(value, minimax(child, depth − 1, FALSE))
		return value
	else (* minimizing player *)
		value := +∞
		for each child of node do
			value := min(value, minimax(child, depth − 1, TRUE))
		return value

	(* Initial call *)
	minimax(origin, depth, TRUE)
*/
