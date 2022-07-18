#include <iostream>
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

class CellTable : public sf::Drawable {
private:
	sf::VertexArray vertices;
	bool *ptr, *ptrNext;
	const unsigned int rows, cols, nCells;
	const double cellSize = 10;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
		target.draw(vertices, states);
	}

	bool getCell(const unsigned int iRow, const unsigned int iCol) const {
		return ptr[iCol + iRow * cols];
	}

	unsigned int getNeighbours(const unsigned int iRow, const unsigned int iCol) const {
		unsigned int neighbourCount = getCell((iRow - 1 + rows) % rows, (iCol - 1 + cols) % cols) + getCell((iRow - 1 + rows) % rows, iCol) + getCell((iRow - 1 + rows) % rows, (iCol + 1 + cols) % cols)
			+ getCell(iRow, (iCol - 1 + cols) % cols) + getCell(iRow, (iCol + 1 + cols) % cols)
			+ getCell((iRow + 1 + rows) % rows, (iCol - 1 + cols) % cols) + getCell((iRow + 1 + rows) % rows, iCol) + getCell((iRow + 1 + rows) % rows, (iCol + 1 + cols) % cols);

		return neighbourCount;
	}

	void changeCellColor(const unsigned int iRow, const unsigned int iCol) {
		sf::Vertex* cell = &vertices[(iCol + iRow * cols) * 4];

		if (getCell(iRow, iCol)) {
			cell[0].color = sf::Color::White;
			cell[1].color = sf::Color::White;
			cell[2].color = sf::Color::White;
			cell[3].color = sf::Color::White;
			return;
		}

		cell[0].color = sf::Color::Black;
		cell[1].color = sf::Color::Black;
		cell[2].color = sf::Color::Black;
		cell[3].color = sf::Color::Black;
	}

	void setupTable() {
		for (unsigned int i = 0; i < nCells; ++i) ptr[i];

		vertices.setPrimitiveType(sf::Quads);
		vertices.resize(nCells * 4);
		for (unsigned int i = 0; i < rows; ++i) {
			for (unsigned int j = 0; j < cols; ++j) {
				sf::Vertex* cell = &vertices[(j + i * cols) * 4];
				
				cell[0].position = sf::Vector2f(j * cellSize, i * cellSize);
				cell[1].position = sf::Vector2f((j + 1) * cellSize, i * cellSize);
				cell[2].position = sf::Vector2f((j + 1) * cellSize, (i + 1) * cellSize);
				cell[3].position = sf::Vector2f(j * cellSize, (i + 1) * cellSize);

				cell[0].color = sf::Color::Black;
				cell[1].color = sf::Color::Black;
				cell[2].color = sf::Color::Black;
				cell[3].color = sf::Color::Black;
			}
		}
	}

	bool getNextCellState(const unsigned int iRow, const unsigned int iCol) const {
		const unsigned int neighbourCount = getNeighbours(iRow, iCol);
		if (neighbourCount == 3) return 1;
		if (neighbourCount == 2 && getCell(iRow, iCol)) return 1;
		return 0;
	}

public:
	CellTable() : rows(10), cols(10), nCells(rows*cols) {
		ptr = new bool[nCells];
		ptrNext = new bool[nCells];

		setupTable();
	}

	CellTable(const unsigned int size) : rows(size), cols(size), nCells(rows* cols) {
		ptr = new bool[nCells];
		ptrNext = new bool[nCells];

		setupTable();
	}

	CellTable(const unsigned int nRows, const unsigned int nCols) : rows(nRows), cols(nCols), nCells(rows* cols) {
		ptr = new bool[nCells];
		ptrNext = new bool[nCells];

		setupTable();
	}

	~CellTable() {
		delete[] ptr;
		delete[] ptrNext;
	}

	void updateTable() {
		for (unsigned int i = 0; i < rows; ++i) {
			for (unsigned int j = 0; j < cols; ++j) {
				ptrNext[j + i * cols] = getNextCellState(i, j);
			}
		}

		for (unsigned int i = 0; i < rows; ++i) {
			for (unsigned int j = 0; j < cols; ++j) {
				ptr[j + i * cols] = ptrNext[j + i * cols];
				changeCellColor(i, j);
			}
		}
	}

	void changeCellState(const unsigned int iRow, const unsigned int iCol) {
		ptr[iCol + iRow * cols] = !ptr[iCol + iRow * cols];
		changeCellColor(iRow, iCol);
	}

	unsigned int getCellSize() const {
		return cellSize;
	}
};

int main() {
	unsigned int rows, cols;
	std::cout << "Input the number of rows: ";
	std::cin >> rows;
	std::cout << "Input the number of collons: ";
	std::cin >> cols;
	std::cout << "White cells are alive, black cells are dead\n";
	std::cout << "Lmb to change the state of the selected cell\n";
	std::cout << "Rmb to move the world view\n";
	std::cout << "I/O - Zoom in / Zoom out\n";
	std::cout << "Right/Left arrow keys - increase/decrease cycle frequency\n";
	std::cout << "Space bar - pause/unpause the game\n\n";
	std::cout << "Game is paused\n";

	CellTable table(rows, cols);
	sf::Color gray(128, 128, 128);

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Game of Life", sf::Style::Default);
	sf::View view(sf::Vector2f(0, 0), sf::Vector2f(1280, 720));
	view.setCenter(sf::Vector2f(cols * table.getCellSize() / 2, rows * table.getCellSize() / 2));
	double zoom = 1, accZoom = 1;
	view.zoom(zoom);
	window.setView(view);
	
	sf::Vector2f oldPos;
	bool rmbPressed = 0, paused = 1, lmbPressed = 0;
	unsigned int cyclePeriodMs = 1024, iRowPrev = rows, iColPrev = cols;
	sf::Clock clock;

	while (window.isOpen()) {
		sf::Event event;

		while (window.pollEvent(event)) {
			switch (event.type) {

			case sf::Event::Closed:
				window.close();

			case sf::Event::KeyPressed:
				switch (event.key.code) {

				case sf::Keyboard::I:
					zoom = .5f;
					accZoom *= zoom;
					view.zoom(zoom);
					window.setView(view);
					break;

				case sf::Keyboard::O:
					zoom = 2.f;
					accZoom *= zoom;
					view.zoom(zoom);
					window.setView(view);
					break;

				case sf::Keyboard::Left:
					if (cyclePeriodMs < 8192)
						cyclePeriodMs *= 2;
					std::cout << "New cycle period: " << cyclePeriodMs << " ms\n";
					break;

				case sf::Keyboard::Right:
					if (cyclePeriodMs > 64)
						cyclePeriodMs /= 2;
					std::cout << "New cycle period: " << cyclePeriodMs << " ms\n";
					break;

				case sf::Keyboard::Space:
					paused = !paused;
					if (paused) std::cout << "Game is now paused\n";
					else std::cout << "Game is now unpaused\n";
				}

			case sf::Event::MouseButtonPressed:
				if(event.mouseButton.button == sf::Mouse::Right){
					rmbPressed = 1;
					oldPos = sf::Vector2f(sf::Mouse::getPosition());
				}

				if (event.mouseButton.button == sf::Mouse::Left) {
					lmbPressed = 1;
					sf::Vector2f mousePosWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window));
					const unsigned int iRow = (unsigned int)floor(mousePosWorld.y / table.getCellSize()), iCol = (unsigned int)floor(mousePosWorld.x / table.getCellSize());
					table.changeCellState(iRow, iCol);
					iRowPrev = iRow;
					iColPrev = iCol;
				}
				break;

			case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Right) {
					rmbPressed = 0;
				}
				if (event.mouseButton.button == sf::Mouse::Left) {
					lmbPressed = 0;
					iRowPrev = rows;
					iColPrev = cols;
				}
				break;

			case sf::Event::MouseMoved:
				if (rmbPressed) {
					const sf::Vector2f newPos = sf::Vector2f(sf::Mouse::getPosition());
					sf::Vector2f dPos = oldPos - newPos;
					dPos.x *= accZoom;
					dPos.y *= accZoom;
					view.move(dPos);
					window.setView(view);
					oldPos = newPos;
				}

				if (lmbPressed) {
					sf::Vector2f mousePosWorld = window.mapPixelToCoords(sf::Mouse::getPosition(window));
					const unsigned int iRow = (unsigned int)floor(mousePosWorld.y / table.getCellSize()), iCol = (unsigned int)floor(mousePosWorld.x / table.getCellSize());

					if (iRow != iRowPrev || iCol != iColPrev) {
						table.changeCellState(iRow, iCol);
						iRowPrev = iRow;
						iColPrev = iCol;
					}
				}
				break;
			}
		}

		sf::Time elapsed = clock.getElapsedTime();
		if (paused) {
			clock.restart();
		}
		else if (elapsed.asMilliseconds() >= cyclePeriodMs) {
			table.updateTable();
			clock.restart();
		}

		window.clear(gray);
		window.draw(table);
		window.display();
	}
	return 0;
}