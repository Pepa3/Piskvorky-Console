// Piškvorky-Console.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>
//#include <format>
constexpr unsigned MM_DEPTH = 6;//4 works  >5 is slow

using namespace std;

class Board {
public:
	enum class BoardState {
		NONE, X, O, BORDER
	};

	BoardState player;
	const unsigned PIECES_FOR_WIN = 5;

	Board(int w, int h, BoardState defval=BoardState(0)){
		state = vector<vector<BoardState>>(w,vector<BoardState>(h,defval));
		center = {w/2, h/2};
		player = BoardState(1);
	}

	BoardState getState(unsigned x, unsigned y) {
		if (x>state.size()-1 || y> state[0].size()-1 || ((signed)x) < 0 || ((signed)y) < 0) return BoardState::BORDER;
		return state[x][y];
	}

	inline bool isX(unsigned x, unsigned y) { return getState(x, y) == BoardState::X; }
	inline bool isO(unsigned x, unsigned y) { return getState(x, y) == BoardState::O; }
	inline bool isXO(unsigned x, unsigned y) { return getState(x, y) != BoardState::NONE; }

	BoardState n(BoardState a) {
		if (a == BoardState::NONE) throw exception();
		return (a == BoardState::X) ? BoardState::O : BoardState::X;
	}

	bool setState(BoardState s, int x, int y) {
		if (getState(x,y) == BoardState::NONE) {
			state[x][y] = s;
			moves.push_back({ x, y });
			return true;
		} else {
			return false;
		}
	}

	char s2c(BoardState stat) {
		switch (stat){
		case Board::BoardState::NONE:
			return '.';
		case Board::BoardState::X:
			return 'X';
		case Board::BoardState::O:
			return 'O';
		case Board::BoardState::BORDER:
			return '%';
		default:
			return '@';
		}
	}

	pair<int, int> getCenter() { return center; }
	void setCenter(pair<int, int> c) { center = c; }

	string window(unsigned w, unsigned h) {
		int x = center.first - w/2;
		int y = center.second - h/2;
		string ret = "";
		ret.append("   0 1 2 3 4 5 6 7 8 9 1011121314\n");
		for (unsigned i = 0; i < w; i++) {
			char* str = new char[4];
			snprintf(str,4,"%3ld", i);
			ret.append(str);
			delete[] str;
			for (unsigned j = 0; j < h; j++) {
				ret.append({ s2c(state[x+i][y+j])});
				ret.append(" ");
			}
			ret.append("\n");
		}
		return ret;
	}

	unsigned evalPoint(unsigned x, unsigned y) {
		BoardState cur = getState(x,y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		unsigned dx = 1, dy = 1, dxy = 1, dmxy = 1;
		for (unsigned i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
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
	signed evalPoint1(unsigned x, unsigned y) {
		switch (getState(x,y)){
		case BoardState::NONE:
			return 0;
		case BoardState::X:
			return evalPoint(x, y);
		case BoardState::O:
			return -(signed)evalPoint(x, y);
		default:
			throw exception();
		}
	}
	/*
		. . ? . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . ? . . .
	*/
	pair<bool,pair<int,int>> evalLineBlocked(int x, int y, bool offense) {
		BoardState cur = getState(x, y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		BoardState bpxy = cur, bpxpy = cur, bxpy = cur, bmxpy = cur, bmxy = cur, bmxmy = cur, bxmy = cur, bpxmy = cur;
		signed dx = 1, dy = 1, dxy = 1, dmxy = 1, dmx = 1, dmy = 1, dxmy = 1, dmxmy = 1;
		for (unsigned i = 1; i <= 5; i++) {//px = v; py = ->
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

			if (!pxy && bpxy == BoardState::NONE) { bpxy = getState(x + i + 1, y);				  if (bpxy != n(cur))bpxy = BoardState::NONE; };
			if (!pxpy && bpxpy == BoardState::NONE) { bpxpy = getState(x + i + 1, y + i + 1);	  if (bpxpy != n(cur))bpxpy = BoardState::NONE;};
			if (!xpy && bxpy == BoardState::NONE) { bxpy = getState(x, y + i + 1);				  if (bxpy != n(cur))bxpy = BoardState::NONE;};
			if (!mxpy && bmxpy == BoardState::NONE) { bmxpy = getState(x - (i + 1), y + i + 1);   if (bmxpy != n(cur))bmxpy = BoardState::NONE;};
			if (!mxy && bmxy == BoardState::NONE) { bmxy = getState(x - (i + 1), y);			  if (bmxy != n(cur))bmxy = BoardState::NONE;};
			if (!mxmy && bmxmy == BoardState::NONE) { bmxmy = getState(x - (i + 1), y - (i + 1)); if (bmxmy != n(cur))bmxmy = BoardState::NONE;};
			if (!xmy && bxmy == BoardState::NONE) { bxmy = getState(x, y - (i + 1));			  if (bxmy != n(cur))bxmy = BoardState::NONE;};
			if (!pxmy && bpxmy == BoardState::NONE) { bpxmy = getState(x + i + 1, y - (i + 1));   if (bpxmy != n(cur))bpxmy = BoardState::NONE;};

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
		pair<int, int> retXY = { -2,-2 };
		int bestb = 0;
		bool oneonethree = false;
		if ((dx + dmx) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			pair<int, int> retXY2 = { -1,-1 };
			int bestb2 = 0;
			if (bpxy == cur || bmxy == cur)oneonethree = true;
			if (bpxy == BoardState::NONE || bpxy == cur) { ret2.first = BoardState::NONE; bestb2++; retXY2 = { x + dx,y }; }
			if (bmxy == BoardState::NONE || bmxy == cur) { ret2.second = BoardState::NONE; bestb2++; retXY2 = { x - dmx,y }; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; retXY = retXY2; }
		}
		if ((dy + dmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			pair<int, int> retXY2 = { -1,-1 };
			int bestb2 = 0;
			if (bxmy == cur || bxpy == cur)oneonethree = true;
			if (bxmy == BoardState::NONE || bxmy == cur) { ret2.first = BoardState::NONE; bestb2++; retXY2 = { x,y - dy }; }
			if (bxpy == BoardState::NONE || bxpy == cur) { ret2.second = BoardState::NONE; bestb2++; retXY2 = { x,y + dmy }; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; retXY = retXY2; }
		}
		if ((dxy + dxmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			pair<int, int> retXY2 = { -1,-1 };
			int bestb2 = 0;
			if (bpxpy == cur || bmxmy == cur)oneonethree = true;
			if (bpxpy == BoardState::NONE || bpxpy == cur) { ret2.first = BoardState::NONE; bestb2++; retXY2 = { x + dxy,y + dxy }; }
			if (bmxmy == BoardState::NONE || bmxmy == cur) { ret2.second = BoardState::NONE; bestb2++; retXY2 = { x - dxmy,y - dxmy }; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; retXY = retXY2; }
		}
		if ((dmxy + dmxmy) == mmax) {
			pair<BoardState, BoardState> ret2 = { n(cur),n(cur) };
			pair<int, int> retXY2 = { -1,-1 };
			int bestb2 = 0;
			if (bpxmy == cur || bmxpy == cur)oneonethree = true;
			if (bpxmy == BoardState::NONE || bpxmy == cur) { ret2.first = BoardState::NONE; bestb2++; retXY2 = { x + dmxy, y - dmxy }; }
			if (bmxpy == BoardState::NONE || bmxpy == cur) { ret2.second = BoardState::NONE; bestb2++; retXY2 = { x - dmxmy, y + dmxmy }; }
			if (bestb2 > bestb) { bestb = bestb2; ret = ret2; retXY = retXY2; }
		}
		bool move = false;
		//mmax value is always one more than the actual stone count
		//TODO: fix .OOOOX vs .OOO. ???
		if (mmax == 5 && bestb >= 1) {//TODO: priority for winning (.XXXX)
			move = true;
		} else if (mmax==4 && bestb == 2) {// .XXX.X is fixed but .XXX..X is false-positive // Doesn't always work
			move = true;
		} else if (offense && mmax == 3 && bestb>=1) {
			move = true;
		} else if(oneonethree && mmax == 4){//fixes OXXX.X
			move = true;
		}

		return { move,retXY };
	}

	signed evalBoard() {
		signed score = 0;
		for (unsigned i = 0; i < state.size(); i++){
			for (unsigned j = 0; j < state[0].size(); j++){
				signed tmp = evalPoint1(i, j);
				if (tmp >= (signed)PIECES_FOR_WIN) { return 2561; }
				if (tmp <= -(signed)PIECES_FOR_WIN) { return -2561; }
				score += tmp;
			}
		}
		return score;
	}

	bool hasNeighbor(int x, int y) {
		if (getState(x - 1, y) != BoardState::NONE && getState(x - 1, y) != BoardState::BORDER ||
			getState(x + 1, y) != BoardState::NONE && getState(x + 1, y) != BoardState::BORDER ||
			getState(x, y - 1) != BoardState::NONE && getState(x, y - 1) != BoardState::BORDER ||
			getState(x, y + 1) != BoardState::NONE && getState(x, y + 1) != BoardState::BORDER ||
			getState(x - 1, y + 1) != BoardState::NONE && getState(x - 1, y + 1) != BoardState::BORDER ||
			getState(x + 1, y + 1) != BoardState::NONE && getState(x + 1, y + 1) != BoardState::BORDER ||
			getState(x - 1, y - 1) != BoardState::NONE && getState(x - 1, y - 1) != BoardState::BORDER ||
			getState(x + 1, y - 1) != BoardState::NONE && getState(x + 1, y - 1) != BoardState::BORDER) {
			return true;
		}
		return false;
	}
	bool hasNeighbor(int x, int y, BoardState s) {
		if (getState(x - 1, y) == s || getState(x + 1, y) == s ||
			getState(x, y - 1) == s || getState(x, y + 1) == s ||
			getState(x - 1, y + 1) == s || getState(x + 1, y + 1) == s ||
			getState(x - 1, y - 1) == s || getState(x + 1, y - 1) == s) {
			return true;
		}
		return false;
	}
	vector<pair<int, int>> neighbors(int x, int y) {
		return { {x - 1,y - 1},{x,y - 1},{x + 1,y - 1},{x - 1,y},{x + 1,y},{x - 1,y + 1},{x,y + 1},{x + 1,y + 1} };
	}

	bool checkX(int x, int y, unsigned a) {return evalPoint(x, y) >= a;}
	bool checkEndgame(int x, int y) {return evalPoint(x, y) >= PIECES_FOR_WIN;}

	pair<bool,pair<int, int>> bot() {
		vector<pair<int, int>> cs = vector<pair<int, int>>();
		//-------Defensive Moves-------
		//Check only interesting stones
		for (unsigned x = 0; x < state.size(); x++){
			for (unsigned y = 0; y < state[0].size(); y++){
				if (getState(x, y) == n(player)) cs.push_back({x,y});
			}
		}
		//defend if neccessary
		for (pair<int,int> c : cs){
			auto tmp = evalLineBlocked(c.first, c.second, false);
			if (tmp.first) { printf("Defense\n"); return tmp; }
		}
		cs.clear();
		/*
		* . . . . .
		* . . O . .
		* . . . O .
		* X O O O ?
		* . . . . .
		*/
		for (unsigned x = 0; x < state.size(); x++) {
			for (unsigned y = 0; y < state[0].size(); y++) {
				Board* b = new Board(*this);
				if(!b->setState(player,x,y)) {delete b; continue;}//TODO: merge ifs
				if(b->evalPoint(x, y) != 4) {delete b; continue;}
				if(b->evalLineBlocked(x, y, false).first) {
					printf("Found a trap!\n");//Doesn't work!
					delete b;
					return { true, {x, y} };
				}
				delete b;
			}
		}
		//-------Offensive Moves-------
		//Check only interesting stones
		for (unsigned x = 0; x < state.size(); x++) {
			for (unsigned y = 0; y < state[0].size(); y++) {
				if (getState(x, y) == player) cs.push_back({ x,y });
			}
		}
		//find the best move
		for (pair<int, int> c : cs) {
			auto tmp = evalLineBlocked(c.first, c.second,true);
			if (tmp.first) { printf("Offense - auto\n"); return tmp; }
		}
		cs.clear();

		//if no best move then random players neighbor
		for (unsigned x = 0; x < state.size(); x++) {
			for (unsigned y = 0; y < state[0].size(); y++) {
				if(hasNeighbor(x, y, player)&&getState(x,y)==BoardState::NONE) {
					cs.push_back({ x,y });
				}
			}
		}
		if (cs.size() > 0) {
			printf("Offense - random\n");
			int i = rand() % cs.size();
			return { true,cs.at(i) };
		}

		printf("First move.\n");
		//TODO:opening
		return { true, {7 + (rand() % 3) - 1, 7 + (rand() % 3) - 1} };
	}

	string serialize() {
		string ret = "";
		ret.append("PISQ 1\n");
		if (moves.size() == 0) { throw exception(); }
		for (pair<int,int> move : moves) {
			char* str = new char[12];
			snprintf(str, 12, "MOVE %d %d\n", move.first, move.second);
			ret.append(str);
		}
		ret.append("CHECK ");
		for (unsigned i = 0; i < state.size(); i++) {
			for (unsigned j = 0; j < state[0].size(); j++) {
				ret.append({ s2c(state[i][j]) });
			}
		}
		ret.append("\n");
		return ret;
	}

	void deserialize(ifstream& in) {
		if (moves.size() >= 1) { printf("Game already started! Please restart.\n"); return; }

		int version = 0;

		string line;
		getline(in, line);
		if (line.substr(0, 4).compare("PISQ") == 0) {
			version = stoi(line.substr(5, 1));
		} else {
			printf("ERROR: Malformed PISQ file.\n");
			return;
		}
		if (version != 1) {
			printf("ERROR: Unknown PISQ format!\n");
			return;
		}

		while (getline(in, line)) {
			if (line.substr(0, 4).compare("MOVE") != 0) break;
			istringstream str(line.substr(5));
			string a,b;
			if (!(str >> a >> b)) break;
			int x = stoi(a), y = stoi(b);
			moves.push_back({ x,y });
			setState(player, x, y);
			player = n(player);
		}
		for (unsigned i = 0; i < state.size(); i++) {
			for (unsigned j = 0; j < state[0].size(); j++) {
				char s = s2c(state[i][j]);
				char s2 = line.at(6+j+i*15);
				if (s != s2) {
					printf("Checking failed, found inconsistency in PISQ file.\n");
					return;
				}
			}
		}
	}

	pair<signed, pair<int, int>> minimax(Board* b, signed depth, signed alpha, signed beta) {
		Board b1 = *b;
		if (depth <= 0) { return { b1.evalBoard(), { -1,-1 } }; }
		else if(depth>MM_DEPTH-3){ printf("%d", depth); }
		if (b1.player == BoardState::X) {
			signed score = -2561;
			pair<int, int> best = { -1,-1 };
			for (unsigned i = 0; i < b1.state.size(); i++) {
				for (unsigned j = 0; j < b1.state[0].size(); j++) {
					pair<signed,pair<int,int>> temp;
					if (!b1.hasNeighbor(i, j)) { continue; }
					Board b2 = *b;
					if (!b2.setState(b2.player, i, j)) continue;
					if (b2.checkEndgame(i, j)) { score = 2561; goto minimax_skip_1; }
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					temp = minimax(&b2, depth - 1, alpha, beta);
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					if (temp.first > score) {
						score = temp.first;
						minimax_skip_1:
						best = { i,j };
						if (score > beta) { return { score, best }; }
						if (score >= 2500) { return { score, best }; }
					}
					alpha = max(alpha, score);
				}
			}
			return { score, best };
		}else if (b1.player == BoardState::O) {
			signed score = 2561;
			pair<int, int> best = { -1,-1 };
			for (unsigned i = 0; i < b1.state.size(); i++) {
				for (unsigned j = 0; j < b1.state[0].size(); j++) {
					pair<signed,pair<int,int>> temp;
					if (!b1.hasNeighbor(i, j)) { continue; }
					Board b2 = *b;
					if(!b2.setState(b2.player, i, j)) continue;
					if (b2.checkEndgame(i, j)) { score = -2561; goto minimax_skip_2; }
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					temp = minimax(&b2, depth - 1, alpha, beta);
					b2.player = (b2.player == BoardState::X) ? BoardState::O : BoardState::X;
					if (temp.first < score) {
						score = temp.first;
						minimax_skip_2:
						best = { i,j };
						if (score < alpha) { return { score,best }; }
						if (score <= -2500) { return { score,best }; }
					}
					beta = min(beta, score);
					//score = min(score, minimax(&b2, depth-1));
				}
			}
			return { score, best };
		}
		printf("Failed\n");
		return { 0, { -1, -1 } };
	}

	vector<pair<int, int>> getMoves() {return moves;}
	
private:
	vector<vector<BoardState>> state;
	vector<pair<int, int>> moves;
	pair<int, int> center;
};

int main() {
	srand((unsigned)time(NULL));
	int ww = 15, wh = 15;
	Board board = Board(ww,wh);

	string in;
	bool endgame = false;
	while(!endgame) {
		printf("Player %c is playing\n", board.s2c(board.player));
		cout << board.window(ww, wh);
		printf("Score: %d\n", board.evalBoard());
		printf("Enter position('Y X'): ");
		getline(cin, in);
		size_t del = in.find(" ");
		std::string tok1 = in.substr(0, del);
		std::string tok2 = in.substr(del + 1, in.length());
		int a = -1, b = -1;
		try	{
			a = stoi(tok1);
			b = stoi(tok2);
		}catch (const exception&) {
			if (in.compare("end") == 0) { endgame = true; continue; }
			if (in.compare("bot") == 0) {
				pair<signed, pair<int, int>> result = board.minimax(&board, MM_DEPTH, -2560, 2560);
				printf("Bot X%d Y%d\n",result.second.first,result.second.second);
				continue;
			} else if (in.compare("botm") == 0) {
				pair<bool, pair<int, int>> result = board.minimax(&board, MM_DEPTH, -2560, 2560);
				
				a = result.second.first;//TODO: fix if windowing implemented
				b = result.second.second;
				goto skip;//...
			}
			if (in.compare("time") == 0) {
				auto t1 = chrono::high_resolution_clock::now();
				board.minimax(&board, MM_DEPTH, -2560, 2560);
				auto t2 = chrono::high_resolution_clock::now();
				cout << "bot takes " << chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
			}
			if (in.compare("save") == 0) {
				ofstream out;
				out.open("out.pisq", ios::out | ios::trunc);
				out << board.serialize();
				out.close();
				printf("Saved to out.pisq\n");
				continue;
			}else if (in.compare("load") == 0) {
				ifstream in;
				in.open("out.pisq", ios::in);
				board.deserialize(in);
				in.close();
				printf("Read from out.pisq\n");
				continue;
			}
			printf("Error, please try again\n");
			continue;
		}
		skip:
		int x = board.getCenter().first - ww / 2;
		int y = board.getCenter().second - wh / 2;
		if (x + a >= ww || y + b >= wh) { printf("Position %d %d doesn't exist\n",a,b); continue; }
		if (!board.setState(board.player, x + a, y + b)) {
			auto tmp = board.evalLineBlocked(x + a, y + b,false);
			if (tmp.first) printf("Line output: %d %d\n", tmp.second.first, tmp.second.second);
			printf("Position already has a piece!\n");
			continue;
		}
		if (board.checkEndgame(x+a,y+b)) {
			printf("Player %c wins!\n",board.s2c(board.player));
			cout << board.window(ww, wh);
			break;
		}
		board.player = Board::BoardState((uint8_t)board.player % 2 + 1);
		printf("Got position: %d %d\n",a,b);
	}
	while (!endgame) {
		this_thread::sleep_for(1ms);
	}
}

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
