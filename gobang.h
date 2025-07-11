#ifndef GOBANG_H
#define GOBANG_H

#include <QWidget>
#include <QPushButton>
#include <vector>
#include <stack>

class Gobang : public QWidget {
    Q_OBJECT

public:
    Gobang(QWidget *parent = nullptr);
    ~Gobang();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void undoMove();
    void restartGame();

private:
    static const int BOARD_SIZE = 15;
    static const int CELL_SIZE = 40;
    static const int MARGIN = 30;

    enum class Player { None, Black, White };

    std::vector<std::vector<Player>> board;
    std::stack<std::pair<int, int>> moveHistory;
    bool gameOver;
    Player currentPlayer;
    QPushButton *undoButton;
    QPushButton *restartButton;

    void initGame();
    bool placePiece(int x, int y);
    void aiMove();
    bool checkWin(int x, int y, Player player);
    int evaluateBoard(int x, int y, Player player);
    std::pair<int, int> findBestMove();
};

#endif // GOBANG_H
