#include <assert.h>
#include <stdio.h>

#include <map>
#include <sstream>
#include <string>
#include <vector>

class Figure {
 public:
  enum Orientation {
    kHorizontal,
    kVertical 
  };

  Figure(int l, Orientation o)
    : length_(l), orient_(o) {}

  int length() const {return length_;}
  int orient() const {return orient_;}

  std::string GetDebugString() const {
    std::ostringstream oss;    
    oss << "l=" << length_ << " " <<
      "o=" << (orient_ == kHorizontal ? "hor" : "vert");
    return oss.str();
  }

  char GetBlockDebugChar(int i) const {
    if (orient_ == kHorizontal) {
      if (i == 0) {
        return '<';
      } else if (i == length_ - 1) {
        return '>';
      } else {
        return '-';
      }
    } else {
      if (i == 0) {
        return '^';
      } else if (i == length_ - 1) {
        return 'v';
      } else {
        return '|';
      }
    } 
  }

 private:
  const int length_;
  const Orientation orient_;
};

class PlacedFigure {
 public:
  PlacedFigure(const Figure* f, int h, int v)
    : figure_(f), hor_pos_(h), vert_pos_(v) {}

  const Figure* figure() const { return figure_; }
  int hor_pos() const { return hor_pos_; }
  int vert_pos() const { return vert_pos_; }

  // TODO: Remove in favor of shifting by one step.
  void MoveTo(int hor_pos, int vert_pos) {
    hor_pos_ = hor_pos;
    vert_pos_ = vert_pos;
  }

  bool operator<(const PlacedFigure& other) const {
    if (figure_ != other.figure_) {
      return figure_ < other.figure_;
    }
    if (hor_pos_ != other.hor_pos_) {
      return hor_pos_ < other.hor_pos_;
    }
    if (vert_pos_ != other.vert_pos_) {
      return vert_pos_ < other.vert_pos_;
    }
    return false;
  }

  bool operator==(const PlacedFigure& other) const {
    return figure_ == other.figure_ &&
           hor_pos_ == other.hor_pos_ &&
           vert_pos_ == other.vert_pos_;
  }

  std::string GetDebugString() const {
    std::ostringstream oss;    
    oss << figure_->GetDebugString() << " " <<
      "h=" << hor_pos_ << " " <<
      "v=" << vert_pos_;
    return oss.str();
  }

  char GetBlockDebugChar(int hor_pos, int vert_pos) const {
    if (figure_->orient() == Figure::kHorizontal) {
      return figure_->GetBlockDebugChar(hor_pos - hor_pos_);
    } else {
      return figure_->GetBlockDebugChar(vert_pos - vert_pos_);
    } 
  }

 private:
  const Figure *figure_;
  int hor_pos_;
  int vert_pos_;  
};

class Board {
 public:
  Board(int w, int l)
    : width_(w), length_(l),
      figures_(), figure_by_pos_(w * l, NULL) {
    figures_.reserve(kMaxFigures);
  }

  Board(const Board& from)
    : width_(from.width_), length_(from.length_),
      figures_(from.figures_), figure_by_pos_() {
    RecomputeFigureByPos();
  }

  void operator=(const Board& from) {
    width_ = from.width_;
    length_ = from.length_;
    figures_ = from.figures_;
    RecomputeFigureByPos();
  }

  int width() const {return width_;}
  int length() const {return length_;}
  std::vector<PlacedFigure>& figures() {return figures_;}

  void AddFigure(const Figure *figure, int hor_pos, int vert_pos) {
    assert(figures_.size() < kMaxFigures);
    figures_.push_back(PlacedFigure(figure, hor_pos, vert_pos));
    AddFigureToPos(&figures_.back());
  }

  bool CanMoveForward(PlacedFigure* pfig) {
    int hor_pos = pfig->hor_pos();
    int vert_pos = pfig->vert_pos();

    if (pfig->figure()->orient() == Figure::kHorizontal) {
      if (hor_pos + pfig->figure()->length() >= width_) {
        return false;
      }
      return GetFigureAtPos(hor_pos + pfig->figure()->length(),
                            vert_pos) == NULL;
    } else {
      if (vert_pos + pfig->figure()->length() >= length_) {
        return false;
      }
      return GetFigureAtPos(hor_pos,
                            vert_pos + pfig->figure()->length()) == NULL;
    }
  }

  bool CanMoveBack(PlacedFigure* pfig) {
    int hor_pos = pfig->hor_pos();
    int vert_pos = pfig->vert_pos();

    if (pfig->figure()->orient() == Figure::kHorizontal) {
      if (hor_pos == 0) {
        return false;
      }
      return GetFigureAtPos(hor_pos - 1, vert_pos) == NULL;
    } else {
      if (vert_pos == 0) {
        return false;
      }
      return GetFigureAtPos(hor_pos, vert_pos - 1) == NULL;
    }
  }

  void MoveForward(PlacedFigure* pfig) {
    int hor_pos = pfig->hor_pos();
    int vert_pos = pfig->vert_pos();

    assert(GetFigureAtPos(hor_pos, vert_pos) == pfig);
    
    SetFigureAtPos(NULL, hor_pos, vert_pos);
    if (pfig->figure()->orient() == Figure::kHorizontal) {
      SetFigureAtPos(pfig, hor_pos + pfig->figure()->length(), vert_pos);
      pfig->MoveTo(hor_pos + 1, vert_pos); 
    } else {
      SetFigureAtPos(pfig, hor_pos, vert_pos + pfig->figure()->length());
      pfig->MoveTo(hor_pos, vert_pos + 1);
    }
    // fprintf(stdout, "Move forward: %s\n", pfig->GetDebugString().c_str());
  }

  void MoveBack(PlacedFigure* pfig) {
    int hor_pos = pfig->hor_pos();
    int vert_pos = pfig->vert_pos();

    if (pfig->figure()->orient() == Figure::kHorizontal) {
      assert(GetFigureAtPos(hor_pos + pfig->figure()->length() - 1,
                            vert_pos) == pfig);
      SetFigureAtPos(NULL, hor_pos + pfig->figure()->length() - 1, vert_pos);
      SetFigureAtPos(pfig, hor_pos - 1, vert_pos);
      pfig->MoveTo(hor_pos - 1, vert_pos); 
    } else {
      assert(GetFigureAtPos(hor_pos,
                            vert_pos + pfig->figure()->length() - 1) == pfig);
      SetFigureAtPos(NULL, hor_pos, vert_pos + pfig->figure()->length() - 1);
      SetFigureAtPos(pfig, hor_pos, vert_pos - 1);
      pfig->MoveTo(hor_pos, vert_pos - 1);
    }
    // fprintf(stdout, "Move back: %s\n", pfig->GetDebugString().c_str());
  }

  bool operator<(const Board& other) const {
    for (int i = 0; i < figures_.size(); ++i) {
      if (figures_[i] == other.figures_[i]) {
        continue;
      } else if (figures_[i] < other.figures_[i]) {
        return true;
      } else {
        return false;
      }
    }
    return false;
  }

  bool operator==(const Board& other) const {
    for (int i = 0; i < figures_.size(); ++i) {
      if (figures_[i] == other.figures_[i]) {
        continue;
      }
      return false;
    }
    return true;
  }

  PlacedFigure* GetFigureAtPos(int hor_pos, int vert_pos) const {
    return figure_by_pos_[vert_pos * width_ + hor_pos];
  }

  bool IsWin() const {
    return figures_[0].hor_pos() == (width_ - figures_[0].figure()->length());
  }
 
  void DebugPrint() const {
    for (int i = 0; i < figures_.size(); ++i) {
      fprintf(stderr, "figure#%d: %s\n", i,
              figures_[i].GetDebugString().c_str());
    }
    for (int j = 0; j < length_; ++j) {
      for (int i = 0; i < width_; ++i) {
        PlacedFigure* pfig = GetFigureAtPos(i, j);
        if (pfig) {
          fprintf(stderr, "%c",
                  pfig->GetBlockDebugChar(i, j));
        } else {
          fprintf(stderr, ".");
        }
      }
      fprintf(stderr, "\n");
    }
  }

 private:
  const static int kMaxFigures = 12;

  void SetFigureAtPos(PlacedFigure *pfig, int hor_pos, int vert_pos) {
    assert((pfig == NULL) || (GetFigureAtPos(hor_pos, vert_pos) == NULL));
    figure_by_pos_[vert_pos * width_ + hor_pos] = pfig;
  }

  void AddFigureToPos(PlacedFigure* pfig) {
    for (int i = 0; i < pfig->figure()->length(); ++i) {
      if (pfig->figure()->orient() == Figure::kHorizontal) {
        SetFigureAtPos(pfig, pfig->hor_pos() + i, pfig->vert_pos());
      } else {
        SetFigureAtPos(pfig, pfig->hor_pos(), pfig->vert_pos() + i);
      }
    }
  }

  void RecomputeFigureByPos() {
    figure_by_pos_.resize(0);
    figure_by_pos_.resize(width_ * length_, NULL);
    for (int i = 0; i < figures_.size(); ++i) {
      AddFigureToPos(&figures_[i]);
    }
  }

  int width_;
  int length_;
  std::vector<PlacedFigure> figures_;
  std::vector<PlacedFigure*> figure_by_pos_;
};

class BoardCache {
 public:
  BoardCache() : cache_() {}
  
  void AddBoard(const Board& board,
                const Board* parent) {
    cache_[board] = parent;
  }

  bool IsBoardPresent(const Board& board) {
    return cache_.find(board) != cache_.end(); 
  }

  const Board& GetBoardRef(const Board& board) {
    return cache_.find(board)->first; 
  }

  void BackTrace(const Board* board) const {
    fprintf(stdout, "BackTrace:\n");
    int moves = 0;
    do {
      moves++;
      fprintf(stdout, "%d move:\n", moves);
      board->DebugPrint();
    } while(board = GetParent(board));
    fprintf(stdout, "%d moves in total\n", moves - 1);
  }
 private:
  const Board* GetParent(const Board* board) const {
    return cache_.find(*board)->second; 
  }

  std::map<Board,const Board*> cache_;
};

int SolveBoard(Board& board) {
  BoardCache cache;
  std::vector<Board> states;

  cache.AddBoard(board, NULL);
  states.push_back(board);
 
  for (int i = 0; i < states.size(); ++i) {
    board = states[i];
    // Set correct win condition.
    if (board.IsWin()) {
      cache.BackTrace(&board);
      fprintf(stdout, "Solved! States analyzed = %d\n", i);
      break;
    }
    const Board* parent = &cache.GetBoardRef(board);
    for (int i = 0; i < board.figures().size(); ++i) {
      PlacedFigure* pfig = &board.figures()[i];

      if (board.CanMoveForward(pfig)) {
        board.MoveForward(pfig);
        if (!cache.IsBoardPresent(board)) {
          cache.AddBoard(board, parent);
          states.push_back(board);
        }
        board.MoveBack(pfig);
      }

      if (board.CanMoveBack(pfig)) {
        board.MoveBack(pfig);
        if (!cache.IsBoardPresent(board)) {
          cache.AddBoard(board, parent);
          states.push_back(board);
        }
        board.MoveForward(pfig);
      }
    }
  }  // while is not empty.
}

int RunTest() {
  // Make board with figures.
  Board board(4, 4);

  const Figure figure1(2, Figure::kHorizontal);
  board.AddFigure(&figure1, 0, 0);
  PlacedFigure* pfig1 = board.GetFigureAtPos(0, 0);
  assert(pfig1->figure() == &figure1);

  const Figure figure2(3, Figure::kVertical);
  board.AddFigure(&figure2, 2, 0);
  PlacedFigure* pfig2 = board.GetFigureAtPos(2, 0);
  assert(pfig2->figure() == &figure2);

  Board board2(board);

  board.DebugPrint();
  
  // Check CanMove*.
  assert(!board.CanMoveForward(pfig1));
  assert(!board.CanMoveBack(pfig1));
  assert(board.CanMoveForward(pfig2));
  assert(!board.CanMoveBack(pfig2));

  // Check cache.
  BoardCache cache;
  cache.AddBoard(board, NULL);
  assert(cache.IsBoardPresent(board));
  assert(cache.IsBoardPresent(board2));

  // Start moving figures.
  board.MoveForward(pfig2);
  
  board.DebugPrint();

  assert(board.CanMoveForward(pfig1));
  assert(!board.CanMoveBack(pfig1));
  assert(!board.CanMoveForward(pfig2));
  assert(board.CanMoveBack(pfig2));
  
  board.MoveForward(pfig1);
  
  board.DebugPrint();

  assert(board.CanMoveForward(pfig1));
  assert(board.CanMoveBack(pfig1));
  assert(!board.CanMoveForward(pfig2));
  assert(!board.CanMoveBack(pfig2));
  
  board.MoveBack(pfig1);
  
  board.DebugPrint();

  assert(board.CanMoveForward(pfig1));
  assert(!board.CanMoveBack(pfig1));
  assert(!board.CanMoveForward(pfig2));
  assert(board.CanMoveBack(pfig2));
  
  board.MoveBack(pfig2);
  
  board.DebugPrint();

  assert(!board.CanMoveForward(pfig1));
  assert(!board.CanMoveBack(pfig1));
  assert(board.CanMoveForward(pfig2));
  assert(!board.CanMoveBack(pfig2));

  assert(board == board2);

  SolveBoard(board);
  return 0;
}

void Solve1() {
  // Make board with figures.
  Board board(6, 6);

  const Figure figure1(2, Figure::kHorizontal);
  board.AddFigure(&figure1, 0, 2);
  const Figure figure2(3, Figure::kHorizontal);
  board.AddFigure(&figure2, 0, 0);
  const Figure figure3(3, Figure::kVertical);
  board.AddFigure(&figure3, 5, 0);
  const Figure figure4(3, Figure::kVertical);
  board.AddFigure(&figure4, 2, 1);
  const Figure figure5(2, Figure::kVertical);
  board.AddFigure(&figure5, 0, 3);
  const Figure figure6(2, Figure::kVertical);
  board.AddFigure(&figure6, 4, 4);
  const Figure figure7(2, Figure::kHorizontal);
  board.AddFigure(&figure7, 4, 3);
  const Figure figure8(3, Figure::kHorizontal);
  board.AddFigure(&figure8, 0, 5);

  board.DebugPrint();

  SolveBoard(board);
}

void Solve2() {
  // Make board with figures.
  Board board(6, 6);

  const Figure figure1(2, Figure::kHorizontal);
  board.AddFigure(&figure1, 0, 2);
  const Figure figure2(3, Figure::kHorizontal);
  board.AddFigure(&figure2, 0, 0);
  const Figure figure3(3, Figure::kVertical);
  board.AddFigure(&figure3, 4, 0);
  const Figure figure4(2, Figure::kVertical);
  board.AddFigure(&figure4, 5, 1);
  const Figure figure5(2, Figure::kVertical);
  board.AddFigure(&figure5, 2, 2);
  const Figure figure6(2, Figure::kVertical);
  board.AddFigure(&figure6, 3, 2);
  const Figure figure7(2, Figure::kHorizontal);
  board.AddFigure(&figure7, 2, 1);
  const Figure figure8(2, Figure::kHorizontal);
  board.AddFigure(&figure8, 4, 3);
  const Figure figure9(2, Figure::kHorizontal);
  board.AddFigure(&figure9, 2, 4);
  const Figure figure10(2, Figure::kVertical);
  board.AddFigure(&figure10, 1, 3);
  const Figure figure11(2, Figure::kVertical);
  board.AddFigure(&figure11, 5, 4);

  board.DebugPrint();

  SolveBoard(board);
}

int main() {
  // RunTest();
  // Solve1();
  Solve2();

  return 0;
}
