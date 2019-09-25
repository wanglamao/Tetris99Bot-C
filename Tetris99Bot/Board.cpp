﻿#include "Board.h"

int Board::s[] = { 0, -800, -800, -500, 6200 };
long Board::EMPTY_ROW = 0;
long Board::FULL_ROW = 0x3FF;
Board::Board() {}

inline Board::Board(long rows[20], Piece* next[6], Piece* hold, Piece *currentPiece) {
	memcpy(this->rows, rows, 20 * sizeof(long));
	memcpy(this->next, next, 6 * sizeof(Piece*));
	this->hold = hold;
	this->currentPiece = currentPiece;

}
inline Board Board::copy() {
	return  Board(rows, next, hold, currentPiece);
}
inline void Board::setBits(int x, long pieceRowCells) {
	rows[x] |= pieceRowCells;
}

/**
 * Cleares part of a piece on the board.
 *
 * @param x                    board row
 * @param inversePieceRowCells filled cells of a specific piece row
 */
inline void Board::clearBits(int x, long inversePieceRowCells) {
	rows[x] &= inversePieceRowCells;
}

/**
 * @param x             board row
 * @param pieceRowCells filled cells of a specific piece row
 * @return true if the piece row cells are not occupied on the board
 */
inline bool Board::isBitsFree(int x, long pieceRowCells) {

	return (rows[x] & pieceRowCells) == 0;

}

inline void Board::fill(int x, int y) {
	setBits(x, 1 << y);
}

inline int Board::fill(int y, PieceShape* shape) {
	int x = minX(y, shape);
	if (x == 20) {
		return x;
	}
	bool isCreateHole = fill(x, y, shape);

	int clean = clearRows(x, shape->h);
	if (isCreateHole && clean == 0) {
		return -99999;
	}

	int height = 2 * x + shape->h - 1;
	//        return 34 * cleaned - height * 45;
	return s[clean] - height * 22;
}

inline  bool Board::fill(int x, int y, PieceShape* shape) {
	Point *shapePoints = shape->getPoints();
	bool isCreateHole = false;
	for (int i = 0; i < 4; i++) {
		Point shapePoint = shapePoints[i];
		int xx = x - shapePoint.x;
		int yy = y + shapePoint.y;
		fill(xx,yy );
	}
	for (int i = 0; i < 4; i++) {
		Point shapePoint = shapePoints[i];
		int xx = x - shapePoint.x;
		int yy = y + shapePoint.y;
		if (isFree(xx - 1, yy))
			isCreateHole = true;
	}
	return isCreateHole;
}

inline int Board::minX(int y, PieceShape* shape) {
	Point* points = shape->getPoints();
	for (int x = 19; x >= 0; x--) {
		for (int j = 0; j < 4; j++) {
			Point point = points[j];
			if (isFilled(x - point.x - 1, y + point.y)) {
				return x;
			}
		}
	}
}

inline int Board::clearRows(int x, int pieceHeight) {
	int clearedRows = 0;
	int x1 = x - pieceHeight;

	// Find first row to clear
	do {
		x1++;
		if (rows[x1] == FULL_ROW) {
			clearedRows++;
		}
	} while (clearedRows == 0 && x1 < x);

	// Clear rows
	if (clearedRows > 0) {
		int x2 = x1;

		while (x1 < 20) {
			x2++;
			while (x2 <= x && rows[x2] == FULL_ROW) {
				clearedRows++;
				x2++;
			}
			if (x2 < 20) {
				rows[x1] = rows[x2];
			}
			else {
				rows[x1] = Board::EMPTY_ROW;
			}
			x1++;
		}
	}
	return clearedRows;
}


int calc_01_change_count(long n_input)
{
	n_input = n_input << 1;
	n_input |= 0b100000000001;

	long tmp = (n_input << 1);
	long n = n_input ^ tmp;//check how many bits are different
	long start = (n >> (12)), end = (n & 0x01);//check the first and the last bit
	long count = 0;//num of 1->0 or 0->1

	for (count = 0; n; ++count)
	{
		n &= (n - 1);
	}
	count -= (start + end);
	return count;
}

bool isWell(long l,int y) {
	l = l << 1;
	l |= 0b100000000001;

	l=l >> y;
	l &= 0b111;
	return l == 0b101;
}

int well[] = { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105,
	120, 136, 153, 171, 190, 210 };
Value Board::value(int avgV) {

	int rowTransitions = 0;
	int colTransitions = 0;
	int numberOfHoles = 0;
	int wellSums = 0;
	for (int y = 0; y < 10; y++) {
		bool now = true;
		for (int x = 0; x < 20; x++) {
			if (isFilled(x, y) != now) {
				now = isFilled(x, y);
				rowTransitions++;
			}
		}
		if (!now)
			rowTransitions++;
	}


	for (int x = 0; x < 20; x++) {
		colTransitions += calc_01_change_count(rows[x]);
	}



	for (int y = 0; y < 10; y++) {
		bool f = false;
		int block = 0;
		for (int x = 19; x >= 0; x--) {
			if (isFilled(x, y)) {
				f = true;
				block++;
			}
			else if (f) {
				numberOfHoles += block;
				f = false;
				block = 0;
			}
		}
	}

	int cnt = 0;
	int maxCnt = 0;

	for (int x = 0; x < 20; x++)
		for (int y = 0; y < 10; y++) {
			if (isWell(rows[x],y)) {
				cnt++;
			}
			else {
				wellSums += well[cnt];
				if (cnt > maxCnt) {
					maxCnt = cnt;
				}
				cnt = 0;
			}
		}
	wellSums -= well[maxCnt];

	bool free9 = true;
	for (int x = 0; x < 20; x++) {
		if (isFilled(x, 0)) {
			free9 = false;
		}
	}

	int f = 0;
	if (free9) {
		f = 500;
	}

	int sumV = f-32 * rowTransitions
		- 93 * colTransitions
		//- 79 * numberOfHoles
		- 500 * numberOfHoles
		- 34 * wellSums;
	return  Value(avgV, sumV);
}


inline bool Board::isFilled(int x, int y) {
	if (x < 0)
		return true;
	return (rows[x] & (1L << y)) != 0;
}
inline bool Board::isFree(int x, int y) {
	if (x < 0)
		return false;
	return (rows[x] & (1L << y)) == 0;
}

void Board::paintAll(Move move) {
	Board copy = this->copy();
	list<Point> list;
	M m = move.m;
	PieceShape* shape = m.piece->pieceShapes[m.rotateIndex];
	Point* pp = shape->getPoints();
	for (int i = 0; i < 4; i++) {
		Point point = *(pp + i);
		int x = copy.minX(m.y, shape) - point.x;
		int y = m.y + point.y;
		list.push_back(Point(x, y));
	}
	string p[20];
	for (int x = 19; x >= 0; x--) {
		p[x] = "|";
		for (int y = 0; y < 10; y++) {
			if (copy.isFilled(x, y)) {
				p[x] += ("█");
			}
			else if (contain(list, x, y)) {
				p[x] += ("▓");
			}
			else {
				p[x] += ("  ");
			}
		}
		p[x] += "|";
	}

	while (move.next != NULL) {
		copy.fill(move);
		list.clear();
		m = move.next->m;
		shape = m.piece->pieceShapes[m.rotateIndex];
		pp = shape->getPoints();
		for (int i = 0; i < 4; i++) {
			Point point = *(pp + i);
			int x = copy.minX(m.y, shape) - point.x;
			int y = m.y + point.y;
			list.push_back(Point(x, y));
		}


		for (int x = 19; x >= 0; x--) {
			p[x] += "|";
			for (int y = 0; y < 10; y++) {
				if (copy.isFilled(x, y)) {
					p[x] += ("█");
				}
				else if (contain(list, x, y)) {
					p[x] += ("▓");
				}
				else {
					p[x] += ("  ");
				}
			}
			p[x] += "|";
		}
		move = *(move.next);
	}
	for (int x = 19; x >= 0; x--) {
		std::cout << p[x] << endl;
	}
	std::cout << "---------------------------------------------------------------------------------------------------------------------------------" << endl;

}

inline bool Board::contain(list<Point> points, int x, int y) {
	for (Point p : points) {
		if (p.x == x && p.y == y) {
			return true;
		}
	}
	return false;
}

void Board::paint() {
	string s = string("\n");
	for (int x = 19; x >= 0; x--) {
		s.append("|");
		for (int y = 0; y < 10; y++) {
			if (isFilled(x, y)) {
				s.append("█");
			}
			else {
				s.append("  ");
			}
		}
		s.append("|\n");
	}
	s.append("----------------------\n"); s.append("nextList:");
	for (int i = 0; i < 6; i++) {
		s.append(next[i] == NULL ? "null" : string(1, next[i]->character()));
	}
	s.append("\n")
		.append("hold    : ").append(hold == NULL ? "null" : string(1, hold->character())).append("\n")
		.append("current : ").append(currentPiece == NULL ? "null" : string(1, currentPiece->character())).append("\n");
	std::cout << s << endl;

}
//struct compare
//{
//	bool operator()(const Move& e1, const Move& e2)const {
//		return (e2.getAllMoveValue() - e1.getAllMoveValue()) > 0;
//
//	}
//};

Move* Board::backGet() {
	if (hold == NULL) {
		hold = next[0];
		refreshNext();
		return new Move(0, 0, hold, Value(0, 0), true);
	}

	set<Move> *moves = new set<Move>();
	backgetMoves(moves);
	if (moves->empty()) {
		return 0;
	}
	set<Move>::iterator itr;
	itr = moves->begin();
	Move b = *itr;
	Move *a = (Move*)&*itr;
	return  a;
}
Move* Board::get(int dep) {
	if (hold == NULL) {
		hold = next[0];
		refreshNext();
		return new Move(0, 0, hold, Value(0, 0), true);
	}

	set<Move> *moves = new set<Move>();
	getMoves(moves);
	if (moves->empty()) {
		return 0;
	}
	set<Move>::iterator itr;
	itr = moves->begin();
	Move b = *itr;
	Move *a = (Move*)&*itr;
	if (dep <= 0) {
		return  a;
	}

	Move* rs = new Move();
	for (int i = 0; i < 4 && itr != moves->end(); i++) {
		Move *move = (Move*)&*itr;
		Board copy = this->copy();
		copy.useMove(*move);
		move->next = copy.get(dep - 1);
		int a = move->getAllMoveValue();
		int b = rs->getAllMoveValue();
		if (b == 0 || a > b) {
			rs = (Move*)&*itr;
		}
		itr++;
	}
	return rs;
}

inline void Board::getMoves(set<Move>* moves) {
	getPieceMoves(moves, currentPiece);
	getPieceMoves(moves, hold, true);
}
inline void Board::backgetMoves(set<Move>* moves) {
	backgetPieceMoves(moves, currentPiece,false);
	backgetPieceMoves(moves, hold, true);
}

inline void Board::backgetPieceMoves(set<Move> *moves, Piece* piece, bool isUseHold ) {

	int index = piece->rotationsEndIndex();
	for (int i = 0; i <= index; i++) {
		PieceShape* shape = piece->pieceShapes[i];
		int width = shape->w;
		for (int y = 0; y <= 10 - width; y++) {
			Board copy = this->copy();
			 copy.fill(y, shape);
			Value value = copy.value(0);
			Move* tmp = new Move(y, i, piece, value, isUseHold);
			moves->insert(*tmp);
		}
	}
}
inline void Board::getPieceMoves(set<Move> *moves, Piece* piece, bool isUseHold) {

	int index = piece->rotationsEndIndex();
	for (int i = 0; i <= index; i++) {
		PieceShape* shape = piece->pieceShapes[i];
		int width = shape->w;
		for (int y = 0; y <= 10 - width; y++) {
			Board copy = this->copy();
			int v = copy.fill(y, shape);
			if (v == -99999) {
				continue;
			}
			Value value = copy.value(v);
			Move* tmp = new Move(y, i, piece, value, isUseHold);
			moves->insert(*tmp);
		}
	}
}

inline void Board::useMove(Move move) {
	if (move.m.isUseHold)
		hold = currentPiece;
	currentPiece = next[0];
	refreshNext();
	fill(move);
}

inline void Board::refreshNext() {
	memcpy(next, next + 1, sizeof(Piece*) * 5);
}

inline void Board::fill(Move move) {
	PieceShape* shape = move.m.piece->pieceShapes[move.m.rotateIndex];
	fill(move.m.y, shape);
}
