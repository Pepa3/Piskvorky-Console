﻿// Piškvorky-Console.cpp


#include <unordered_map>
#include <functional>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <fstream>
#include <sstream>
#include <bitset>
#include <chrono>
#include <string>
#include <vector>
#include <thread>
#include <random>
#include <time.h>
#include <array>
#include <list>
//#include <format>
constexpr int MM_DEPTH = 7;//5 doesn't work; 7 is slow => 4
//odd loses in midgame; even loses in earlygame but is accurate in midgame
constexpr int N = 2;
constexpr int PIECES_FOR_WIN = 5;

using namespace std;

// 
// perf = long game until win
// perf 4                       2600  ms
// perf 5                       140000 ms
// perf 6                       long
// 
// 
// one-move win =               ~10 ms
// 
// TODO: use lineEnds() to speed up minimax AB
// TODO: better bot progress reporting (percent)
// TODO: accuracy tests
// TODO: automatic playing through CLI, ./Piskvorky game.pisq => Bot: 4, 6


enum BoardState {
	NONE, X, O, BORDER
};

BoardState n(BoardState a)noexcept{
	return (a == X) ? O : X;
}

char s2c(BoardState stat) noexcept{
	switch (stat){
	case NONE:
		return '.';
	case X:
		return 'X';
	case O:
		return 'O';
	case BORDER:
		return '%';
	default:
		return '@';
	}
}

array<pair<int, int>,8> neighbours(int x, int y) noexcept{
	return { pair(x-1,y-1),{x,y-1},{x+1,y-1},{x-1,y},{x+1,y},{x-1,y+1},{x,y+1},{x+1,y+1} };
}

template<int W, int H> class Board {
public:
	BoardState player;

	Board(){
		stateX = 0;
		stateO = 0;
		hasNbour = 0;
		neighbourC = {0};
		evalP = {0};
		moves = vector<pair<int,int>>();
		player = BoardState(1);
		positions = unordered_map<size_t, bool>();
	}

	inline BoardState getState(unsigned x, unsigned y) noexcept{
		if (x>=W || y>=H || ((signed)x) < 0 || ((signed)y) < 0) return BORDER;
		return (BoardState)(stateX[x + y * W] + stateO[x + y * W] * 2);
	}
	inline bool getStateX(unsigned x, unsigned y) noexcept {
		if (x >= W || y >= H || ((signed)x) < 0 || ((signed)y) < 0) return false;
		return stateX[x + y * W];
	}
	inline bool getStateO(unsigned x, unsigned y) noexcept {
		if (x >= W || y >= H || ((signed)x) < 0 || ((signed)y) < 0) return false;
		return stateO[x + y * W];
	}

	bool setState(BoardState s, int x, int y, bool isPlayer)noexcept{
		if (getState(x,y) == NONE) {
			//hasNbour[x+y*W] = false; TODO:
			if (s == X) {
				stateX[x + y * W] = 1;
				evalP[x + y * W] = evalPointB(x, y);
				auto f = [&](int a, int b) -> void { if (a >= W || b >= H || a < 0 || b < 0)return; evalP[a + b * W]++; };
				for (unsigned i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
					f((x + i), (y));
					f((x + i), (y + i));
					f((x), (y + i));
					f((x - i), (y + i));
					f((x - i), (y));
					f((x - i), (y - i));
					f((x), (y - i));
					f((x + i), (y - i));
				}
			}else if(s==O){
				stateO[x + y * W] = 1;
				evalP[x + y * W] = -(signed)evalPointB(x, y);
				auto f = [&](int a, int b) -> void { if (a >= W || b >= H || a < 0 || b < 0)return; evalP[a + b * W]--; };
				for (unsigned i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
					f((x + i), (y));
					f((x + i), (y + i));
					f((x), (y + i));
					f((x - i), (y + i));
					f((x - i), (y));
					f((x - i), (y - i));
					f((x), (y - i));
					f((x + i), (y - i));
				}
			}
			if(isPlayer)moves.push_back({ x, y });
			//positions[hash()] = true;//TODO: add debug output
			for (auto& pos : neighbours(x, y)) {
				if (getState(pos.first, pos.second) != BORDER) {//Cannot use neighbourC>0 as hasNbour because its slower
					neighbourC[pos.first + pos.second * W]++;
					hasNbour[pos.first + pos.second * W] = true;
				}
			}
			/*auto f = [&](int a, int b) -> void { if (a >= W || b >= H || a < 0 || b < 0)return; hasNbour[a + b * W] = true; };
			f((x - 2) , (y    ));
			f((x + 2) , (y    ));
			f((x    ) , (y - 2));
			f((x    ) , (y + 2));
			f((x - 2) , (y + 2));
			f((x + 2) , (y + 2));
			f((x - 2) , (y - 2));
			f((x + 2) , (y - 2));*/
			return true;
		} else {
			return false;
		}
	}

	string window(){
		string ret = "";
		ret.append("    0 1 2 3 4 5 6 7 8 9 1011121314\n");
		char* str = new char[5];
		for (unsigned i = 0; i < W; i++) {
			snprintf(str,5,"%3d ", i);
			ret.append(str);
			for (unsigned j = 0; j < H; j++) {
				ret.append({ s2c(getState(i,j))});
				ret.append(" ");
			}
			ret.append("\n");
		}
		delete[] str;
		return ret;
	}

	string debug() {
		string ret = "";
		ret.append("    0 1 2 3 4 5 6 7 8 9 1011121314\n");
		char* str = new char[5];
		for (unsigned i = 0; i < W; i++) {
			snprintf(str, 5, "%3d ", i);
			ret.append(str);
			for (unsigned j = 0; j < H; j++) {
				ret.append(hasNeighbour(i, j) ? to_string(neighbourCount(i, j)) : string(1,'.'));
				ret.append({' '});
			}
			ret.append("\n");
		}
		ret.append("    0 1 2 3 4 5 6 7 8 9 1011121314\n");
		for (unsigned i = 0; i < W; i++) {
			snprintf(str, 5, "%3d", i);
			ret.append(str);
			for (unsigned j = 0; j < H; j++) {
				snprintf(str, 5, "%2d", evalP[i+j*W]);
				ret.append(str);
			}
			ret.append("\n");
		}
		delete[] str;
		return ret;
	}

	unsigned evalPoint(unsigned x, unsigned y) {
		BoardState cur = getState(x,y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		unsigned dx = 1, dy = 1, dxy = 1, dmxy = 1;
		if (cur == X) {
			for (int i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
				pxy &= getStateX(x + i, y);
				pxpy &= getStateX(x + i, y + i);
				xpy &= getStateX(x, y + i);
				mxpy &= getStateX(x - i, y + i);
				mxy &= getStateX(x - i, y);
				mxmy &= getStateX(x - i, y - i);
				xmy &= getStateX(x, y - i);
				pxmy &= getStateX(x + i, y - i);
				if (pxy) dx++;
				if (mxy) dx++;
				if (xmy) dy++;
				if (xpy) dy++;
				if (pxpy) dxy++;
				if (mxmy) dxy++;
				if (pxmy) dmxy++;
				if (mxpy) dmxy++;
			}
		} else {
			for (int i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
				pxy &= getStateO(x + i, y);
				pxpy &= getStateO(x + i, y + i);
				xpy &= getStateO(x, y + i);
				mxpy &= getStateO(x - i, y + i);
				mxy &= getStateO(x - i, y);
				mxmy &= getStateO(x - i, y - i);
				xmy &= getStateO(x, y - i);
				pxmy &= getStateO(x + i, y - i);
				if (pxy) dx++;
				if (mxy) dx++;
				if (xmy) dy++;
				if (xpy) dy++;
				if (pxpy) dxy++;
				if (mxmy) dxy++;
				if (pxmy) dmxy++;
				if (mxpy) dmxy++;
			}}
		return max(max(dx,dy), max(dxy, dmxy));
	}
	unsigned evalPointB(signed x, signed y) {
		BoardState cur = getState(x,y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		unsigned dx = 0, dy = 0, dxy = 0, dmxy = 0;
		if (cur == X) {
			for (int i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
				if (x + i >= W || y + i >= H || x - i < 0 || y - i < 0)break;
				pxy &=  stateX[x + i+ (y    )*W];
				pxpy &= stateX[x + i+ (y + i)*W];
				xpy &=  stateX[x    + (y + i)*W];
				mxpy &= stateX[x - i+ (y + i)*W];
				mxy &=  stateX[x - i+ (y    )*W];
				mxmy &= stateX[x - i+ (y - i)*W];
				xmy &=  stateX[x    + (y - i)*W];
				pxmy &= stateX[x + i+ (y - i)*W];
				dx += pxy;
				dx += mxy;
				dy += xmy;
				dy += xpy;
				dxy += pxpy;
				dxy += mxmy;
				dmxy += pxmy;
				dmxy += mxpy;
			}
		}else if(cur==O){
			for (int i = 1; i < PIECES_FOR_WIN; i++) {//px = ->; py = v
				if (x + i >= W || y + i >= H || x - i < 0 || y - i < 0)break;
				pxy &=  stateO[x + i+ (y)*W];
				pxpy &= stateO[x + i+ (y + i) * W];
				xpy &=  stateO[x+ (y + i) * W];
				mxpy &= stateO[x - i+ (y + i) * W];
				mxy &=  stateO[x - i+ (y)*W];
				mxmy &= stateO[x - i+ (y - i) * W];
				xmy &=  stateO[x    +(y - i) * W];
				pxmy &= stateO[x + i+ (y - i) * W];
				dx += pxy;
				dx += mxy;
				dy += xmy;
				dy += xpy;
				dxy += pxpy;
				dxy += mxmy;
				dmxy += pxmy;
				dmxy += mxpy;
			}
		}
		return 1+dx+dy+dxy+dmxy;
	}
	/*
		. . ? . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . O . . .
		. . ? . . .
	*/
	vector<pair<int,int>> getLineEnds(int x, int y) {
		BoardState cur = getState(x, y);
		bool pxy = true, pxpy = true, xpy = true, mxpy = true, mxy = true, mxmy = true, xmy = true, pxmy = true;
		bool bpxy = true, bpxpy = true, bxpy = true, bmxpy = true, bmxy = true, bmxmy = true, bxmy = true, bpxmy = true;
		vector<pair<int,int>> ends = vector<pair<int,int>>(8);
		for (unsigned i = 1; i <= 5; i++) {//px = v; py = ->
			pxy  &= (getState(x + i, y    ) == cur);
			pxpy &= (getState(x + i, y + i) == cur);
			xpy  &= (getState(x    , y + i) == cur);
			mxpy &= (getState(x - i, y + i) == cur);
			mxy  &= (getState(x - i, y    ) == cur);
			mxmy &= (getState(x - i, y - i) == cur);
			xmy  &= (getState(x    , y - i) == cur);
			pxmy &= (getState(x + i, y - i) == cur);
			if (!pxy  && bpxy)  { ends.push_back({ x + i, y     }); bpxy=false;};
			if (!pxpy && bpxpy) { ends.push_back({ x + i, y + i }); bpxpy =false;};
			if (!xpy && bxpy) { ends.push_back({ x    , y + i });   bxpy =false;};
			if (!mxpy && bmxpy) { ends.push_back({ x - i, y + i }); bmxpy =false;};
			if (!mxy && bmxy) { ends.push_back({ x - i, y });       bmxy =false;};
			if (!mxmy && bmxmy) { ends.push_back({ x - i, y - i }); bmxmy =false;};
			if (!xmy && bxmy) { ends.push_back({ x    , y - i });   bxmy =false;};
			if (!pxmy && bpxmy) { ends.push_back({ x + i, y - i }); bpxmy = false; };
		}
		return ends;
	}

	signed evalBoard() {
		//score = accumulate(evalP.begin(), evalP.end(), 0,plus<int>());
		//score = stateX.count() - stateO.count();
		auto first = evalP.begin();
		auto last = evalP.end();
		int init = 0;
		for (int i = 0; first != last; ++first){
			init += (stateX[i] || stateO[i]) ? *first : 0;
			i++;
		}
		return init;
	}

	inline bool hasNeighbour(int x, int y){
		return hasNbour[x + y * W];
	}
	
	inline unsigned neighbourCount(int x, int y){
		return neighbourC[x + y * W];
	}

	inline bool checkEndgame(int x, int y){return evalPoint(x, y) >= PIECES_FOR_WIN;}

	size_t hash() {
		long long a = std::hash<bitset<W*H>>()(stateX);
		long long b = std::hash<bitset<W*H>>()(stateO);
		long long A = a >= 0 ? 2 * a : -2 * a - 1;
		long long B = b >= 0 ? 2 * b : -2 * b - 1;
		long long C = (A >= B ? A * A + A + B : A + B * B) / 2;
		return a < 0 && b < 0 || a >= 0 && b >= 0 ? C : -C - 1;
	}

	string serialize(){
		string ret = "";
		ret.append("PISQ 1\n");
		if (moves.empty()) { printf("No moves were made. Cannot serialize the board.\n");return ret; }
		for (pair<int,int> move : moves) {
			char* str = new char[12];
			snprintf(str, 12, "MOVE %d %d\n", move.first, move.second);
			ret.append(str);
		}
		ret.append("CHECK ");
		for (unsigned i = 0; i < W; i++) {
			for (unsigned j = 0; j < H; j++) {
				ret.append({ s2c(getState(i,j)) });
			}
		}
		ret.append("\n");
		return ret;
	}
	void deserialize(ifstream& in){
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
			setState(player, x, y, true);
			player = n(player);
		}
		for (unsigned i = 0; i < W; i++) {
			for (unsigned j = 0; j < H; j++) {
				char s = s2c(getState(i,j));
				char s2 = line.at(6+j+i*15);
				if (s != s2) {
					printf("Checking failed, found inconsistency in PISQ file.\n");
					return;
				}
			}
		}
	}

	inline vector<pair<int, int>> getMoves() {return moves;}

private:
	bitset<W*H> stateX;
	bitset<W*H> stateO;
	bitset<W*H> hasNbour;
	unordered_map<size_t, bool> positions;
	array<unsigned, W*H> neighbourC;
	array<signed, W*H> evalP;
	vector<pair<int, int>> moves;
};

template<int W, int H> pair<signed, pair<int, int>> minimax(Board<W, H>* b, signed depth, signed alpha, signed beta) {
	Board<W,H> b1 = *b;
	if (depth <= 0) { return { b1.evalBoard(), { -1,-1 } }; }
	if (b1.player == X) {
		signed score = -2561;
		pair<int, int> best = { -1,-1 };
		vector<pair<int,int>> poss = vector<pair<int,int>>();
		poss.reserve(40);
		for (unsigned i = 0; i < W; i++) {
			for (unsigned j = 0; j < H; j++) {
				if(!b1.hasNeighbour(i,j)||b1.getState(i,j)!=NONE){continue;}
				poss.push_back({i,j});
			}
		}
		/*sort(poss.begin(), poss.end(), [&](pair<int, int> p1, pair<int, int> p2) {
			return b1.neighbourCount(p1.first,p1.second) < b1.neighbourCount(p2.first,p2.second);//closer together
		});*/
		sort(poss.begin(), poss.end(), [&](pair<int, int> p1, pair<int, int> p2) {
			return abs((signed)b1.neighbourCount(p1.first, p1.second) - N) < abs((signed)b1.neighbourCount(p2.first, p2.second) - N);//mid
		});
		/*auto lastMove = b1.getMoves().back();
		//if(depth==MM_DEPTH)	printf("%d %d \n",lastMove.first,lastMove.second);
		for (auto pos : neighbours(lastMove.first, lastMove.second)) {
			if (b1.getState(pos.first, pos.second) == NONE) {
				poss.insert(poss.begin(), pos);
			}
		}*/
		for (pair<int, int> pos : poss) {
			Board<W, H> b2 = std::move(b1);
			unsigned i = pos.first, j = pos.second;
			b2.setState(b2.player, i, j, false);
			if (b2.checkEndgame(i, j)) {
				best = { i,j };
				return { -score, best };
			}
		}
		unsigned count = 0;
		for (pair<int,int> pos : poss) {
			if (depth == MM_DEPTH) { cout << count << "/" << poss.size() << " " << flush; count++; }
			pair<signed,pair<int,int>> temp;
			Board<W,H> b2 = *b;
			unsigned i = pos.first, j = pos.second;
			b2.setState(b2.player, i, j, false);
			if (b2.checkEndgame(i, j)) { score = 2561; goto minimax_skip_1; }
			b2.player = n(b2.player);
			temp = minimax(&b2, depth - 1, alpha, beta);
			b2.player = n(b2.player);
			if (temp.first > score) {
				score = temp.first;
				minimax_skip_1:
				best = { i,j };
				if (score >= beta) { return { score, best }; }
				if (score >= 2500) { return { score, best }; }
			}
			alpha = max(alpha, score);
		}
		return { score, best };
	}else if (b1.player == O) {
		signed score = 2561;
		pair<int, int> best = { -1,-1 };
		vector<pair<int,int>> poss = vector<pair<int,int>>();
		poss.reserve(40);
		for (unsigned i = 0; i < W; i++) {
			for (unsigned j = 0; j < H; j++) {
				if(!b1.hasNeighbour(i,j)||b1.getState(i,j)!=NONE){continue;}
				poss.push_back({i,j});
			}
		}
		sort(poss.begin(), poss.end(), [&](pair<int, int> p1, pair<int, int> p2) {
			return abs((signed)b1.neighbourCount(p1.first, p1.second) - N) < abs((signed)b1.neighbourCount(p2.first, p2.second) - N);//mid
		});
		for (pair<int, int> pos : poss) {
			Board<W, H> b2 = std::move(b1);
			unsigned i = pos.first, j = pos.second;
			b2.setState(b2.player, i, j, false);
			if (b2.checkEndgame(i, j)) {
				best = { i,j };
				return { -score, best };
			}
		}
		unsigned count = 0;
		for (pair<int, int> pos : poss) {
			if (depth == MM_DEPTH) { cout << count << "/" << poss.size() << " " << flush; count++; }
			pair<signed,pair<int,int>> temp;
			Board<W,H> b2 = *b;
			unsigned i = pos.first, j = pos.second;
			b2.setState(b2.player, i, j, false);
			if (b2.checkEndgame(i, j)) { score = -2561; goto minimax_skip_2; }
			b2.player = n(b2.player);
			temp = minimax(&b2, depth - 1, alpha, beta);
			b2.player = n(b2.player);
			if (temp.first < score) {
				score = temp.first;
				minimax_skip_2:
				best = { i,j };
				if (score <= alpha) { return { score,best }; }
				if (score <= -2500) { return { score,best }; }
			}
			beta = min(beta, score);
		}
		return { score, best };
	}
	printf("FATAL: Unexpected player\n");
	return {-1,{-1,-1}};
}

int main() {
	srand((unsigned)time(NULL));
	const int ww = 15, wh = 15;
	Board<ww,wh> board = Board<ww,wh>();

	string in;
	bool endgame = false;
	while(!endgame) {
		printf("Player %cs turn\n", s2c(board.player));
		cout << board.window();
		printf("Score: %d\n", board.evalBoard());
		printf("Enter position('Y X'): ");
		getline(cin, in);
		size_t del = in.find(" ");
		std::string tok1 = in.substr(0, del);
		std::string tok2 = in.substr(del + 1, in.length());
		int a = -1, b = -1;
		if (in.compare("end") == 0) { endgame = true; continue; }
		if (in.compare("bot") == 0) {
			pair<signed, pair<int, int>> result = minimax(&board, MM_DEPTH, -2560, 2560);
			printf("Bot X%d Y%d\n", result.second.first, result.second.second);
			continue;
		}else if (in.compare("botm") == 0) {
			pair<bool, pair<int, int>> result = minimax(&board, MM_DEPTH, -2560, 2560);

			a = result.second.first;
			b = result.second.second;
			goto skip;//...
		}
		if (in.compare("time") == 0) {
			Board<15, 15>* b1 = new Board<15, 15>();
			ifstream ins;
			ins.open("perftest.pisq", ios::in);
			b1->deserialize(ins);
			ins.close();
			auto t1 = chrono::high_resolution_clock::now();
			minimax(b1, MM_DEPTH, -2560, 2560);
			auto t2 = chrono::high_resolution_clock::now();
			cout << "bot takes " << chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
			delete b1;
			continue;
		}else if (in.compare("test") == 0) {
		}
		if (in.compare("save") == 0) {
			ofstream out;
			out.open("out.pisq", ios::out | ios::trunc);
			out << board.serialize();
			out.close();
			printf("Saved to out.pisq\n");
			continue;
		}else if (in.compare("load") == 0) {
			ifstream ins;
			ins.open("out.pisq", ios::in);
			board.deserialize(ins);
			ins.close();
			printf("Read from out.pisq\n");
			continue;
		}
		if (in.compare("debug") == 0) {
			cout << board.debug();
			continue;
		}

		try	{
			a = stoi(tok1);
			b = stoi(tok2);
		}catch (const exception&) {
			printf("Error, please try again\n");
			continue;
		}
		skip:
		if (a >= ww || b >= wh) { printf("Position %d %d doesn't exist\n",a,b); continue; }
		if (!board.setState(board.player, a, b, true)) {
			//auto tmp = board.evalLineBlocked(a, b,false);
			//if (tmp.first) printf("Line output: %d %d\n", tmp.second.first, tmp.second.second);
			printf("Position already has a piece!\n");
			continue;
		}
		if (board.checkEndgame(a,b)) {
			printf("Player %c wins!\n",s2c(board.player));
			cout << board.window();
			break;
		}
		board.player = n(board.player);
		printf("Got position: %d %d\n",a,b);
	}
	while (!endgame) {
		this_thread::sleep_for(1ms);
	}
}
