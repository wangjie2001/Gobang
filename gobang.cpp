#include "Gobang.h"
#include <QPainter>
#include <QMouseEvent>
#include <QMessageBox>
#include <QVBoxLayout>
#include <algorithm>

Gobang::Gobang(QWidget *parent) : QWidget(parent) {
    setFixedSize(MARGIN * 2 + BOARD_SIZE * CELL_SIZE, MARGIN * 2 + BOARD_SIZE * CELL_SIZE + 100);
    board.resize(BOARD_SIZE, std::vector<Player>(BOARD_SIZE, Player::None));
    undoButton = new QPushButton("Undo", this);
    restartButton = new QPushButton("Restart", this);
    undoButton->setGeometry(MARGIN, MARGIN * 2 + BOARD_SIZE * CELL_SIZE + 10, 100, 30);
    restartButton->setGeometry(MARGIN + 120, MARGIN * 2 + BOARD_SIZE * CELL_SIZE + 10, 100, 30);
    connect(undoButton, &QPushButton::clicked, this, &Gobang::undoMove);
    connect(restartButton, &QPushButton::clicked, this, &Gobang::restartGame);
    initGame();
}

Gobang::~Gobang() {
    delete undoButton;
    delete restartButton;
}

void Gobang::initGame() {
    for (auto &row : board) {
        std::fill(row.begin(), row.end(), Player::None);
    }
    while (!moveHistory.empty()) moveHistory.pop();
    gameOver = false;
    currentPlayer = Player::Black;
    update();
}

void Gobang::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw board
    painter.setPen(Qt::black);
    for (int i = 0; i < BOARD_SIZE; ++i) {
        painter.drawLine(MARGIN + i * CELL_SIZE, MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (BOARD_SIZE - 1) * CELL_SIZE);
        painter.drawLine(MARGIN, MARGIN + i * CELL_SIZE, MARGIN + (BOARD_SIZE - 1) * CELL_SIZE, MARGIN + i * CELL_SIZE);
    }

    // Draw pieces
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (board[y][x] == Player::Black) {
                painter.setBrush(Qt::black);
                painter.drawEllipse(MARGIN + x * CELL_SIZE - CELL_SIZE / 2, MARGIN + y * CELL_SIZE - CELL_SIZE / 2, CELL_SIZE - 5, CELL_SIZE - 5);
            } else if (board[y][x] == Player::White) {
                painter.setBrush(Qt::white);
                painter.drawEllipse(MARGIN + x * CELL_SIZE - CELL_SIZE / 2, MARGIN + y * CELL_SIZE - CELL_SIZE / 2, CELL_SIZE - 5, CELL_SIZE - 5);
            }
        }
    }
}

void Gobang::mousePressEvent(QMouseEvent *event) {
    if (gameOver || currentPlayer != Player::Black) return;

    int x = (event->x() - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
    int y = (event->y() - MARGIN + CELL_SIZE / 2) / CELL_SIZE;

    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && placePiece(x, y)) {
        moveHistory.push({x, y});
        if (checkWin(x, y, Player::Black)) {
            gameOver = true;
            QMessageBox::information(this, "Game Over", "Black wins!");
            return;
        }
        currentPlayer = Player::White;
        aiMove();
        update();
    }
}

bool Gobang::placePiece(int x, int y) {
    if (board[y][x] != Player::None) return false;
    board[y][x] = currentPlayer;
    return true;
}

void Gobang::aiMove() {
    auto [x, y] = findBestMove();
    placePiece(x, y);
    moveHistory.push({x, y});
    if (checkWin(x, y, Player::White)) {
        gameOver = true;
        QMessageBox::information(this, "Game Over", "White (AI) wins!");
    }
    currentPlayer = Player::Black;
}

bool Gobang::checkWin(int x, int y, Player player) {
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    for (auto &dir : directions) {
        int count = 1;
        for (int step = -1; step <= 1; step += 2) {
            for (int i = 1; i < 5; ++i) {
                int nx = x + dir[0] * i * step;
                int ny = y + dir[1] * i * step;
                if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE || board[ny][nx] != player) break;
                count++;
            }
        }
        if (count >= 5) return true;
    }
    return false;
}

int Gobang::evaluateBoard(int x, int y, Player player) {
    int score = 0;
    int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
    for (auto &dir : directions) {
        int count = 1;
        int openEnds = 0; // 修改为 int 类型
        for (int step = -1; step <= 1; step += 2) {
            bool blocked = false;
            for (int i = 1; i < 5; ++i) {
                int nx = x + dir[0] * i * step;
                int ny = y + dir[1] * i * step;
                if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE || board[ny][nx] == (player == Player::Black ? Player::White : Player::Black)) {
                    blocked = true;
                    break;
                }
                if (board[ny][nx] == player) count++;
                else if (board[ny][nx] == Player::None) break;
            }
            if (!blocked) openEnds++;
        }
        if (count >= 5) score += 100000;
        else if (count == 4 && openEnds == 2) score += 10000;
        else if (count == 4 && openEnds == 1) score += 1000;
        else if (count == 3 && openEnds == 2) score += 500;
        else if (count == 3 && openEnds == 1) score += 100;
        else if (count == 2 && openEnds == 2) score += 50;
    }
    return score;
}

std::pair<int, int> Gobang::findBestMove() {
    int bestScore = -1;
    std::pair<int, int> bestMove = {BOARD_SIZE / 2, BOARD_SIZE / 2};

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (board[y][x] != Player::None) continue;
            board[y][x] = Player::White;
            int score = evaluateBoard(x, y, Player::White);
            board[y][x] = Player::None;
            if (score > bestScore) {
                bestScore = score;
                bestMove = {x, y};
            }
        }
    }

    // Also consider blocking player's strong moves
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (board[y][x] != Player::None) continue;
            board[y][x] = Player::Black;
            int score = evaluateBoard(x, y, Player::Black);
            board[y][x] = Player::None;
            if (score >= 1000 && score > bestScore) {
                bestScore = score;
                bestMove = {x, y};
            }
        }
    }

    return bestMove;
}

void Gobang::undoMove() {
    if (moveHistory.size() < 2 || gameOver) return;
    for (int i = 0; i < 2; ++i) {
        auto [x, y] = moveHistory.top();
        board[y][x] = Player::None;
        moveHistory.pop();
    }
    currentPlayer = Player::Black;
    update();
}

void Gobang::restartGame() {
    initGame();
}
